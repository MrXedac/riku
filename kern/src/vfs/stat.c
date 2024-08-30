#include "vfs/descriptor.h"
#include "vfs/devfs.h"
#include "vfs/fs.h"
#include "vfs/mount.h"
#include "heap.h"
#include "mem.h"
#include "errno.h"
#include "printk.h"
#include "task.h"
#include <stdint.h>

uint32_t stat(int fd, struct riku_stat* buffer)
{
  /* Check file descriptor bounds */
  if(fd >= MAX_FILES)
    return -ENOFILE;

  /* Check file descriptor state */
  struct riku_descriptor* desc = current_task->files[fd];
  if(desc == 0x0)
    return -ECLOSED;
  if(desc->state != OPENED)
    return -ECLOSED;

  strcpy(buffer->name, desc->fileinfo->name);
  buffer->size = desc->fileinfo->size;

  return 0;
}