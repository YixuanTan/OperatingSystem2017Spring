//
//  main.c
//  ex2_thread_practice
//
//  Created by Yixuan Tan on 4/12/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
void * action( void * arg )
{
    int t = *(int *)arg;
    printf( "THREAD %u: I'm alive %d\n", (unsigned int)pthread_self(), t );
    sleep( t );
    return NULL;
}
int main() {
    pthread_t tid[4];
    int i, t;
    for ( i = 0 ; i < 4 ; i++ )
    {
        t = 1 + i * 3;
        printf( "MAIN: Creating thread for task %d\n", t );
        pthread_create( &tid[i], NULL, action, &t );
    }
    for ( i = 0 ; i < 4 ; i++ )
    {
        pthread_join( tid[i], NULL );
        printf( "MAIN: Joined child thread %u\n", (unsigned int)tid[i] );
    }
    return 0; }
