#include "vfs/dup2.h"
#include "vfs/devfs.h"
#include "vfs/descriptor.h"
#include "errno.h"
#include "task.h"
#include <stdint.h>

/* Duplicated fd into a free file descriptor */
uint32_t dup2(uint32_t fd)
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

  /* Find first free descriptor */
  uint32_t i = 0;
  while(current_task->files[i] != 0x0 && i < MAX_FILES)
    i++;

  if(i == MAX_FILES) {
    return -EMAXFD;
  } else {
    current_task->files[i] = desc;
    desc->clients++;
    return i;
  }
}
