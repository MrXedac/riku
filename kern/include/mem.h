#ifndef __MEM_H__
#define __MEM_H__
#include <stdint.h>
#include <string.h>

void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t count);

#endif
