#include "stdio.h"
#include "syscall.h"

int fork ()
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0xA, (uint64_t)0x0);
  return ret;
}
