#ifndef __VGA_H__
#define __VGA_H__

#include <stdint.h>

void puts(char* c);
void cls();
void putdec(uintptr_t n);
void puthex(uintptr_t n);

#endif
