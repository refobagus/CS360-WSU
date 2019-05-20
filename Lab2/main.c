#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int (*fptr[])(char*) = {(int(*)())mkdir, rmdir, ls, cd, pwd, creat, rm, quit, menu, reload, save};

int main()
{
  int r, com;
  root = malloc(sizeof(NODE));
  strcpy(root->name, "/");
  root->type = 'D';
  root->siblingPtr = NULL;
  root->parentPtr = NULL;
  root->childPtr = NULL;
  
  cwd = root;

  printf("enter \"menu\" to get a list of commands.\n");
  while(1)
  {
    printf("=>");
    fgets(line, 128, stdin);
    sscanf(line, "%s %s", command, pathname);
    printf("****%s\n", line);
    com = findCmd(command);
    if(com != -1) 
	  r = fptr[com](pathname);
    else printf("err: command invalid\n");

    clearmem();
  }

  return 0;
}


