#ifndef __OPENCLOSE__
#define __OPENCLOSE__

uint32_t open(const char* file, uint8_t mode); /* Opens a file/device descriptor at file, with mode mode */
uint32_t close(uint32_t fd); /* Closes file descriptor fd */

#endif
