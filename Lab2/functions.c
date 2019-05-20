#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "quit", "menu", "reload", "save", "?", 0};

int findCmd(char *command)
{
  int i = 0;
  while(cmd[i])
  {
    if(!strcmp(command, cmd[i])) 
	return i;
    i++;
  }
  return -1;
}

int mkdir(char path[])
{
  if(strcmp(path, "") == 0)
  {
    printf("err: empty pathname\n");
    return 0;
  }
  else if(strcmp(path, "/") == 0) return 0;

  NODE *child, *parent;
  if(path[0] == '/') parent = root;
  else parent = cwd;
 
  split(path);
  
  char *mover = strtok(dirname, "/");
  
  while(mover != NULL)
  {
    child = parent->childPtr;
    while(1)
    {
      if(child == NULL)
      {
        return 0;
      }
      if(strcmp(child->name, mover) == 0 && 'D' == child->type)
      {
        parent = child;
        break;
      }
      
      child = child->siblingPtr;
    }
    mover = strtok(NULL, "/");
  }

  NODE *sibling = child = parent->childPtr;
  NODE *prevSibling;
  while(sibling != NULL)
  {
    if(strcmp(sibling->name, basename) == 0)
    {
      printf("err: pathname used\n");
      return 0;
    }
    prevSibling = sibling;
    sibling = sibling->siblingPtr;
  }
    
  if(child == NULL) 
  {
    child = malloc(sizeof(NODE));
    child->parentPtr = parent;
    strcpy(child->name, basename);
    child->type = 'D';
    child->siblingPtr = NULL;
    child->childPtr = NULL;

    parent->childPtr = child;
  }
  else
  {
    sibling = malloc(sizeof(NODE));
    sibling->parentPtr = parent;
    strcpy(sibling->name, basename);
    sibling->type = 'D';
    sibling->childPtr = NULL;
    sibling->siblingPtr = NULL;

    prevSibling->siblingPtr = sibling;
  }
}

void rmdir(char *path)
{
  if(strcmp(path, "") == 0)
  {
    printf("err: empty pathname\n");
    return 0;
  }
 
  split(path);

  NODE *child, *parent;
  if(dirname[0] == '/') parent = root;
  else parent = cwd;
  
  char *mover = strtok(dirname, "/");
  while(mover != NULL)
  {
    child = parent->childPtr;
    while(1)
    {
      if(child == NULL)
      {
        printf("err: pathname does not exist\n");
        return 0;
      }
      if(strcmp(child->name, mover) == 0 && 'D' == child->type)
      {
        parent = child;
        break;
      }
      
      child = child->siblingPtr;
    }
    mover = strtok(NULL, "/");
  }

  NODE *sibling = child = parent->childPtr;
  if(child != NULL)
  {
    if(strcmp(child->name, basename) == 0) 
    {
      if('F' == child->type)
      {
        printf("err: wrong file type\n");
        return 0;
      }
      if(child->childPtr == NULL)
      {
        parent->childPtr = child->siblingPtr;
      }
      else printf("err: dir not empty\n"); 
      return 0;
    }
    else
    {
      while(sibling != NULL)
      {
        if(strcmp(sibling->name, basename) == 0 && 'D' == sibling->type)
        {
          if(sibling->childPtr == NULL)
          {
            child->siblingPtr = sibling->siblingPtr;
          }
          else 
          { 
            printf("err: dir not empty\n"); 
          }
          return 0;
        }
        child = sibling;
        sibling = sibling->siblingPtr;
      }
    }
  }
  printf("err: dir does not exist\n");
}

