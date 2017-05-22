#include "vfs/putgetc.h"
#include "vfs/devfs.h"
#include "vfs/descriptor.h"
#include "task.h"
#include "printk.h"
#include "errno.h"
#include <stdint.h>

uint32_t putc(int fd, char c)
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

  desc->device->putch(desc->device, c);

  return 0;
}
