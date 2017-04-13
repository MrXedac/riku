#ifndef __PRINTK__
#define __PRINTK__

#include <stdint.h>
#include <serial.h>

#define PRINTK_JIFFIES_DIVISOR  100

extern uint8_t printk_enabled;

#define printk(a,...) { if(printk_enabled) printk_internal(a, ##__VA_ARGS__); else slprintf("[Early console in %s:%d] " a, __FILE__, __LINE__, ##__VA_ARGS__); }

/* Main late-boot kernel log entrypoint */
void printk_internal(char *fmt, ...);

#endif
