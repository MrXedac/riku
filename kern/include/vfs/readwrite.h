#ifndef __READWRITE__
#define __READWRITE__

#include <stdint.h>

uint32_t write(int fd, char* buffer, uint32_t size); /* Writes data in buffer, count bytes to file descriptor fd */
uint32_t read(int fd, char* buffer, uint32_t size); /* Reads data from file descriptor fd, count bytes to buffer */

#endif
