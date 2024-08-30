#include "vfs/descriptor.h"
#include "vfs/devfs.h"
#include "vfs/fs.h"
#include "vfs/mount.h"
#include "vfs/dir.h"
#include "vfs/dirent.h"
#include "heap.h"
#include "mem.h"
#include "errno.h"
#include "printk.h"
#include "task.h"
#include <stdint.h>

int closedir(struct riku_fileinfo* dir)
{
    return 0;
}

int opendir(const char* path, struct riku_fileinfo* buffer)
{
  /* FIXME : this is dirty as hell */
  int offset = 0;

  /* Find drive letter and get mountpoint */
  char drvLetter = path[0];
  printk("Attempting to open %c\n", drvLetter);
  if(drvLetter < 'A' || drvLetter > 'Z') return -ENOFILE;

  /* Mountpoints definition */
  extern struct riku_mountpoint mounts[MAX_MOUNTS];

  /* Find expected offset */
  int drvOffset = drvLetter - 'A';

  printk("Checking for device at offset %d\n", drvOffset);
  /* Only continue if we have a valid device descriptor and filesystem */
  if((!mounts[drvOffset].device) || (!mounts[drvOffset].fs)) return -ENOFILE;

  /* Allocate result structure - path+3 eliminates 'C:/' */
  if(strlen(path) >= 3) path+=3; 
  int ret = mounts[drvOffset].fs->opendir(&mounts[drvOffset], path, buffer);
  
  printk("return %d\n", ret);
  /* ret = 0 : open success, else open failed */
  if(ret) return -ENOENT;

  return 0;
}

int readdir(struct riku_fileinfo* dir, uint32_t offset, struct riku_fileinfo* buffer)
{
    if(dir->flags != FLAGS_DIRECTORY) return -1;

    struct riku_mountpoint* mount = (struct riku_mountpoint*)dir->extended;
    
    /* Ignore offset, iterate */
    return mount->fs->readdir(mount, dir, offset, buffer);
}