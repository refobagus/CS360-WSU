#include <stdarg.h>
#include <stdio.h>
#include "myprintf.h"

//Write YOUR own prints(char* s) function to print a string
void prints(char * s)
{
	int i = 0;
  	for(i; s[i] != '\0'; i++)
  	{
    		putchar(s[i]);
	}
}

typedef unsigned int u32;


void rpu(u32 x, int base)
{
	static const char * ctable = "0123456789DEF";
	char c;
	if(x)
	{
		c = ctable[x % base];
		rpu(x / base, base);
		putchar(c);
	}
}

void printu_base(u32 x, char * prefix, int base)
{
	prints(prefix);
	(x == 0) ? putchar('0') : rpu(x, base);
}

//Write YOUR ONW fucntions 
void printd(int x)
{
	if(x < 0)
	{
		putchar('-');
		x = -x;
	}
	printu_base(x, "", 10);
}

void printu(u32 x)
{
	printu_base(x, "", 10);
}

void printx(u32 x)
{
	printu_base(x, "0x", 16);
}

void printo(u32 x)
{
	printu_base(x, "0", 8);
}


void myprintf(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	for(; *fmt != '\0'; fmt++)
	{
		if(*fmt == '%')
		{
			fmt++;
			switch(*fmt)
			{
				case 's': prints(va_arg(arg, char *));
					break;
				case 'd': printd(va_arg(arg, int));
					break;
				case 'c': putchar(va_arg(arg, int));
					break;
				case 'u': printu(va_arg(arg, u32));
					break;
				case 'x': printx(va_arg(arg, u32));
					break;
				case 'o': printo(va_arg(arg, u32));
					break;
				default:
					return;
			}
		}

		else if(*fmt == '\n')
		{
			prints("\r\n");
		}

		else
		{
			putchar(*fmt);
		}
		
	}
	va_end(arg);
}
