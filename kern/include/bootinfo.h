#ifndef __BOOTINFO__
#define __BOOTINFO__

#include <stdint.h>

void parse_mbi(uintptr_t mbi);
void start_modules(uintptr_t mbi);

#endif