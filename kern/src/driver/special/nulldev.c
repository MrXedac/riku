#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"

int null_write(struct riku_devfs_node* self, const char* buf, uint32_t count)
{
  return 0;
}

int null_putch(struct riku_devfs_node* self, char c)
{
  return 0;
}

int null_getch(struct riku_devfs_node* self, char* c)
{
  *c = 0x0;
  return 0;
}

int null_read(struct riku_devfs_node* self, const char* buf, uint32_t count)
{
  for(uint32_t i = 0; i < count; i++)
    *(char*)(buf + i) = 0x0;

  return 0;
}

/* Creates special NULL device */
void nulldev_init()
{
  /* Allocate node */
  struct riku_devfs_node* devfs_node = hardware_create_node("devices");
  if(devfs_node)
    devfsVirtPtr = devfs_node;
    
  struct riku_devfs_node* null = hardware_create_node("null");
  if(null)
  {
      null->write = &null_write;
      null->putch = &null_putch;
      null->getch = &null_getch;
      null->read = &null_read;

      nullDev = null;
  }
  return;
}
