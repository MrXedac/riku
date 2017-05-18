#ifndef __STDLIB__
#define __STDLIB__

#include "stdio.h"

void* memset(void* bufptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t count);

#endif
