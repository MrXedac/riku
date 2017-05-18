#include "stdio.h"
#include "syscall.h"

int close (int filedes)
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0x3, (uint64_t)filedes);
  return ret;
}
