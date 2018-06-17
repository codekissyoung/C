#include "common.h"

#define oops(m,x) {perror(m);exit(x);}

int main( int argc, char *argv[] )
{
	int pid;
	int todc[2];	
	int fromdc[2];

	pipe(todc);
	pipe(fromdc);
	
	pid = fork();

	if( pid == -1 )
		oops("fork error",1);

	if( pid == 0 )
	{
		be_dc( todc, fromdc );
	}

	be_bc( todc, fromdc );
	wait( NULL );
}

void be_dc( int in[2], int out[2] )
{
	if( dup2( in[0], 0 ) == -1 ) // 标准输入换成 in的 输出端
		oops("dc : cannot redirect stdin", 3 );
	close( in[0] );
	close( in[1] );

	if( dup2( out[1], 1 ) == -1 ) // 标准输出换成 out 的输入端
		oops("dc : cannot redirect stdout", 4 );

	execlp( "dc", "dc", "-", NULL );
	oops("Can not run dc", 5 );
}

void be_bc( int todc[2], int fromdc[2] )
{
	int num1;
	int num2;
	char operation[BUFSIZ], message[BUFSIZ];
	FILE *fpout, *fpin;


}