int cd()
{
  if(strcmp(pathname, "") == 0 || strcmp(pathname, "/") == 0)
  {
    cwd = root;
    return 0;
  }

  NODE *parent, *child;
  if(pathname[0] == '/') parent = root;
  else parent = cwd;
  child = parent->childPtr;

  char *mover = strtok(pathname, "/");
  while(mover != NULL)
  {
    while(1)
    {
      if(strcmp(mover,"..")==0){
        parent = parent->parentPtr;
        break;
      }
      if(child == NULL)
      {
        printf("err: pathname does not exist\n");
        return 0;
      }
      if(strcmp(child->name, mover) == 0 && 'D' == child->type)
      {
        parent = child;
        break;
      }
      child = child->siblingPtr;
    }
    child = parent->childPtr;
    mover = strtok(NULL, "/");
  }

  cwd = parent;
}

int ls()
{
  NODE *child, *parent;
  if((strcmp(pathname, "") == 0) || pathname[0] != '/') parent = cwd;
  else parent = root;
  
  char *mover = strtok(pathname, "/");
  child = parent->childPtr;
  while(mover != NULL)
  {
    while(1)
    {
      if(child == NULL)
      {
        printf("err: pathname does not exist\n");
        return 0;
      }
      if(strcmp(child->name, mover) == 0)
      {
        parent = child;
        break;
      }
      child = child->siblingPtr;
    }
    child = parent->childPtr;
    mover = strtok(NULL, "/");
  }
  
  child = parent->childPtr;
  while(child != NULL)
  {
    printf("   %c\t%s\n", child->type, child->name);
    child = child->siblingPtr;
  }
}


int save()
{
  file = fopen("myfile", "w+");   

  if(file == NULL)
  {
    printf("err: file not created\n");
    return 0;
  }

  preOrderWrite(root->childPtr);
  
  fclose(file);
}

void preOrderWrite(NODE* node)
{
  if(node == NULL)
  {
    return 0;
  }
  
  fprintf(file, "%c\t", node->type);

  NODE *temp = node;
  Stack *top = malloc(sizeof(Stack)), *pusher;
  strcpy(top->name, temp->name);
  top->prev = NULL;

  while(temp->parentPtr != NULL)
  {   
    temp = temp->parentPtr;

    if(temp != NULL)
    {
      pusher = malloc(sizeof(Stack));
      strcpy(pusher->name, temp->name);

      pusher->prev = top;
      top = pusher;
    }
  }
  
  top = top->prev;

  while(top != NULL)
  {
    fprintf(file, "/%s", top->name);
    top = top->prev;
  }
  fprintf(file, "\n");

  if(node->childPtr != NULL) 
  {
    preOrderWrite(node->childPtr);
  }
  if(node->siblingPtr != NULL)
  {
    preOrderWrite(node->siblingPtr);
  }
}

int pwd()
{
  printf("   ");
  rpwd(cwd);
  printf("\n");
}

int rpwd(NODE *node)
{
  if(strcmp(node->name, "/"))
  {
    rpwd(node->parentPtr);
    printf("%s/", node->name);
  }
  else printf("%s", node->name);
}

int creat(char path[])
{
  if(strcmp(path, "") == 0)
  {
    printf("err: empty filename\n");
    return 0;
  }

  NODE *child, *parent;
  if(path[0] == '/') parent = root;
  else parent = cwd;

  split(path);

  char *mover = strtok(dirname, "/");
  
  while(mover != NULL)
  {
    child = parent->childPtr;
    while(1)
    {
      if(child == NULL)
      {
        printf("err: pathname does not exist\n");
        return 0;
      }
      if(strcmp(child->name, mover) == 0 && 'D' == child->type)
      {
        parent = child;
        break;
      }
      
      child = child->siblingPtr;
    }
    mover = strtok(NULL, "/");
  }

  child = parent->childPtr;
  if(child == NULL)
  {
    // Create file node
    child = malloc(sizeof(NODE));
    strcpy(child->name, basename);
    child->type = 'F';
    child->parentPtr = parent;
    child->childPtr = NULL;
    child->siblingPtr = NULL;
    parent->childPtr = child;
    return 0;
  }
  NODE *sibling;
  while(child != NULL)
  {
    if(strcmp(child->name, basename) == 0)
    {
      printf("err: filename exist\n");
      return 0;
    }  
    sibling = child;
    child = child->siblingPtr;  
  }

  child = malloc(sizeof(NODE));
  strcpy(child->name, basename);
  child->type = 'F';
  child->parentPtr = parent;
  child->siblingPtr = NULL;
  child->childPtr = NULL;
  sibling->siblingPtr = child;
}

