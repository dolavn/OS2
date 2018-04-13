#include "signal.h"
#include "proc.h"

int
handleKill(int) {
  struct proc *p;
  p = myproc();
  p->killed = 1;
  
}
