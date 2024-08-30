#ifndef __STAT__
#define __STAT__

#include <stdint.h>
#include "vfs/fs.h"

uint32_t stat(int fd, struct riku_stat* buffer);

#endif
