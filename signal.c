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
  memmove(dst,src,sizeof(struct trapframe));
}

int
handleKill() {
  struct proc *p = myproc();
  p->killed=1;
  return 0;
}


int handleStop(){
  struct proc* currProc = myproc();
  if((currProc->state==RUNNING || currProc->state==RUNNABLE) && !currProc->frozen){
    currProc->frozen=1;
    int contFlag = 1<<SIGCONT;
    int flag=1;
    while(flag==1){
      if((currProc->pendingSigs&contFlag)==0){
        yield();
      }else{
        currProc->frozen=0;
        turnOffBit(SIGSTOP,currProc);
        turnOffBit(SIGCONT,currProc);
        flag=0;
      }
    }
    return 0;
  }
  return -1;
}

int handleCont(){
  struct proc* currProc = myproc();
  if(currProc->frozen){
    currProc->frozen=0;
    return 0;
  }
  return -1;
}


sighandler_t
setSignalHandler(int signum,sighandler_t handler){
  struct proc *curproc = myproc();
  sighandler_t ans = curproc->sigHandlers[signum];
  curproc->sigHandlers[signum] = handler;
  return ans;
}


uint
setSigMask(uint mask){
  struct proc *curproc = myproc();
  uint ans = curproc->sigMask;
  curproc->sigMask = mask;
  return ans;
}

int sigret(void) {
  struct proc *p = myproc();
  copyTF(p->tf,p->usrTFbackup);
  p->usrTFbackup = 0;
  p->handlingSignal = 0;
  return p->tf->eax;
}

void turnOffBit(int bit,struct proc* p){
  int operand = 1;
  operand<<=bit;
  operand = ~operand;
  int pending = p->pendingSigs;
  while(!cas(&p->pendingSigs,pending,pending&operand)){pending=p->pendingSigs;}
}

void handleUserSignal(struct proc* p,int signum){
    uint esp = p->tf->esp - sizeof(struct trapframe);
    p->usrTFbackup = (struct trapframe*)esp;
    copyTF(p->usrTFbackup,p->tf);
    p->tf->esp = esp;
    p->tf->eip = (uint)(p->sigHandlers[signum]);
    uint funcSize = sigRetCallEnd-sigRetCall;
    p->tf->esp = p->tf->esp-funcSize;
    memmove((void*)(p->tf->esp),sigRetCall,funcSize);
    void* sigFunc = (void*)p->tf->esp; //address to function on stack
    while(p->tf->esp--%4!=0);
    p->tf->esp = p->tf->esp-4;
    memmove((void*)(p->tf->esp),&signum,4);
    p->tf->esp = p->tf->esp-4;
    memmove((void*)(p->tf->esp),&sigFunc,4);
    turnOffBit(signum,p);
    p->handlingSignal=1;
    return;
}

void handleSignal(struct trapframe* tf){
  struct proc *p;
  p = myproc(); 
  if(p!=0){
    if (((tf->cs) & 3) != DPL_USER) {
      return;
    }
    if(p->handlingSignal){
      return;
    }
    for(int i=0;i<NUM_OF_SIGS;++i){
      if((1<<i&p->pendingSigs) && !((1<<i)&p->sigMask)){
        if(p->sigHandlers[i]==(void*)SIG_IGN){continue;}
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
            turnOffBit(i,p);
        }else{ //user space handler
            handleUserSignal(p,i);
            return;
        }
      }
    }
  }
  return;
}