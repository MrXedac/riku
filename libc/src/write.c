#include "stdio.h"
#include "syscall.h"

ssize_t write (int filedes, const void *buffer, size_t size)
{
  uint64_t ret;
  SYSCALL3(ret, (uint64_t)0x5, (uint64_t)filedes, buffer, (uint64_t)size);
  return (ssize_t)ret;
}
