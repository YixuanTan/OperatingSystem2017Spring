//
//  main.c
//  playbuffer
//
//  Created by Yixuan Tan on 2/22/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, const char * argv[]) {
    // insert code here...
    /*
    char buffer[11] = "0123456789";
    buffer[10] = '\0';
    write( 1, buffer, 10 ); // stdout immediately
    printf(buffer); // the output will be buffered
    */
    
    
    
    int fd;
    close( 2 );
#if 0
    close( 1 ); /* <== add this line later.... */
#endif
    printf( "HI\n" );
    fflush( stdout );
    fd = open( "newfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
    printf( "==> %d\n", fd );
    printf( "WHAT?\n" );
    fprintf( stderr, "ERROR\n" );
    close( fd );
    return EXIT_SUCCESS;
}
