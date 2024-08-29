#include <unistd.h>
#include <syscall.h>
#include <stdint.h>

int wait(int pid) 
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0xD, (uint64_t)pid);
  return ret;
}