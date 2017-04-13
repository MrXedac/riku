#ifndef __SERIAL_DRIVER__
#define __SERIAL_DRIVER__

#include <stdarg.h>
#include <stdint.h>
#include "printk.h"

extern uint8_t printk_enabled;

#define PORT 0x3f8


#define KTRACERAW(a) { slputs(a); }
#define KTRACE(a,...) { if(printk_enabled) printk(a, ##__VA_ARGS__); else slprintf(" [%s:%d] " a, __FILE__, __LINE__, ##__VA_ARGS__); }

void init_serial();
int serial_received();
char read_serial();
int is_transmit_empty();
void write_serial(char a);
void slputs(char* a);
void slputdec(int n);
void slputhex(int n);
void slprintf(char *fmt, ...);
void sprintdec(int n, char* buf);
#endif
