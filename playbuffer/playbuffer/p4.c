#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main()
{
int fd;
close( 2 );
#if 0
close( 1 ); /* <== add this line later.... */
#endif
printf( "HI\n" );
fflush( stdout );
fd = open( "newfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
printf( "==> %d", fd );
printf( "WHAT?\n" );
fprintf( stderr, "ERROR" );
//sleep(10);
//fflush(NULL);
close( fd );
return EXIT_SUCCESS;
}