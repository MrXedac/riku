#ifndef __PORT__
#define __PORT__

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
unsigned char inb(uint16_t port);

#endif
