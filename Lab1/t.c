#include <stdio.h>
#include <stdlib.h>
#include "myprintf.h"

int *FP;

int main(int argc, char *argv[], char *env[])
{

	char z = 'A';
	char * s = "this is a test";
	int d = 100;
	int neg = -100;
	unsigned int u = 100;
	
	myprintf("c=%c, s=%s, d=%d, u=%u, o=%o, x=%x, neg=%d\n", z, s, d, u, u, u, neg);
	//Write C code to print values of argc and argv[] entries
	//2-2. In the int main(int argc, char *argv[ ], char *env[ ]) function, 
    	//use YOUR myprintf() to print
        //      argc value
        //      argv strings
        //      env  strings
 
	myprintf("argc=%d\n\n", argc);

	for(int i = 0; i < argc; i++){
		myprintf("argv[%d]=%s\n",i,argv[i]);
	}
	myprintf("\n");

}



