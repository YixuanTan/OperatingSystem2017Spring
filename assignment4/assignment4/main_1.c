//
//  main.c
//  assignment4
//
//  Created by Yixuan Tan on 4/13/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>      /* <===== */
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

// What sync should be taken care?
// 1. file system read / write
// 2. when read, write(to the same file) is prohibited, read is fine
// 3. when write, read and write (to the same file) are prohibited
// 4. when spawning a new thread, the client_sockets[] should be protected by mutex
// So, every file should have a mutex

/*
struct FileInfo{
    pthread_mutex_t fileMutex;
    char filename[32];
};
struct FileInfo** fileSystem;
*/

#define BUFFER_SIZE 20
#define MAX_CLIENTS 100      /* <===== */

fd_set readfds;
int client_sockets[MAX_CLIENTS];
pthread_t tid[MAX_CLIENTS];
int client_socket_index = 0;

// The operation on the file system is protected by a mutex
pthread_mutex_t filesystem_reading = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t filesystem_writting = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientlist = PTHREAD_MUTEX_INITIALIZER;

int runsave(char filename[] , int byteCount, char* fileContent, int newsd){
    // return value: -1 INVALID REQUSET
    //               -2 FILE EXISTS
    //                0 successful
    int len = (int)strlen(filename);
    int i;
    for(i = 0; i < len; i++) {
        if(i == 0 && isalpha(filename[i]) == 0) return -1;
        if(isalnum(filename[i]) == 0 && filename[i] != '.') return -1;
    }
    // now the filename is valid
    if(byteCount <= 0) return -1;
    if (access(filename, F_OK) != -1) return -2;
    
    // store file
    char* fileBuffer = (char *)malloc((byteCount + 1)*sizeof(char));
    long j = strlen(fileContent); // strlen() not including the '\0'
    memcpy(fileBuffer, fileContent, j);
    
    // fill the fileBuffer, starting from j
    int n = 0;
    char buffer_tmp[BUFFER_SIZE + 1];
    while(j != byteCount)
    {
        /* recv() will block until we receive data (n > 0)
         or there's an error (n == -1)
         or the client closed the socket (n == 0) */
        n = (int)recv( newsd, buffer_tmp, BUFFER_SIZE, 0 );
        
        if ( n == -1 )
        {
            perror( "recv() failed" );
            break;
        }
        else if ( n == 0 )
        {
            printf( "SERVER: Rcvd 0 from recv(); closing socket...\n" );
            break;
        }
        else /* n > 0 */
        {
            buffer_tmp[n] = '\0';    /* assume this is text data */
            strncpy(&fileContent[j], buffer_tmp, n);
            j += n;
        }
    }
    
    pthread_mutex_lock(&filesystem_reading);
    pthread_mutex_lock(&filesystem_writting);
    FILE *fp;
    fp = fopen( filename , "wb" );
    fwrite(fileContent , sizeof(char), byteCount, fp );
    fclose(fp);
    pthread_mutex_unlock(&filesystem_writting);
    pthread_mutex_unlock(&filesystem_reading);
    return 0;
}

