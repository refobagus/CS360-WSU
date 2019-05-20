#include "queue.c"

int tswitch();

int sleep(int event)
{
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
}

int wakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
     if (p->event == event){
        printf("wakeup %d\n", p->pid);
        p->status = READY;
        enqueue(&readyQueue, p);
     }
     else{
	      enqueue(&temp, p);
     }
  }
  sleepList = temp;
  printList("sleepList", sleepList);
}

int kexit(int exitValue)
{
  if(exitValue != 1){
    printf("proc %d in kexit(%d)\n", running->pid, exitValue);
    running->exitCode = exitValue;          //record exitValue in proc's exitCode
    running->status = ZOMBIE;               //become a ZOMBIE
    
    if(running->child){
      PROC *runChild = running->child; //point to running child
      PROC *p1child = P1->child;   //point to p1's child
      if(p1child){//give away children, if any, to P1
        while(p1child->sibling){         
          p1child = p1child->sibling; 
        }

        p1child->sibling = runChild;          //combine

        while(runChild){                      //set runChild parent ptr to p1
          runChild->ppid = P1->pid;
          runChild->parent = P1;
          runChild = runChild->sibling;
        }
        running->child = NULL;
      }
      else{
      }
    }
    wakeup(running->ppid);         //wakeup parent (by parent proc address)
    tswitch();
  }
  else{
  }
}

int letgo(PROC *ptrChild, PROC *ptrPrevChild){
   if(!ptrChild->sibling){                        
      running->child = NULL;
    }
    else if(running->child == ptrChild){          
      running->child = ptrChild->sibling;         
    }
    else{                                         
      ptrPrevChild->sibling = ptrChild->sibling;  
    }
}

int freeZomb(PROC *ptr){
  ptr->sibling = NULL;   
  ptr->parent = NULL;//remove all connection
  ptr->ppid = 0;         
  ptr->status = FREE;
}

int wait(int *status){
  int zomBid;

  if(running->child){
    PROC *child = running->child;
    PROC *prev = running;
    while(1){
      if(child->status == ZOMBIE){  //found zombie child
        zomBid = child->pid;            
        (*status) = child->exitCode;    //copyt ZOMBIE child exitCode to *status
        letgo(child, prev);       //unlink
        freeZomb(child);        //bury
        enqueue(&freeList,child);       //put into freeList
        wakeup(running->ppid);
        return zomBid;//return pid
      } 
      if(!child->sibling){              
        child = running->child;         
      }
      else{                        
        prev = child;                       
        child = child->sibling;
      }
      sleep(running->pid);
    }
  }
  else{
    return -1; //if no child
  }      
}

