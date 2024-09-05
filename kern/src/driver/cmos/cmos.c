#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"
#include "mbr.h"

void cmos_select_reg(struct riku_devfs_node* self, uint8_t reg)
{
    outb(self->resources[0].begin, reg); /* Disable NMI */
}

void cmos_write_reg(struct riku_devfs_node* self, uint8_t value)
{
    outb(self->resources[0].begin + 1, value);
}

uint8_t cmos_read_reg(struct riku_devfs_node* self)
{
    return inb(self->resources[0].begin + 1);
}

int cmos_seek(struct riku_devfs_node* self, uint64_t position)
{
    cmos_select_reg(self, (uint8_t)position);
    return 0;
}

int cmos_read(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    if(count > 1)
        printk("cmos: unsupported read of more than 1 byte\n");

    char* buf = (char*)buffer;

    *buf = cmos_read_reg(self);
    printk("cmos: read %x\n", *buf);
    return 1;
}

int cmos_write(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    if(count > 1)
        printk("cmos: unsupported read of more than 1 byte\n");

    cmos_write_reg(self, *buffer);
    return 1; 
}

/* Initializes CMOS */
void cmos_init()
{
  /* Allocate node */
  struct riku_devfs_node* cmos_node = hardware_create_node("cmos");
  if(cmos_node)
  {
      hardware_add_resource(cmos_node, PORTIO, 0x70, 0x70 + 1);
      cmos_node->read = cmos_read;
      cmos_node->write = cmos_write;
      cmos_node->seek = cmos_seek;
      cmos_node->type = SpecialDevice;
      devfs_add(cmos_node);

      printk("cmos: initialized CMOS driver\n");
  }
  return;
}