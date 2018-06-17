#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main( int argc, char* argv[] )
{
	if( write( 1, "Here is some data\n", 18 ) )
	{
		write( 2, "A write error has occurred on file descriptior\n", 46 );
	}
	exit( 0 );
}

