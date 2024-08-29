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

/* Opens a file/device descriptor at file, with mode mode */
uint32_t open(const char* file, uint64_t mode)
{
  /* Find devfs node name */
  /* FIXME : this is dirty as hell */
  int offset = 0;

  /* Find drive letter and get mountpoint */
  char drvLetter = file[0];
  printk("Attenmpting to open %c\n", drvLetter);
  if(drvLetter < 'A' || drvLetter > 'Z') return -ENOFILE;

  /* Mountpoints definition */
  extern struct riku_mountpoint mounts[MAX_MOUNTS];

  /* Find expected offset */
  int drvOffset = drvLetter - 'A';

  printk("Checking for device at offset %d\n", drvOffset);
  /* Only continue if we have a valid device descriptor and filesystem */
  if((!mounts[drvOffset].device) || (!mounts[drvOffset].fs)) return -ENOFILE;

  char* relPath = (char*)file + 3; /* Remove X:/ from path */
  printk("Relpath=%s\n", relPath);

  /* Allocate result structure */
  struct riku_fileinfo* retInfo = (struct riku_fileinfo*)kalloc(sizeof(struct riku_fileinfo));

  int ret = mounts[drvOffset].fs->open(&mounts[drvOffset], relPath, retInfo);
  
  /* ret = 0 : open success, else open failed */
  if(!ret)
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
      desc->device = mounts[drvOffset].device;
      desc->fs = mounts[drvOffset].fs;
      desc->fileinfo = retInfo;
      desc->mountpoint = &mounts[drvOffset];
      desc->clients = 1;
      desc->extended = retInfo->extended; /* Set extended here :D */

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
