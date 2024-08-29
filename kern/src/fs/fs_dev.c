#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include "string.h"
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
  desc->extended = (void*)self;
  /* Right now, we only have files in devfs - simplify this and don't use opendir, only use readdir */
  return 0;
}

int devfs_readdir_type(struct riku_devfs_node* node)
{
  switch(node->type)
  {
    case HIDDevice:
      return 0x1;
      break;
    case StorageDevice:
      return 0x2;
      break;
    case NetworkDevice:
      return 0x3;
      break;
    case SpecialDevice:
      return 0x4;
      break;  
    case UnknownDevice:
    default:
      return 0x5;
      break;
  }
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
  strcpy(result->name, tmp->name);
  result->type = devfs_readdir_type(tmp);
  result->size = 0x0;
  return 0;
}

int devfs_write(struct riku_mountpoint* self, struct riku_fileinfo* file, const char*  buffer, uint64_t length, uint64_t offset)
{
  struct riku_devfs_node* node = (struct riku_devfs_node*)file->extended;
  node->write(node, buffer, length);
    return 0;
}

int devfs_read(struct riku_mountpoint* self, struct riku_fileinfo* file, char*  buffer, uint64_t length, uint64_t offset)
{
  struct riku_devfs_node* node = (struct riku_devfs_node*)file->extended;
  return node->read(node, buffer, length);
}

int devfs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{
  /* Special case with devfs and ustarfs : return devnode as extended */
  struct riku_devfs_node *tmp = devfs_find_node((char*)file);
  if(tmp) {
      result->extended = (void*)tmp;
      return 0;
  } else return -ENOENT;
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
