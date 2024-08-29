#include <unistd.h>
#include <syscall.h>
#include <fileinfo.h>
#include <stdint.h>

int closedir(struct riku_fileinfo* dir)
{
    return 0;
}

int opendir(const char* path, struct riku_fileinfo* buffer)
{
    uint64_t ret;
    SYSCALL2(ret, (uint64_t)0xE, (uint64_t)path, (uint64_t)buffer);
    return ret;
}

int readdir(struct riku_fileinfo* dir, uint32_t offset, struct riku_fileinfo* buffer)
{
    uint64_t ret;
    SYSCALL3(ret, (uint64_t)0xF, (uint64_t)dir, (uint64_t)offset, (uint64_t)buffer);
    return ret;
}