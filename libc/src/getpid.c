#include "stdio.h"
#include "syscall.h"
#include "stdint.h"

int getpid ()
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0x8, (uint64_t)0x0);
  return ret;
}
