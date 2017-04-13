//
//  main.c
//  ex2_p5
//
//  Created by Yixuan Tan on 4/11/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

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

#define BUFFER_SIZE 10
int main()
{
    char buffer[ BUFFER_SIZE + 1 ];
    int sd = socket( PF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in server;
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    unsigned short port = 8123;
    server.sin_port = htons( port );
    bind( sd, (struct sockaddr *)&server, sizeof( server ) );
    listen( sd, 5 );
    printf( "SERVER: Listener bound to port %d\n", port );
    struct sockaddr_in client;
    int fromlen = sizeof( client );
    int clients = 10;
    while ( clients > 0 )
    {
        printf( "SERVER: Blocked on accept()\n" );
        int newsd = accept( sd, (struct sockaddr *)&client, (socklen_t *)&fromlen );
        while ( 1 )
        {
            printf( "SERVER: Blocked on recv()\n" );
            int n = recv( newsd, buffer, BUFFER_SIZE, 0 );
            if(n == 0) break;
            buffer[n] = '\0'; /* assume text data */
            printf( "SERVER: Rcvd [%s]\n", buffer );
        }
        close( newsd );
        clients--;
    }
    printf( "SERVER: Exiting\n" );
    return EXIT_SUCCESS;
}