int split(char *path)
{
  char temp[64];
  strcpy(temp, path);
  char *mover = strtok(temp, "/");
  while(mover != NULL)
  {
    strcpy(basename, mover);
    mover = strtok(NULL, "/");
  }

  if(strlen(path) - strlen(basename) == 0)
  { 
	strncpy(dirname, path, strlen(path) - strlen(basename));
  }
  else strncpy(dirname, path, strlen(path) - strlen(basename) - 1);
}

int rm(char *path)
{
  if(strcmp(path, "") == 0)
  {
    printf("err: pathname empty\n");
    return 0;
  }

  NODE *child, *parent;
  if(path[0] == '/') parent = root;
  else parent = cwd;

  split(path);
  
  char *mover = strtok(dirname, "/");
  while(mover != NULL)
  {
    child = parent->childPtr;
    while(1)
    {
      if(child == NULL)
      {
        printf("err: pathname does not exist\n");
        return 0;
      }
      if(strcmp(child->name, mover) == 0 && 'D' == child->type)
      {
        parent = child;
        break;
      }
      
      child = child->siblingPtr;
    }
    mover = strtok(NULL, "/");
  }

  child = parent->childPtr;
  NODE *sibling = child->siblingPtr;
  if(child != NULL)
  {
    if(strcmp(child->name, basename) == 0) 
    {
      if('D' == child->type)
      {
        printf("err: wrong file type\n");
        return 0;
      }
      parent->childPtr = child->siblingPtr;
      return 0;
    }
    else
    {
      while(sibling != NULL)
      {
        if(strcmp(sibling->name, basename) == 0 && 'F' == sibling->type)
        {
          child->siblingPtr = sibling->siblingPtr;
          return 0;
        }
        child = sibling;
        sibling = sibling->siblingPtr;
      }
    }
  }
  printf("err: file does not exist\n");
}

int reload()
{
  file = fopen("myfile", "r");

  if(file == NULL)
  {
    printf("err: cannot open file\n");
    return 0;
  }
  
  char type, path[100], line[100];
  while(1)
  {

    fgets(line, 100, file);

    if(feof(file)) break;

    line[strlen(line)] = 0; line[strlen(line)-1] = 0;
    if(line != NULL || strcmp(line, "") != 0)
    {

      char *splitter = strtok(line, "\t");
      type = *splitter;
      splitter = strtok(NULL, "\t");
      strcpy(path, splitter);
      switch(type)
      {
        case 'F':
          creat(path);
        break;
        case 'D':
          mkdir(path);
        break;
      }
    }
    clearmem();
  }
  fclose(file);
}

int menu()
{
  	printf("mkdir [pathname]	: creates new directory if it doesn't exist.\n");
  	printf("rmdir [pathname]	: removes directory if it is empty.\n");
  	printf("cd [pathname]		: change current directory.\n");
  	printf("ls [pathname]		: list the directory contents.\n");
  	printf("pwd			: print the (absolute) pathname of the current directory.\n");
  	printf("creat [pathname]	: create a file.\n");
  	printf("rm [pathname]		: remove a file.\n");
  	printf("save [filename]		: save the current file system tree in.\n");
  	printf("reload [filename]	: re-initalize the file system tree from a.\n");
	printf("menu			: show a menu of valid commands\n");
  	printf("quit			: save the file system tree, then terminate the program.\n");
}

int quit()
{
  save();
  printf("Output Saved\n");
  exit(0);  
}

int clearmem()
{
  memset(dirname, 0, sizeof(dirname));
  memset(pathname, 0, sizeof(pathname));
  memset(basename, 0, sizeof(basename));
}
