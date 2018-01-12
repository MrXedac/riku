#ifndef __KERN_STRING__
#define __KERN_STRING__

#include <stdint.h>

typedef unsigned int size_t;

char *strcat(char *dest, const char *src);
size_t strlen(const char * str);
int strcmp(const char* s1, const char* s2);
void* memcpy(void* dest, const void* src, size_t count);
char *strcpy(char* dest, const char* source);
void* memset(void* bufptr, int value, size_t size);

#endif
