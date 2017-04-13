//
//  main.c
//  read_close_pipe
//
//  Created by Yixuan Tan on 2/13/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int p[2];
    
    int rc = pipe( p );
    
    if ( rc == -1 )
    {
        perror( "pipe() failed" );
        return EXIT_FAILURE;
    }
    
    pid_t pid = fork();    /* both parent and child have a copy of the fd table */
    
    if ( pid == -1 )
    {
        perror( "fork() failed" );
        return EXIT_FAILURE;
    }
    
    if ( pid == 0 )   /* CHILD */
    {
        close( p[1] );  /* close the read end of the pipe */
        int result = 1234;
        printf( "CHILD: writing result %d to pipe\n", result );
        printf( "CHILD: whoops, something went wrong; exiting\n" );
        
#if 0
        int bytes = write( p[1], &result, sizeof( int ) );
        if ( bytes == -1 )
        {
            perror( "CHILD: write() failed" );
            return EXIT_FAILURE;
        }
#endif
        exit( EXIT_SUCCESS );
    }
    else /* PARENT */
    {
#if 1
        close( p[1] );  /* close the write end of the pipe */
#endif
        int result;
        printf( "PARENT: blocking on the read() from pipe\n" );
        int bytes = read( p[0], &result, sizeof( int ) );
        if ( bytes == -1 )
        {
            perror( "PARENT: read() failed" );
            return EXIT_FAILURE;
        }
        
        if ( bytes == 0 )
        {
            printf( "PARENT: child did not send me anything\n" );
        }
        else /* bytes > 0 */
        {
            printf( "PARENT: read result %d from pipe\n", result );
        }
        
        /* waitpid() should go here... */
    }
    
    return EXIT_SUCCESS;
}
