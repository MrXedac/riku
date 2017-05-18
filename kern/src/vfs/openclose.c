#include "vfs/descriptor.h"
#include "vfs/devfs.h"
#include "heap.h"
#include "mem.h"
#include "errno.h"
#include "printk.h"
#include "task.h"
#include <stdint.h>

/* Opens a file/device descriptor at file, with mode mode */
uint32_t open(const char* file, uint8_t mode)
{
  /* Find devfs node name */
  /* FIXME : this is dirty as hell */
  int offset = 0;
  /* Parse file name to squash the devfs:/ thing, if present */
  while(file[offset] != '\0')
  {
    offset++;
    /* Found devfs's / : increment offset and break */
    if(file[offset] == '/'){
      offset++;
      break;
    }
  }

  /* Right now we only support devfs:/, and "devfs:/" doesn't take more than 16 characters eh */
  char buffer[16];
  if(offset >= 16)
    return -ENOENT;

  /* Check for devfs:/ correctness */
  memcpy(buffer, file, offset);
  buffer[offset+1] = '\0';

  if(strcmp(buffer, "devfs:/"))
    return -ENOENT;

  /* We have something following devfs:/ : continue */
  const char* nodename = file + offset;

  struct riku_devfs_node* node = devfs_find_node((char*)nodename);
  if(node)
  {
    /* Find first free descriptor */
    uint32_t i = 0;
    while(current_task->files[i] != 0x0 && i < MAX_FILES)
      i++;

    if(i == MAX_FILES) {
      return -EMAXFD;
    } else {
      /* Open descriptor, and add it to the task */
      struct riku_descriptor* desc = (struct riku_descriptor*)kalloc(sizeof(struct riku_descriptor));
      desc->state = OPENED;
      desc->device = node;
      desc->clients = 1;

      current_task->files[i] = desc;
      return i;
    }
  } else return -ENOENT;

}

/* Closes file descriptor fd */
uint32_t close(uint32_t fd)
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

  /* Invalidate descriptor */
  current_task->files[fd] = 0x0;

  desc->clients--;

  /* No more clients connected to descriptor : free it */
  if(desc->clients == 0x0)
    kfree((void*)desc);

  return 0;
}
