#include "vfs/devfs.h"
#include "vfs/descriptor.h"
#include "vfs/fs.h"
#include "errno.h"
#include "task.h"
#include "printk.h"
#include <stdint.h>

 /* Writes data in buffer, count bytes to file descriptor fd */
uint32_t write(int fd, char* buffer, uint32_t size)
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

  desc->fs->write(desc->mountpoint, desc->fileinfo, buffer, size, 0x0);

  return 0;
}

/* Reads data from file descriptor fd, count bytes to buffer */
uint32_t read(int fd, char* buffer, uint32_t size)
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

  desc->fs->read(desc->mountpoint, desc->fileinfo, buffer, size, 0x0);

  return 0;
}
