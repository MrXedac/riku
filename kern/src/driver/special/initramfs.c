#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"

int initramfs_write(struct riku_devfs_node* self, const char* buf, uint32_t count)
{
  return 0;
}

int initramfs_putch(struct riku_devfs_node* self, char c)
{
  return 0;
}

int initramfs_getch(struct riku_devfs_node* self, char* c)
{
  *c = 0x0;
  return 0;
}

int initramfs_read(struct riku_devfs_node* self, const char* buf, uint32_t count)
{
  for(uint32_t i = 0; i < count; i++)
    *(char*)(buf + i) = 0x0;

  return 0;
}

extern uintptr_t initramfs_begin;

/* Creates special initramfs device */
void initramfs_init()
{
  struct riku_devfs_node* initramfs = hardware_create_node("initramfs");
  if(initramfs)
  {
      initramfs->write = &initramfs_write;
      initramfs->putch = &initramfs_putch;
      initramfs->getch = &initramfs_getch;
      initramfs->read = &initramfs_read;
      initramfs->resources[0].begin = initramfs_begin; 
      devfs_add(initramfs);
  }
  return;
}
