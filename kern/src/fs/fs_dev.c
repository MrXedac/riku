#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>

int devfs_init(struct riku_mountpoint* self)
{
  printk("devfs_init: checking sanity of devfs.\n");
  struct riku_devfs_node *tmp = devfs_find_node("null");
  if(tmp)
    return 0;
  else return EFSERR;
}

int devfs_opendir(struct riku_mountpoint* self, const char* directory, struct riku_fileinfo* desc)
{
  extern struct riku_devfs_node *devfsVirtPtr;
  if(!desc)
    return EWRPTR;

  desc->diroff = 0x0;
  desc->flags = FLAGS_DIRECTORY;
  desc->extended = (void*)devfsVirtPtr;
  /* Right now, we only have files in devfs - simplify this and don't use opendir, only use readdir */
  return 0;
}

int devfs_readdir(struct riku_mountpoint* self, struct riku_fileinfo* desc, uint32_t offset, struct riku_fileinfo* result)
{
  /* Get first entry in devfs - devfs:/null */
  struct riku_devfs_node *tmp = devfs_find_node("null");
  uint64_t trueOffset;
  if(desc)
    trueOffset = desc->diroff;
  else
    trueOffset = offset;

  uint32_t i = 0;
  while(i < trueOffset)
  {
    tmp = tmp->next;
    if(!tmp)
      return ENMFIL;

    i++;
  }

  /* If we're readdir'ing through a descriptor, increment directory offset */
  if(desc)
    desc->diroff++;

  /* Put node into FileInfo structure */
  result->extended = (void*)tmp;
  return 0;
}

int devfs_write(struct riku_mountpoint* self, struct riku_fileinfo* file, const char*  buffer, uint64_t length, uint64_t offset)
{
  printk("devfs_write\n");
    return 0;
}

int devfs_read(struct riku_mountpoint* self, struct riku_fileinfo* file, char*  buffer, uint64_t length, uint64_t offset)
{
  printk("devfs_read\n");
    return 0;
}

int devfs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{
  printk("devfs_open\n");
    return 0;
}

int devfs_close(struct riku_mountpoint* self, struct riku_fileinfo* file)
{
  printk("devfs_close\n");
    return 0;
}

struct riku_filesystem fs_devfs =
{
  .init = devfs_init,
  .opendir = devfs_opendir,
  .readdir = devfs_readdir,
  .write = devfs_write,
  .read = devfs_read,
  .open = devfs_open,
  .close = devfs_close
};
