#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"

int ser_putch(struct riku_devfs_node* self, char c)
{
  uint32_t portBase = (uint32_t)(self->resources[0].begin);
  uint32_t ret;
  do {
    ret = inb(portBase + 5) & 0x20;
  } while(ret == 0);

  outb(portBase, c);
  return 0;
}


int serial_write(struct riku_devfs_node* self, const char* buf, uint32_t count)
{
  for(uint32_t i = 0; i < count; i++)
    ser_putch(self, *(buf + i));

  return 0;
}

/* Probes x86 serial driver, and adds corresponding devfs entry if required */
void x86serial_init()
{
  /* Allocate node */
  struct riku_devfs_node* serial = hardware_create_node("serial0");
  if(serial)
  {
      hardware_add_resource(serial, PORTIO, 0x3F8, 0x3F8 + 5);
      serial->write = &serial_write;
      serial->putch = &ser_putch;
      devfs_add(serial);

      if(!kconsole)
      {
        kconsole = serial;
        printk_enabled = 1;
        printk("Registered devfs:/serial0 as kernel console\n");
      }
  }
  return;
}
