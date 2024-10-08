#ifndef __DESCRIPTOR__
#define __DESCRIPTOR__

#include <stdint.h>

enum riku_descriptor_state { INVALID, OPENED, CLOSED };

/* Opened file/device descriptor (can be shared by multiple tasks) */
struct riku_descriptor {
  enum riku_descriptor_state state; /* Descriptor state */
  struct riku_mountpoint* mountpoint;
  struct riku_devfs_node* device; /* Associated devfs node */
  struct riku_filesystem* fs; /* FS associated to the descriptor */
  struct riku_fileinfo* fileinfo;
  uint64_t offset;  /* Offset to seek */
  uint32_t clients; /* Amount of tasks associated to this descriptor */
  void* extended; /* Some extended, untyped data for special filesystems */
};

#endif
