#include <string.h>
#include <stdint.h>

void* memset(void* bufptr, int value, size_t size)
{
    unsigned char* buf = (unsigned char*) bufptr;
    for ( size_t i = 0; i < size; i++ )
        buf[i] = (unsigned char) value;
    return bufptr;
}

char *strcpy(char* dest, const char* source) {
    int i = 0;
    while ((dest[i] = source[i]) != '\0') {
        i++;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;

    while (count--) {
        *dst8++ = *src8++;
    }
    return dest;
}

char *strcat(char *dest, const char *src)
{
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++);

    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];

    dest[i+j] = '\0';
    return dest;
}

int memcmp (const char* str1, const char* str2, size_t count)
{
    const unsigned char *s1 = (const unsigned char*)str1;
    const unsigned char *s2 = (const unsigned char*)str2;

    while (count-- > 0)
    {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
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
