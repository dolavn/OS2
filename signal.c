#include "param.h"
#include "signal.h"
#include "types.h"
#include "mmu.h"
#include "defs.h"
#include "proc.h"

int
handleKill(int) {
  struct proc *p;
  p = myproc();
  p->killed = 1;

}


int handleStop(int pid){
  struct proc* currProc = myproc();
  if(currProc->pid!=pid){
    panic("sig stop");
  }
  if(currProc->state==RUNNING || currProc->state==RUNNABLE){
    currProc->state=FROZEN;
    return 0;
  }
  return -1;
}

int handleCont(int pid){
  struct proc* currProc = myproc();
  if(currProc->pid!=pid){
    panic("sig stop");
  }
  if(currProc->state==FROZEN){
    currProc->state=RUNNABLE;
    return 0;
  }
  return -1;
}
