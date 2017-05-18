#include "string.h"
#include "stdio.h"
#include "stdint.h"

char *strcat(char *dest, const char *src)
{
	size_t i,j;
	for (i = 0; dest[i] != '\0'; i++);

	for (j = 0; src[j] != '\0'; j++)
		dest[i+j] = src[j];

	dest[i+j] = '\0';
	return dest;
}

int strcmp(const char* s1, const char* s2)
{
	while(*s1 && (*s1==*s2))
		s1++,s2++;
	return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

size_t strlen(const char * str)
{
	const char *s;
	for (s = str; *s; ++s) {}
	return(s - str);
}
