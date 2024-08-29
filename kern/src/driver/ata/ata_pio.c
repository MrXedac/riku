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

#define ATA0_IO_BASE    0x1F0
#define ATA0_CTRL_BASE  0x3F6

#define ATA_IO_DATA             0x0
#define ATA_IO_ERROR_FEATURES   0x1
#define ATA_SECTOR_COUNT        0x2
#define ATA_SECTOR_NUMBER       0x3
#define ATA_CYL_LOW             0x4
#define ATA_CYL_HIGH            0x5
#define ATA_DRIVE_HEAD          0x6
#define ATA_STATUS_COMMAND      0x7

#define ATA_STATUS_ERR          1
#define ATA_STATUS_IDX          1 << 1
#define ATA_STATUS_CORR         1 << 2
#define ATA_STATUS_DRQ          1 << 3
#define ATA_STATUS_SRV          1 << 4
#define ATA_STATUS_DF           1 << 5
#define ATA_STATUS_RDY          1 << 6
#define ATA_STATUS_BSY          1 << 7

#define DRIVER_IO_PORT(self, x)   (self->resources[0].begin + x)
#define DRIVER_CTRL_PORT(self, x) (self->resources[1].begin + x)

void ata_wait_for_ready(struct riku_devfs_node* self)
{
    uint8_t result = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    while((result & ATA_STATUS_BSY) == ATA_STATUS_BSY) result = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    while((result & ATA_STATUS_DRQ) == 0x0 && (result & ATA_STATUS_ERR) == 0x0) result = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
}

void ata_delay(struct riku_devfs_node *self)
{
    inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
}

int ata_seek(struct riku_devfs_node* self, uint64_t position)
{
    outb(DRIVER_IO_PORT(self, ATA_DRIVE_HEAD), 0xE0 | ((position >> 24) & 0x0F));
    outb(DRIVER_IO_PORT(self, ATA_SECTOR_NUMBER), (unsigned char)position);
    outb(DRIVER_IO_PORT(self, ATA_CYL_LOW), (unsigned char)(position >> 8));    
    outb(DRIVER_IO_PORT(self, ATA_CYL_HIGH), (unsigned char)(position >> 16));
    
    return 0;
}

int ata_read(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    outb(DRIVER_IO_PORT(self, ATA_SECTOR_COUNT), count);
    outb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND), 0x20);
    uint8_t status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    while((status & ATA_STATUS_DRQ) == 0x0 || (status & ATA_STATUS_BSY) == ATA_STATUS_BSY) status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    
    uint16_t* buf = (uint16_t*)buffer;
    for(int i = 0; i < count; i++)
    {
        for(int j = 0; j < 256; j++)
            *(buf + (i * 256) + j) = inw(DRIVER_IO_PORT(self, ATA_IO_DATA));

        if(i < count - 1) { 
            // 400ns delay...
            ata_delay(self);

            // Poll until next set of data is available
            status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
            while((status & ATA_STATUS_DRQ) == 0x0 || (status & ATA_STATUS_BSY) == ATA_STATUS_BSY) status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
        }
    }

    return count;
}

int ata_write(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    outb(DRIVER_IO_PORT(self, ATA_SECTOR_COUNT), count);
    outb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND), 0x30);
    uint8_t status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    while((status & ATA_STATUS_DRQ) == 0x0 || (status & ATA_STATUS_BSY) == ATA_STATUS_BSY) status = inb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND));
    
    uint16_t* buf = (uint16_t*)buffer;
    for(int i = 0; i < count * 256; i++)
    {
        outw(DRIVER_IO_PORT(self, ATA_IO_DATA), *(buf + i));
    }

    outb(DRIVER_IO_PORT(self, ATA_STATUS_COMMAND), 0xE7);

    return count;
}

void ata_pio_init()
{
/* Allocate node */
  struct riku_devfs_node* node = hardware_create_node("ata0");
  if(node)
  {
      hardware_add_resource(node, PORTIO, ATA0_IO_BASE, ATA0_IO_BASE + 7);
      hardware_add_resource(node, PORTIO, ATA0_CTRL_BASE, ATA0_CTRL_BASE + 1);

      outb(DRIVER_IO_PORT(node, ATA_DRIVE_HEAD), 0xA0);
      outb(DRIVER_IO_PORT(node, ATA_SECTOR_COUNT), 0x0);
      outb(DRIVER_IO_PORT(node, ATA_SECTOR_NUMBER), 0x0);
      outb(DRIVER_IO_PORT(node, ATA_CYL_LOW), 0x0);
      outb(DRIVER_IO_PORT(node, ATA_CYL_HIGH), 0x0);
      outb(DRIVER_IO_PORT(node, ATA_STATUS_COMMAND), 0xEC);
      
      uint8_t result = inb(DRIVER_IO_PORT(node, ATA_STATUS_COMMAND));
      if(result == 0)
      {
        printk("ata0: drive not present\n");
      } else {
        printk("ata0: waiting for BSY to get reset\n");
        while((result & ATA_STATUS_BSY) == ATA_STATUS_BSY) result = inb(DRIVER_IO_PORT(node, ATA_STATUS_COMMAND));

        printk("ata0: polling until DRQ or ERR is set\n");
        while((result & ATA_STATUS_DRQ) == 0x0 && (result & ATA_STATUS_ERR) == 0x0) result = inb(DRIVER_IO_PORT(node, ATA_STATUS_COMMAND));

        printk("ata0: identify complete, reading\n");
        if((result & ATA_STATUS_ERR) == ATA_STATUS_ERR) {
            printk("ata0: identify completed with error status\n");
        } else {
            int index = 0;
            uint16_t data[256];
            for(index = 0; index < 256; index++)
            {
                data[index] = inw(DRIVER_IO_PORT(node, ATA_IO_DATA));
            }

            if((data[83] & (1 << 10)) == (1 << 10)) {
                printk("ata0: lba48 supported\n");
                uint64_t lba48_sectors = ((uint64_t)data[100] << 48) | ((uint64_t)data[101] << 32) | ((uint64_t)data[102] << 16) || (uint64_t)data[103];
                printk("ata0: %d lba48 sectors addressable\n");

                uint64_t* lbasectors = (uint64_t*)&data[100];
                printk("ata0: %d lba48 sectors addressable\n");
            }
        }
      }

      //serial->write = &serial_write;
      //serial->putch = &ser_putch;n
      node->seek = ata_seek;
      node->read = ata_read;
      node->write = ata_write;
      node->type = StorageDevice;

      /* ATA tests */
      /*char buffer[512];
      memset(buffer, 0, 512);

      ata_seek(node, 0);
      ata_read(node, buffer, 1);

      printk("%s", buffer);
      memset(buffer, 0, 512);

      strcpy(buffer, "RIKU TEST STRING");

        // Write 1 sector
      printk("sector 1\n");
      ata_seek(node, 1);
      ata_write(node, buffer, 1);
      printk("sector 0\n");
      ata_seek(node, 0);
      printk("write\n");
      ata_write(node, buffer, 1);*/

      devfs_add(node);
  }
  return;
}