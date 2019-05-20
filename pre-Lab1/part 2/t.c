#include <stdio.h>
#include <stdlib.h>

int getebp(void);
int A(int, int);
int B(int, int);
int C(int, int);

int *FP;

int main(int argc, char *argv[], char *env[])
{
	int a,b,c;
	printf("\nenter main\n");
  
	printf("&argc=%p argv=%p env=%p\n", (void*)&argc, (void*)argv, (void*)env);
	printf("&a=%p &b=%p &c=%p\n", (void*)&a, (void*)&b, (void*)&c);
 
	printf("argc=%d\n\n", argc);

	for(int i = 0; i < argc; i++){
		printf("argv[%d]=%s\n",i,argv[i]);
	}
	printf("\n");
	a=1; b=2; c=3;
	A(a,b);
	printf("exit main\n");
}

int A(int x, int y)
{
	int d,e,f;
	printf("\nenter A\n");
    	// PRINT ADDRESS OF d, e, f
		printf("&d=%p &e=%p &f=%p\n",(void*)&d,(void*)&e,(void*)&f);
    	d=4; e=5; f=6;
    	B(d,e);
    	printf("exit A\n");
}

int B(int x, int y)
{
    	int g,h,i;
    	printf("\nenter B\n");
    	// PRINT ADDRESS OF g,h,i
		printf("&g=%p &h=%p &i=%p\n",(void*)&g, (void*)&h, (void*)&i);
    	g=7; h=8; i=9;
    	C(g,h);
    	printf("exit B\n");
}

int C(int x, int y)
{
    	int u, v, w, i, *p;

    	printf("\nenter C\n");
    	// PRINT ADDRESS OF u,v,w,i,p;
		printf("&u=%p &v=%p &w=%p &i=%p &p=%p\n",(void*)&u,(void*)&v,(void*)&w,(void*)&i,(void*)&p);
    	u=10; v=11; w=12; i=13;

    	FP = (int *)getebp();
    	
    	//Write C code to print the stack frame link list
	printf("\nprint stack frame link list\n");
  	while(FP != 0)
  	{
   	 	printf("%p ->\n", (void*)FP);
    		FP = (int*)*FP;
 	}
	printf("%d\n", (int)FP);

    	//Print the stack contents from p to the frame of main()
    	//YOU MAY JUST PRINT 128 entries of the stack contents.
	FP = (int *)getebp();

	printf("\nprint stack contents\n");
	int* z = p;
	int f=-11;
  	FP -= 11;
  	while(f < 62)
  	{
    	printf("%p -> %x\n", FP, *FP);

    	FP++;
    	z++;
		f++;
	}

    	//On a hard copy of the print out, identify the stack contents
    	//as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.
}


