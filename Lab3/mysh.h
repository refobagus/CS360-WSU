#ifndef MYSH_H
#define MYSH_H

#include <stdio.h>
#include <stdlib.h>
#include <envz.h>
#include <string.h>
#include <fcntl.h> 

#define SHELL_NAME "RYshell"
#define LINE_MAX   1024

typedef struct path_t
{
    char* name;
    struct path_t* next;
} Path_t;

Path_t* pathList;            
char*   PATH;               
char*   HOME;              
char    line[LINE_MAX];   
char    command[64];        
char    head[512], tail[512];

void  initialize        (char** env);
char* strReverse   	(char* string);
char* removePre  	(char* string);
char* removePost 	(char* string);
int   tokenize     	(char* string, char* cmdArgs, char* file);
void  exe  		(char** env);

#endif
