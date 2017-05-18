#include "stdio.h"
#include "syscall.h"

int open (const char *filename, int flags)
{
  uint64_t ret;
  SYSCALL2(ret, (uint64_t)0x2, filename, (uint64_t)flags);
  return ret;
}