int runread(char filename[] , int byteOffset, int length, char** fileContent, int newsd){
    // return value: -1 INVALID REQUSET
    //               -2 NO SUCH FILE
    //               -3 INVALID BYTE RANGE
    //                0 successful

    int len = (int)strlen(filename);
    int i;
    for(i = 0; i < len; i++) {
        if(i == 0 && isalpha(filename[i]) == 0) return -1;
        if(isalnum(filename[i]) == 0 && filename[i] != '.') return -1;
    }
    // now the filename is valid
    if(length <= 0) return -1;
    if (access(filename, F_OK) == -1) return -2;
    
    // read file to fileContent
    // http://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
    pthread_mutex_lock(&filesystem_writting);
    
    FILE *fileptr;
    long filelen;
    fileptr = fopen(filename, "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file
    
    if(length > filelen - byteOffset) return -3;
    fseek(fileptr, byteOffset, SEEK_SET);
    *fileContent = (char *)malloc((length + 1)*sizeof(char)); // Enough memory for length + \0
    fread(*fileContent, length, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file
    pthread_mutex_unlock(&filesystem_writting);
    return 0;
}

int cmpfunc (const void * a, const void * b) {
    char* file1 = (char *) a;
    char* file2 = (char *) b;
    int len1 = (int)strlen(file1);
    int len2 = (int)strlen(file2);
    int i = 0, j = 0;
    while(i < len1 && j < len2) {
        if(file1[i] - file2[j] > 0) return 1;
        else if(file1[i] - file2[j] < 0) return -1;
        else {
            i++;
            j++;
        }
    }
    if(i == len1 && j == len2) return 0;
    if(i == len1) return -1;
    if(j == len2) return 1;
    return 0;
}


char** runlist(int* nfiles) {
    char wkdir[128];
    memset(wkdir, '\0', 128);
    getcwd(wkdir, 128);
    int len = (int)strlen(wkdir);
    strcpy(&wkdir[len], "/storage");
    chdir (wkdir);
    DIR * dir = opendir( "/Users/yixuantan/Developer/os/assignment4/Build/Products/Debug/storage" );   /* open the current working directory */
    
    if ( dir == NULL )
    {
        perror( "opendir() failed" );
        return NULL;
    }
    
    struct dirent * file;
    char** filelist;
    pthread_mutex_lock(&filesystem_writting);
    while ( ( file = readdir( dir ) ) != NULL )
    {
        printf( "found %s", file->d_name );

        struct stat buf;
        int rc = lstat( file->d_name, &buf );
        
        if ( rc == -1 )
        {
            perror( "lstat() failed" );
            return NULL;
        }
        
        
        if ( S_ISREG( buf.st_mode ) )
        {
            if(*nfiles == 0) {
                *nfiles = 1;
                filelist = (char **) malloc((*nfiles) * sizeof(char*));
            }
            else {
                (*nfiles)++;
                void* tmp = realloc(filelist, (*nfiles) * sizeof(char*));
                if(tmp != NULL) {
                    filelist = (char**) tmp;
                }else {
                    perror("ERROR");
                    exit(1);
                }
            }
            
            filelist[*nfiles - 1] = (char *) malloc(32 * sizeof(char));
            memset(filelist[*nfiles - 1], '\0', 32);
            strcpy(filelist[*nfiles - 1], file->d_name);
        }
    }
    
    pthread_mutex_unlock(&filesystem_writting);
    closedir( dir );
    if(*nfiles == 0) return NULL;
    //qsort(*filelist, *nfiles, 32 * sizeof(char), cmpfunc);
    return 0;
}


void* TcpClientHandler(void* TcpClientHandlerArg) {
    pthread_detach(pthread_self()); // detach from main, automatic releasing resources after termination
    
    int sd_tcp = *((int*) TcpClientHandlerArg);
    struct sockaddr_in client;
    int fromlen = sizeof( client );
    int newsd = accept( sd_tcp, (struct sockaddr *)&client, (socklen_t *)&fromlen );
    int n;
    char buffer[ BUFFER_SIZE ];

    printf( "SERVER: Accepted new client connection on sd %d\n", newsd );
    do
    {
        printf( "SERVER: Blocked on recv()\n" );
        
        /* recv() will block until we receive data (n > 0)
         or there's an error (n == -1)
         or the client closed the socket (n == 0) */
        n = (int)recv( newsd, buffer, BUFFER_SIZE, 0 );
        
        if ( n == -1 )
        {
            perror( "recv() failed" );
            return NULL;
        }
        else if ( n == 0 )
        {
            printf( "[child %lld]: Client disconnected\n", (long long)pthread_self());
        }
        else /* n > 0 */
        {
            // echo command
            buffer[n] = '\0';    /* assume this is text data */
            char cmd[1024];
            int i;
            for(i = 0; i < n; i++) {
                if(buffer[i] == '\n') break;
                cmd[i] = buffer[i];
            }
            cmd[i] = '\0';
            printf( "[child %lld]: Received %s\n", (long long)pthread_self(), cmd);
            
            // process command
            char type[5];
            size_t typelen = 4;
            strncpy(type, buffer, typelen);
            //if(rc != typelen) fprintf(stderr, "ERROR: wrong command");
            type[4] = '\0';
            
            char return_msg[1024]; // store error return message to be sent back to client
            if(strcmp(type, "SAVE") == 0) {
                char filename[32];
                int fi;
                for(fi = 5; buffer[fi] != ' '; fi++) filename[fi-5] = buffer[fi];
                fi++; // skip space;
                int byteCount;
                byteCount = 0;
                for(; buffer[fi] != '\n'; fi++) byteCount = byteCount * 10 + (buffer[fi] - '0');
                char filecontent[ BUFFER_SIZE ];
                fi++;
                int fc = 0;
                for(; fi < BUFFER_SIZE; fi++) filecontent[fc++] = buffer[fi];
                filecontent[fc] = '\0';
                
                int rc = runsave(filename, byteCount, filecontent, newsd);
                if(rc == -1) {
                    memset(return_msg, '\0', 1024);
                    strcpy(return_msg, "ERROR INVALID REQUEST\n");
                    send( newsd, return_msg, strlen(return_msg), 0 );
                    //fprintf(stderr, "ERROR INVALID REQUEST\n");
                }
                else if(rc == -2){
                    memset(return_msg, '\0', 1024);
                    strcpy(return_msg, "ERROR FILE EXISTS\n");
                    send( newsd, return_msg, strlen(return_msg), 0 );
                    //fprintf(stderr, "ERROR FILE EXISTS\n");
                }
                else {
                    send( newsd, "ACK\n", 4, 0 );
                    printf( "[child %lld]: Sent ACK\n", (long long)pthread_self());
                }
            }
            else if(strcmp(type, "READ") == 0) {
                char filename[32];
                int fi;
                for(fi = 5; buffer[fi] != ' '; fi++) filename[fi-5] = buffer[fi];
                fi++; // skip space;
                int byteOffset = 0;
                for(; buffer[fi] != ' '; fi++) byteOffset = byteOffset * 10 + (buffer[fi] - '0');
                int length = 0;
                fi++; // skip space;
                for(; buffer[fi] != '\n'; fi++) length = length * 10 + (buffer[fi] - '0');
                char* fileContent;
                int rc = runread(filename, byteOffset, length, &fileContent, newsd);
                if(rc == -1) {
                    memset(return_msg, '\0', 1024);
                    strcpy(return_msg, "ERROR INVALID REQUEST\n");
                    send( newsd, return_msg, strlen(return_msg), 0 );
                    //fprintf(stderr, "ERROR INVALID REQUEST\n");
                }
                else if(rc == -2){
                    memset(return_msg, '\0', 1024);
                    strcpy(return_msg, "ERROR FILE EXISTS\n");
                    send( newsd, return_msg, strlen(return_msg), 0 );
                    //fprintf(stderr, "ERROR FILE EXISTS\n");
                }
                else {
                    n = (int)send( newsd, "ACK\n", 4, 0 );
                    printf( "[child %lld]: Sent ACK %d\n", (long long)pthread_self(), length);
                    printf( "[child %lld]: Sent %d bytes of \"%s\" from offset %d\n", (long long)pthread_self(), length, filename, byteOffset);
                    free(fileContent);
                    fileContent = NULL;
                }
            }
            else if(strcmp(type, "LIST") == 0) {
                int nfiles = 0;
                char** filelist = runlist(&nfiles);
                if(runlist == NULL) {
                    memset(return_msg, '\0', 1024);
                    strcpy(return_msg, "0\n");
                    send( newsd, return_msg, strlen(return_msg), 0 );
                }
                else {
                    char *return_list = (char *) malloc( (nfiles + 1) * 33 * sizeof(char));
                    char numOfFiles[16];
                    memset(numOfFiles, '\0', 16);
                    int nfiles_tmp = nfiles;
                    int i = 0;
                    for(; i < 16; i++) {
                        if(nfiles_tmp == 0) break;
                        numOfFiles[i] = nfiles_tmp % 10 + '0';
                        nfiles_tmp /= 10;
                    }
                    int j = 0;
                    i--;
                    // reverse string
                    while(i < j) {
                        char c = numOfFiles[i];
                        numOfFiles[i] = numOfFiles[j];
                        numOfFiles[j] = c;
                    }
                    
                    // fill number of files
                    long long p = 0;
                    strcpy(&return_list[p], numOfFiles);
                    p = (long long)strlen(return_list);
                    return_list[p++] = ' ';
                    
                    int f = 0;
                    for(; f < nfiles; f++) {
                        strcpy(&return_list[p], filelist[f]);
                        p = (long long)strlen(return_list);
                        if(f != nfiles - 1) return_list[p++] = ' ';
                    }
                    
                    send(newsd, filelist, p, 0);
                    
                    // clean the dyna memo
                    f = 0;
                    for(; f < nfiles; f++) {
                        free(filelist[f]);
                        filelist[f] = NULL;
                    }
                    free(filelist);
                    filelist = NULL;
                
                }
            }
            
        }
        
    }
    while ( n > 0 );
    
    close( newsd );
    return NULL;
}


void UdpClientHandler(int sd_udp) {
    /* the code below implements the application protocol */
    int n;
    char buffer[ BUFFER_SIZE ];
    struct sockaddr_in client;
    socklen_t len = sizeof( client );
    
    while(1)
    {
        /* read a datagram from the remote client side (BLOCKING) */
        n = (int)recvfrom( sd_udp, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client, (socklen_t *) &len );
        printf("n is %d, sd_udp is %d", n, sd_udp);
        if ( n < 0 )
        {
            perror( "recvfrom() failed" );
            break;
        }
        else if ( n == 0 )
        {
            printf( "SERVER: Rcvd 0 from recv(); closing socket...\n" );
            break;
        }
        else
        {
            printf( "Rcvd datagram from %s port %d\n",
                   inet_ntoa( client.sin_addr ), ntohs( client.sin_port ) );
            
            printf( "RCVD %d bytes\n", n );
            buffer[n] = '\0';
            printf( "RCVD: [%s]\n", buffer );
            
            /* echo the data back to the sender/client */
            // sendto( sd_udp, buffer, n, 0, (struct sockaddr *) &client, len );
            /* to do: check the return code of sendto() */
        }
    }
    
}


int main(int argc, char** argv)
{
    /*
    check the incoming connection -> TCP / UDP
    use: select()
         create a TCP server and a UDP server
    */
    // main is only responsible for detect TCP or UDP and spawn a thread to take care of the connection.
    if(argc != 3) {
        fprintf(stderr, "ERROR: command line argument must be executable port_tcp# port_udp#");
    }
    int port_tcp = atoi(argv[1]);
    int port_udp = atoi(argv[2]);
    
    /* create the socket (endpoint) on the server side */
    int sd_tcp = socket( PF_INET, SOCK_STREAM, 0 );
    int tr=1;
    if (port_tcp < 0 && setsockopt(port_tcp,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(long long)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    int sd_udp = socket( PF_INET, SOCK_DGRAM, 0 );
    
    if ( sd_tcp < 0 || sd_udp < 0 )   /* this will be part of the file descriptor table! */
    {
        perror( "socket() failed" );
        return EXIT_FAILURE;
    }
    
    // create the TCP/UDP server
    struct sockaddr_in server_tcp, server_udp;

    server_tcp.sin_family = server_udp.sin_family = PF_INET;  /* IPv4 */
    server_tcp.sin_addr.s_addr = server_udp.sin_addr.s_addr = htonl( INADDR_ANY );
    
    /* specify the port number for the server */
    server_tcp.sin_port = htons( port_tcp);
    server_udp.sin_port = htons( port_udp);
    
    /* bind to a specific (OS-assigned) port number */
    int bindtcp = bind( sd_tcp, (struct sockaddr *) &server_tcp, sizeof( server_tcp ) );
    int bindudp = bind( sd_udp, (struct sockaddr *) &server_udp, sizeof( server_udp ) );
    if (bindtcp < 0 || bindudp < 0)
    {
        perror( "bind() failed" );
        return EXIT_FAILURE;
    }
    
    int length_tcp = sizeof( server_tcp );
    int length_udp = sizeof( server_udp );
    
    listen( sd_tcp, 5 );  /* 5 is number of waiting clients */
    printf( "Listener bound to port %d\n", port_tcp );

    if ( getsockname( sd_tcp, (struct sockaddr *) &server_tcp, (socklen_t *) &length_tcp ) < 0 || getsockname( sd_udp, (struct sockaddr *) &server_udp, (socklen_t *) &length_udp ) < 0 )
    {
        perror( "getsockname() failed" );
        return EXIT_FAILURE;
    }
    
    printf( "UDP server at port number %d\n", ntohs( server_udp.sin_port ) );
    
    while ( 1 )
    {
        struct timeval timeout;
        timeout.tv_sec = 3;
        timeout.tv_usec = 500;  /* 3 AND 500 microseconds */
        
        FD_ZERO( &readfds );
        FD_SET( sd_tcp, &readfds ); // put in the tcp server file descriptor
        FD_SET( sd_udp, &readfds ); // put in the udp server file descriptor

        printf( "Set FD_SET to include listener fd %d and %d\n", sd_tcp, sd_udp );
        
        // blocking until a connection request is received
        int ready = select( FD_SETSIZE, &readfds, NULL, NULL, NULL );
        /* ready is the number of ready file descriptors */
        printf("ready is %d\n", ready);
        if ( ready == 0 ) {
            printf( "No activity\n" );
            continue;
        }
        printf( "select() identified %d descriptor(s) with activity\n", ready );
        
        if ( FD_ISSET( sd_tcp, &readfds ) ) {
            int nthsocket;
            pthread_mutex_lock(&clientlist);
            nthsocket = client_socket_index;
            client_socket_index++;
            pthread_mutex_unlock(&clientlist);
            
            int rc = pthread_create( &tid[nthsocket], NULL, TcpClientHandler, &sd_tcp );
            
            if ( rc != 0 ) {
                fprintf( stderr, "MAIN: Could not create child thread (%d)\n", rc );
            }
        }
        if ( FD_ISSET( sd_udp, &readfds ) ) {
            //int nthsocket;
            //pthread_mutex_lock(&clientlist);
            //nthsocket = client_socket_index;
            //client_socket_index++;
            //pthread_mutex_unlock(&clientlist);

            UdpClientHandler(sd_udp);
        }
    }
    
    return EXIT_SUCCESS;
}
