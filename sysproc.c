#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"


//////////////////stam////////////////////////
static int sharedCounter=0;

int
sys_cascall(void) {
  // int counter;
  // if(argint(0,&counter)<0) return -1;
  // cprintf("%d\n", counter);
  int old;
  do{
    old = sharedCounter;
  }while (!cas(&sharedCounter, old, old+1)) ;

  // sharedCounter++;
  // cprintf("%d\n",cas(&counter, old, 2));
  // cprintf("%d\n",counter);
  return sharedCounter;
}
// static inline int sys_cas(volatile void *addr, int expected, int newval) {
//   int ans;
//   asm volatile("movl %1, %%eax;"//"": "a" (expected) : ", %eax\n\t"
//                "movl (%2), %%ebx;"//"": "d" (*addr) : "), %ebx\n\t"
//                "lock cmpxchg %%ebx, %3;"//"($"+addr+")\n\t"
//                "pushfl;"
//                "popl %%eax;"
//                "movl %%eax, %0;"
//                // get ZF from eax and return
//                : "=r" (ans)
//                : "r" (expected) , "r" (addr) , "r" (newval)
//                : "eax" , "ebx"
//                );
//   ans &= ZF_mask;
//   ans >>= ZF_pos;
//   return ans;
// }
////////////////////////////////////////////

int
sys_isStopped(void){
  int pid;
  if(argint(0,&pid) < 0){
    return -1;
  }
  return isStopped(pid);
}

int
sys_sigret(void) {
  sigret();
  return 0;
}

int
sys_signal(void){
  int signum;
  sighandler_t handler;
  if(argint(0,&signum)<0){
    return -1;
  }
  if(argptr(1,(char**)(&handler),sizeof(void*))<0){
    return (int)setSignalHandler(signum,(void*)SIG_IGN);  
  }
  return (int)setSignalHandler(signum,handler);
}

int
sys_sigprocmask(void){
  int newMask;
  if(argint(0,&newMask)<0){
    return -1;
  }
  return setSigMask(newMask);
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid,signum;

  if(argint(0, &pid) < 0)
    return -1;
  if(argint(1,&signum) < 0)
    return -1;
  return kill(pid,signum);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
