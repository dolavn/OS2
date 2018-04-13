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

void turnOffBit(int bit,uint* pendingSigs){
  int operand = 1;
  operand<<=bit;
  operand = ~operand;
  *pendingSigs&=operand;
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
                  handleKill();
                  break;
              case SIGSTOP:
                  handleStop();
                  break;
              case SIGCONT:
                  handleCont();
                  break;
              default:
                  handleKill();
                  break;
          }
        }else{
          ret=0;
        }
        uint pendingSigs = p->pendingSigs;
        turnOffBit(i,&pendingSigs);
        setPendingSignals(pendingSigs);
      }
    }
  }
  return ret;
}

