#ifndef __MOUNT__
#define __MOUNT__

#include "vfs/devfs.h"
#include <stdint.h>

#define MAX_MOUNTS  26

enum Riku_Mount_State { FREE, MOUNTED, RESERVED };
struct riku_filesystem;

struct riku_mountpoint {
  enum Riku_Mount_State state;
  struct riku_devfs_node* device;
  struct riku_filesystem* fs;
};

extern struct riku_mountpoint mounts[MAX_MOUNTS];
uint32_t mount_internal(struct riku_devfs_node* device, struct riku_filesystem* fs);

#endif
