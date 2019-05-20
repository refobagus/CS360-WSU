#include "type.h"
PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs 
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer

PROC *sleepList;       // list of SLEEP procs
PROC *P1;          // to always point at P1
char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};
/***************** queue.c file *****************/
int enqueue(PROC **queue, PROC *p) 
{
  PROC *q = *queue;
  if (q == 0 || p->priority > q->priority){
     *queue = p;
     p->next = q;
  }
  else{
     while (q->next && p->priority <= q->next->priority)
            q = q->next;
     p->next = q->next;
     q->next = p;
  }
}
PROC *dequeue(PROC **queue) 
{
     PROC *p = *queue;
     if (p)
        *queue = (*queue)->next;
     return p;
}
int printList(char *name, PROC *p) 
{
  printf("%s = ", name);
  while(p){
     printf("[%d %d]->", p->pid, p->priority);
     p = p->next;
  }
  printf("NULL\n");
}

int printSleep(char *name, PROC *p) 
{
  printf("%s = ", name);
  while(p){
     printf("[%d event=%d]->", p->pid, p->event);
     p = p->next;
  }
  printf("NULL\n");
}

int printChild(char *words, PROC *child)
{
   printf("%s = ", words);
   while(child){
      printf("[%d %s]->", child->pid, status[child->status]);
      child = child->sibling;
   }
   putchar('\n');
}

