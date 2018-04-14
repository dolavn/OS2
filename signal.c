#include "param.h"
#include "signal.h"
#include "types.h"
#include "mmu.h"
#include "defs.h"
#include "proc.h"
#include "x86.h"

extern void* sigRetCall(void);
extern void* sigRetCallEnd(void);

void copyTF(struct trapframe* dst,struct trapframe* src){
  memmove(dst,src,sizeof(struct trapframe));
}

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
    if((p->tf->cs&3) != DPL_USER){
      return 0;
    }
    p->oldMask = setSigMask(-1);
    char bits[NUM_OF_SIGS];
    getAllSignals(p->pendingSigs,bits);
    for(int i=0;i<NUM_OF_SIGS;++i){
      if(bits[i]){
        if(p->sigHandlers[i]==(void*)SIG_DFL){ //kernel space handler
          switch(i){
              case SIGKILL:
                  handleKill();
                  break;
              case SIGSTOP:
                  handleStop();
                  break;
              default:
                  handleKill();
                  break;
          }
        }else{ //user space handler
          copyTF(p->usrTFbackup,p->tf);
          p->tf->eip = (uint)(p->sigHandlers[i]);
          cprintf("old eip:%p\nnew eip:%p\n",p->usrTFbackup->eip,p->tf->eip);
          int param = i;
          uint funcSize = sigRetCallEnd-sigRetCall;
          p->tf->esp = p->tf->esp-funcSize;
          memmove((void*)(p->tf->esp),sigRetCall,funcSize);
          void* sigFunc = (void*)p->tf->esp; //address to function on stack
          while(p->tf->esp%4!=0){p->tf->esp--;} 
          p->tf->esp = p->tf->esp-4;
          memmove((void*)(p->tf->esp),&param,4);
          p->tf->esp = p->tf->esp-4;
          memmove((void*)(p->tf->esp),&sigFunc,4);
        }
        uint pendingSigs = p->pendingSigs;
        turnOffBit(i,&pendingSigs);
        setPendingSignals(pendingSigs);
      }
    }
    setSigMask(p->oldMask);
  }
  return ret;
}

