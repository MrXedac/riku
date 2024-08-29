#ifndef __DIR__
#define __DIR__

#include "vfs/fs.h"

int closedir(struct riku_fileinfo* dir);
int opendir(const char* path, struct riku_fileinfo* buffer);
int readdir(struct riku_fileinfo* dir, uint32_t offset, struct riku_fileinfo* buffer);

#endif