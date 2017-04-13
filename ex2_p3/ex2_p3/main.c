//
//  main.c
//  ex2_p3
//
//  Created by Yixuan Tan on 4/12/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <stdio.h>
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
#include <pthread.h>
int x = 12;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   // ADDED mutex code

void * lmao( void * arg )
{
    int q = 4;
    int * z = (int *)arg;
    q -= 2;
    printf( "%u %d %d\n", (unsigned int)pthread_self(), *z, q );
    pthread_mutex_lock( &mutex );  // ADDED mutex code
    *z *= 2;
    pthread_mutex_unlock( &mutex ); // ADDED mutex code
    return NULL;
}

int main()
{
    printf( "%u %d\n", (unsigned int)pthread_self(), x );
    pthread_t tid1, tid2;
    pthread_create( &tid1, NULL, lmao, &x );
    pthread_create( &tid2, NULL, lmao, &x );
    pthread_join( tid1, NULL );
    pthread_join( tid2, NULL );
    printf( "%u %d\n", (unsigned int)pthread_self(), x );
    return EXIT_SUCCESS;
}
