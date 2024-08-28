#include <unistd.h>
#include <syscall.h>
#include <stdint.h>

int execve(char* name, char** argv, char** env) 
{
  uint64_t ret;
  SYSCALL3(ret, (uint64_t)0xB, (uint64_t)name, (uint64_t)argv, (uint64_t)env);
  return ret;
}