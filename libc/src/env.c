#include <unistd.h>
#include <syscall.h>
#include <stdint.h>

int cwd(char* wd)
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0x11, (uint64_t)wd);
  return ret;
}

int gwd(char* buf)
{
  uint64_t ret;
  SYSCALL1(ret, (uint64_t)0x12, (uint64_t)buf);
  return ret;
}