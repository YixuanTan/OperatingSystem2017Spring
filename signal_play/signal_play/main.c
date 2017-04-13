//
//  main.c
//  signal_play
//
//  Created by Yixuan Tan on 3/22/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

/* signal-input.c */

/* man 7 signal */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler( int n )   /* n is the signal rcvd (e.g., SIGINT) */
{
    printf( "Rcvd signal %d\n", n );
    
    if ( n == SIGINT )
    {
        printf( "Stop hitting CTRL-C!\n" );
    }
    else if ( n == SIGUSR1 )
    {
        printf( "Reloading the config file....jk\n" );
    }
}

int main()
{
    /* change the behavior/disposition for SIGINT (ctrl-C) */
    signal( SIGINT, SIG_IGN );   /* ignore SIGINT */
    
    signal( SIGINT, SIG_DFL );   /* reset back to the default behavior   */
    /* ...should be Term (see man 7 signal) */
    
    /* redefine SIGINT and SIGUSR1 signals to be caught */
    /*  and handled by the signal_handler() function    */
    signal( SIGINT, signal_handler );
    signal( SIGUSR1, signal_handler );
    
    char name[100];
    printf( "Enter your name ==> " );
    scanf( "%s", name );
    
    printf( "Your name is %s\n", name );
    
    return EXIT_SUCCESS;
}
