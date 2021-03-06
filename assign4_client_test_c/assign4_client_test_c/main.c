/* tcp-client.c */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main()
{
    /* create TCP client socket (endpoint) */
    int sd = socket( PF_INET, SOCK_STREAM, 0 );
    
    if ( sd < 0 )
    {
        perror( "socket() failed" );
        exit( EXIT_FAILURE );
    }
    
#if 0
    /* localhost maps to 127.0.0.1, which stays on the local machine */
    struct hostent * hp = gethostbyname( "localhost" );
    
    struct hostent * hp = gethostbyname( "128.113.126.29" );
#endif
    
    struct hostent * hp = gethostbyname( "127.0.0.1" );
    
    
    if ( hp == NULL )
    {
        fprintf( stderr, "ERROR: gethostbyname() failed\n" );
        return EXIT_FAILURE;
    }
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    memcpy( (void *)&server.sin_addr, (void *)hp->h_addr, hp->h_length );
    unsigned short port = 9051;
    server.sin_port = htons( port );
    
    printf( "server address is %s\n", inet_ntoa( server.sin_addr ) );
    
    
    printf( "connecting to server.....\n" );
    if ( connect( sd, (struct sockaddr *)&server, sizeof( server ) ) == -1 )
    {
        perror( "connect() failed" );
        return EXIT_FAILURE;
    }
    
    char * msg = "READ test.png 0 25099";
    int l = (int)strlen( msg );
    int n = (int)write( sd, msg, l);
    
    if ( n < strlen( msg ) )
    {
        perror( "write() failed" );
        return EXIT_FAILURE;
    }
    
    
    /* The implementation of the application layer is below... */
    int length = 25099;
    char* buffer = (char *)malloc((length + 1)*sizeof(char)); // Enough memory for length + \0
    memset(buffer, '\0', length + 1);
    char tmp[1024];
    int count = 0;
    while(count < length) {
        memset(tmp, '\0', 1024);
        n = (int)read( sd, tmp, 1024);    /* BLOCKING */
        strcat(buffer, tmp);
        count += n;
    }
    
    if ( n == -1 )
    {
        perror( "read() failed" );
        return EXIT_FAILURE;
    }
    else if ( n == 0 )
    {
        printf( "Rcvd no data; also, server socket was closed\n" );
    }
    else
    {
        buffer[n] = '\0';    /* assume we rcvd text-based data */
        //printf( "Rcvd from server: %s\n", buffer );
        FILE *fp2 = fopen( "check.png" , "wb" );
        fwrite(buffer , sizeof(char), length, fp2 );
        fclose(fp2);
        
    }
    
    close( sd );
    
    return EXIT_SUCCESS;
}
