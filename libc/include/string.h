#ifndef __STRING__
#define __STRING__

#include "stdint.h"
#include "stdio.h"

char *strcpy(char* dest, const char* source);
char *strcat(char *dest, const char *src);
int strcmp(const char* s1, const char* s2);
size_t strlen(const char * str);

#endif
