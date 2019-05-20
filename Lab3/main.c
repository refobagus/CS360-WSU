#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mysh.h"
int main (int argc, char* args[], char* env[])
{
    char *token;                
    char tmpString[LINE_MAX];   
    int  pid, status; 

    system("clear");
    initialize(env);
    printf("****************************** Welcome to RYshell *****************************\n\n");
    while (1)
    {
        printf("RYshell:~$ ");
        fgets(line, LINE_MAX - 1, stdin);

        // empty loop
        short len = strlen(line);
        if (len==0) 
	{ 
		continue; 
	}
        line[len - 1] = 0;

        strcpy(tmpString, line);
        token = strtok(tmpString, " ");
        if (token==NULL) 
	{ 
		continue; 
	}
        strcpy(command, token);

	// do simple commands
        if (strcmp(command, "exit") == 0)
        {
            fprintf(stderr, "PROC %s %d exits\n", SHELL_NAME, getpid());
            exit(0);
        }
        else if (strcmp(command, "cd")== 0)
        {
            token = strtok(NULL, " ");
            // Home directory
            if (token == NULL)
            {
                chdir(HOME);
                fprintf(stderr, "PROC %d cd to HOME\n", getpid());
            }
            // if argument
            else
            {
                if (chdir(token) == 0) 
		{ 
			fprintf(stderr, "PROC %d cd to %s OK\n", getpid(), token); 
		}
                else { fprintf(stderr, "PROC %d cd to %s FAILED!\n", getpid(), token); }
            }
        }
        else
        {
            // do fork
            pid = fork();
            if (pid < 0)
            {
                fprintf(stderr, "Fork failed!");
                exit(-1);
            }
            if (pid)
            {
                fprintf(stderr, "parent %s PROC %d forks a child process %d\n", SHELL_NAME, getpid(), 			pid);
                fprintf(stderr, "parent sh PROC %d waits\n", SHELL_NAME, getpid());
                pid = wait(&status);
                fprintf(stderr, "child sh PROC %d died : exit status = %04x\n", SHELL_NAME, pid, 			status);
            }
            else
            {
                fprintf(stderr, "PROC %d: line=%s\n", getpid(), line);
                strcpy(head, line);
                strcpy(tail, "");
                exe(env);

                exit(0);
            }
        }
    }

    return 0;
}
