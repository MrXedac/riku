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

#define RTC_IDX_SECONDS     0
#define RTC_IDX_MINUTES     1
#define RTC_IDX_HOURS       2
#define RTC_IDX_WEEKDAY     3
#define RTC_IDX_DAYOFMONTH  4
#define RTC_IDX_MONTH       5
#define RTC_IDX_YEAR        6
#define RTC_IDX_CENTURY     7

#define CMOS_ADDRESS        0x70
#define CMOS_DATA           0x71

void rtc_select_reg(struct riku_devfs_node* self, uint8_t reg)
{
    outb(self->resources[0].begin, (1 << 7) | reg); /* Disable NMI */
}

void rtc_write_reg(struct riku_devfs_node* self, uint8_t value)
{
    outb(self->resources[0].begin + 1, value);
}

uint8_t rtc_read_reg(struct riku_devfs_node* self)
{
    return inb(self->resources[0].begin + 1);
}

int rtc_seek(struct riku_devfs_node* self, uint64_t position)
{
    /* Seek in CMOS context */
    uint64_t realPosition = 0;
    switch(position)
    {
        case RTC_IDX_SECONDS:
            realPosition = 0x00;
            break;
         case RTC_IDX_MINUTES:
            realPosition = 0x02;
            break;
        case RTC_IDX_HOURS:
            realPosition = 0x04;
            break;
        case RTC_IDX_WEEKDAY:
            realPosition = 0x06;
            break;
        case RTC_IDX_DAYOFMONTH:
            realPosition = 0x07;
            break;
        case RTC_IDX_MONTH:
            realPosition = 0x08;
            break;
        case RTC_IDX_YEAR:
            realPosition = 0x09;
            break;
        case RTC_IDX_CENTURY:
            realPosition = 0x32;
            break;           
    }

    outb(CMOS_ADDRESS, realPosition);
    return 0;
}

uint8_t bcd_to_bin(uint8_t val)
{
    uint8_t ret= (val & 0xF) + ((val / 16) * 10);
    return ret;
}

int rtc_ready()
{
    outb(CMOS_ADDRESS, 10);
    return (inb(CMOS_DATA) & 0x80);
}

int rtc_read(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    while(rtc_ready());

    if(count != 8)
    {
        printk("rtc: unsupported read from rtc (%d bytes)\n", count);
        return 0;
    }
    

    char buf[8];
    char bufCache[8];

    char flags;
    outb(CMOS_ADDRESS, 0x00); buf[0] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x02); buf[1] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x04); buf[2] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x06); buf[3] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x07); buf[4] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x08); buf[5] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x09); buf[6] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x32); buf[7] = inb(CMOS_DATA);
    outb(CMOS_ADDRESS, 0x0B); flags = inb(CMOS_DATA);
    
    do {
        bufCache[0] = buf[0];
        bufCache[1] = buf[1];
        bufCache[2] = buf[2];
        bufCache[3] = buf[3];
        bufCache[4] = buf[4];
        bufCache[5] = buf[5];
        bufCache[6] = buf[6];
        bufCache[7] = buf[7];

        while(rtc_ready());
        outb(CMOS_ADDRESS, 0x00); buf[0] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x02); buf[1] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x04); buf[2] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x06); buf[3] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x07); buf[4] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x08); buf[5] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x09); buf[6] = inb(CMOS_DATA);
        outb(CMOS_ADDRESS, 0x32); buf[7] = inb(CMOS_DATA);
    } while(bufCache[0] != buf[0]
    || bufCache[1] != buf[1]
    || bufCache[2] != buf[2]
    || bufCache[3] != buf[3]
    || bufCache[4] != buf[4]
    || bufCache[5] != buf[5]
    || bufCache[6] != buf[6]
    || bufCache[7] != buf[7]
    );

    if(!(flags & 2) && (buf[2] & 0x80))
    {
        buf[2] = ((buf[2] & 0x80) + 12) % 24;
    }

    if(!(flags & 4))
    {
        for(int i = 0; i < 8; i++) buf[i] = bcd_to_bin(buf[i]);
    }

    buf[3] = (buf[3] + 5) % 7;

    memcpy((void*)buffer, buf, 8);
    
    return 8;
}

int rtc_write(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    /* if(count != 8)
    {
        printk("rtc: unsupported write to rtc (%d bytes)\n", count);
        return 0;
    }

    int curCount = 0;
    
    while(curCount < count)
    {
        rtc_seek(self, curCount);
        self->parent->write(self->parent, buffer+curCount, 1);
        curCount++;
    }

    return count;*/
    printk("rtc: unsupported write\n");
    return 0;
}

/* Initializes rtc */
void rtc_init()
{
  /* Allocate node */
  struct riku_devfs_node* rtc_node = hardware_create_node("rtc");
  if(rtc_node)
  {
      struct riku_devfs_node* cmos = devfs_find_node("cmos");
      if(!cmos)
      {
        printk("rtc: no cmos found\n");
        return;
      }

      hardware_add_resource(rtc_node, PORTIO, 0x00, 0x00 + 0x32);
      rtc_node->parent = cmos;
      rtc_node->read = rtc_read;
      rtc_node->write = rtc_write;
      rtc_node->seek = rtc_seek;
      rtc_node->type = SpecialDevice;

      devfs_add(rtc_node);

      printk("rtc: initialized rtc driver\n");
      char buf[8];
      memset(buf, 0, 8);
      rtc_node->read(rtc_node, buf, 8);

      printk("rtc: current datetime: %d/%d/%d%d %d:%d:%d\n", 
        buf[RTC_IDX_DAYOFMONTH],
        buf[RTC_IDX_MONTH],
        buf[RTC_IDX_CENTURY],
        buf[RTC_IDX_YEAR],
        buf[RTC_IDX_HOURS],
        buf[RTC_IDX_MINUTES],
        buf[RTC_IDX_SECONDS]
      );
  }
  return;
}