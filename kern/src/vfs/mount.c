#include "vfs/mount.h"
#include "vfs/devfs.h"
#include "vfs/fs.h"
#include "printk.h"
#include "errno.h"

struct riku_mountpoint mounts[MAX_MOUNTS];

uint32_t mount_internal(struct riku_devfs_node* device, struct riku_filesystem* fs)
{
  uint32_t index = 0;
  while(index < MAX_MOUNTS)
  {
      if(mounts[index].state == FREE)
      {
        /* Found free mountpoint */
        break;
      }
      index++;
  }

  /* Couldn't find an available mountpoint */
  if(index == MAX_MOUNTS)
  {
    return EMAXMP;
  }

  /* Okay, if we get here, we have a free mountpoint, so let's do some stuff. */
  mounts[index].device = device;
  mounts[index].fs = fs;

  /* Request FS initialization and checks. */
  if(mounts[index].fs->init(&mounts[index]))
  {
    /* Failure. Rollback. */
    mounts[index].device = 0x0;
    mounts[index].fs = 0x0;
    return EFSERR;
  } else {
    /* Filesystem managed to mount device, success */
    mounts[index].state = MOUNTED;
    printk("vfs: mounted device devfs:/%s into mountpoint %c:/ (index %d)\n", device->name, ('A' + (char)index), index);
    return 0;
  }
}
