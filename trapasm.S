#include "mmu.h"
#include "traps.h"
  # vectors.S sends all traps here.
.globl alltraps
alltraps:
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  pushl %esp
  call trap
  addl $4, %esp

  # Return falls through to trapret...
.globl trapret
trapret:
  push %eax #saves value of eax
  call handleSignal
  pop %eax #retrieves value of eax
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret
  
.globl sigRetCall
.globl sigRetCallEnd

sigRetCall:
  movl $24,%eax
  int $64
sigRetCallEnd:
