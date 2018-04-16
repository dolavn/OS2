#include "param.h"
#include "signal.h"
#include "types.h"
#include "mmu.h"
#include "defs.h"
#include "proc.h"
#include "x86.h"

extern void* sigRetCall(void);
extern void* sigRetCallEnd(void);

void printTF(struct trapframe* tf){
  cprintf("--TRAP FRAME-- \neip:%p\neax:%d\nebx:%d\necx:%d\nedx:%d\nesp:%p\nebp:%p\n",tf->eip,tf->eax,tf->ebx,tf->ecx,tf->edx,tf->esp,tf->ebp);
  cprintf("--END OF TRAP FRAME--\n");
}

void copyTF(struct trapframe* dst,struct trapframe* src){
  //*dst=*src;
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

void handleSignal(struct trapframe* tf){
  struct proc *p;
  p = myproc(); 
  if(p!=0){
//     if(p->sigret==12){
//       cprintf("handler again\n");
//       printTF(p->tf);
//     }
    if (((tf->cs) & 3) != DPL_USER) {
      return;
    }
    //char bits[NUM_OF_SIGS];
    //getAllSignals(p->pendingSigs,bits);
    for(int i=0;i<NUM_OF_SIGS;++i){
      if((1<<i&p->pendingSigs) && !((1<<i)&p->sigMask)){
        p->oldMask = setSigMask(-1);  
        if(p->sigHandlers[i]==(void*)SIG_DFL){ //kernel space handler
            switch(i){
                case SIGKILL:
                    handleKill();
                    break;
                case SIGSTOP:
                    handleStop();
                    break;
                case SIGCONT:
                    break;
                default:
                    handleKill();
                    break;
            }
            setSigMask(p->oldMask);
            uint pendingSigs = p->pendingSigs;
            turnOffBit(i,&pendingSigs);
            setPendingSignals(pendingSigs);
        }else{ //user space handler
            copyTF(p->usrTFbackup,p->tf);
            p->tf->eip = (uint)(p->sigHandlers[i]);
            //cprintf("before user handler:\n");
            //printTF(p->tf);
            int param = i;
            uint funcSize = sigRetCallEnd-sigRetCall;
            p->tf->esp = p->tf->esp-funcSize;
            memmove((void*)(p->tf->esp),sigRetCall,funcSize);
            void* sigFunc = (void*)p->tf->esp; //address to function on stack
            //while(p->tf->esp%4!=0){p->tf->esp--;} 
            p->tf->esp = p->tf->esp-4;
            memmove((void*)(p->tf->esp),&param,4);
            p->tf->esp = p->tf->esp-4;
            memmove((void*)(p->tf->esp),&sigFunc,4);
            uint pendingSigs = p->pendingSigs;
            turnOffBit(i,&pendingSigs);
            setPendingSignals(pendingSigs);
            p->handlingSignal=1;
            return;
        }
      }
    }
//     if(p->sigret==12){
//       cprintf("after handling\n");
//       printTF(p->tf);
//     }
  }
  return;
}