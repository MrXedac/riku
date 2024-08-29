#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>
#include <string.h>
#include "elf64.h"
#include "heap.h"
#include "fs/ext2/ext2.h"

int ext2_read_block(struct riku_devfs_node* device, int block_index, char* buffer)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    
    int sector = info->sectors_per_block * block_index;
    char* block = (char*)kalloc(info->sectors_per_block * 512);
    device->seek(device, sector);
    device->read(device, block, info->sectors_per_block);
    memcpy(buffer, block, info->sectors_per_block * 512);

    kfree((void*)block);

    return 0;
}