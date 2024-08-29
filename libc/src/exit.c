#include <unistd.h>
#include <syscall.h>
#include <stdint.h>

void exit(int exitcode) 
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0xC, (uint64_t)exitcode);
}