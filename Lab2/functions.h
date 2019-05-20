#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node{
  struct node *childPtr, *siblingPtr, *parentPtr;
  char name[64];
  char type;
}NODE;

typedef struct stack{
  struct stack *prev;
  char name[64];
}Stack;

NODE *root, *cwd;
char line[128], command[16], pathname[64],
     dirname[64], basename[64];

FILE* file;

int findCmd(char *command);
int mkdir(char path[]);
void rmdir(char *path);
int cd();
int ls();
int save();
void preOrderWrite(NODE* node);
int pwd();
int rpwd(NODE *node);
int creat(char path[]);
int split(char *path);
int rm(char *path);
int reload();
int menu();
int quit();
int clearmem();

