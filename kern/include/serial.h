#ifndef __SERIAL_DRIVER__
#define __SERIAL_DRIVER__

#define PORT 0x3f8

void init_serial();
int serial_received();
char read_serial();
int is_transmit_empty();
void write_serial(char a);
void slputs(char* a);
void slputdec(int n);
void slputhex(int n);

#endif