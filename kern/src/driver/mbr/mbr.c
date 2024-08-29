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

// Maximum 2 drives with 4 partitions, max. 8... should be "safe" for now
char curLetter = 'a';

int mbr_seek(struct riku_devfs_node* self, uint64_t position)
{
    return 0;
}

int mbr_read(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    return 0;
}

int mbr_write(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
    return 0;
}

int partition_size_mb(struct mbr_partition_entry* entry)
{
    return (entry->sector_count * 512) / 1024 / 1024; // megabytes
}

void mbr_init_entry(struct mbr_partition_entry* entry)
{
    if(entry->lba_start > 0)
    {
        printk("ata0.mbr: partition attribs %x lba_start %x size %dMb\n", entry->attributes, entry->lba_start, partition_size_mb(entry));
        char devName[6];
        strcpy(devName, "ata0");
        devName[4] = curLetter;
        devName[5] = '\0';

        struct riku_devfs_node* node = hardware_create_node(devName);
        hardware_add_resource(node, UNKNOWN, entry->lba_start, entry->lba_start + entry->sector_count);
        node->type = StorageDevice;
        devfs_add(node);
        curLetter++;
    }
}

void mbr_init()
{
    /* Allocate node */
    struct riku_devfs_node* drive = devfs_find_node("ata0");
    if(!drive) 
    {
        printk("mbr: no drive attached\n");
        return;
    }

    char buffer[512];
    memset(buffer, 0, 512);
    drive->seek(drive, 0);
    drive->read(drive, buffer, 1);

    struct mbr_header* mbr = (struct mbr_header*)buffer;

    mbr_init_entry(&mbr->first_entry);
    mbr_init_entry(&mbr->second_entry);
    mbr_init_entry(&mbr->third_entry);
    mbr_init_entry(&mbr->fourth_entry);

/*
    node->seek = ata_seek;
    node->read = ata_read;
    node->write = ata_write;

    devfs_add(node);*/
  return;
}