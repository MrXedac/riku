#include "stdio.h"
#include "stdint.h"
#include "syscall.h"

int putc ( int character, FILE * stream )
{
    uint64_t ret;
    SYSCALL2(ret, (uint64_t)6, (uint64_t)(stream->fd), (uint64_t)character);
    return ret;
}
