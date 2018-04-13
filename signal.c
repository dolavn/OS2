#include "param.h"
#include "signal.h"
#include "types.h"
#include "mmu.h"
#include "defs.h"
#include "proc.h"

void getAllSignals(uint pendingSigs,char bits[NUM_OF_SIGS]){
  int currBit=0;
  while(pendingSigs>0){
    bits[currBit++]=pendingSigs%2==0?0:1;
    pendingSigs=pendingSigs/2;
  }
  while(currBit<NUM_OF_SIGS){
    bits[currBit++]=0;
  }
}

int handleSignal(){
  struct proc *p;
  p = myproc(); 
  int ret=1;
  if(p!=0){
    char bits[NUM_OF_SIGS];
    getAllSignals(p->pendingSigs,bits);
    for(int i=0;i<NUM_OF_SIGS;++i){
      if(bits[i]){
        if(p->sigHandlers[i]==SIG_DFL){
          switch(i){
              case SIGKILL:
                  handleKill(p->pid);
                  break;
              case SIGSTOP:
                  handleStop(p->pid);
                  break;
              case SIGCONT:
                  handleCont(p->pid);
                  break;
              default:
                  handleKill(p->pid);
                  break;
          }
        }else{
          ret=0;
        }
      }
    }
  }
  return ret;
}

int
handleKill(int pid) {
  struct proc *p;
  p = myproc();
  p->killed = 1;
  return 0;
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
