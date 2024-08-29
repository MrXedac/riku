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

int ext2_check_superblock(struct riku_devfs_node* device)
{
    char buffer[1024];

    /* Read 2 sectors from sector 2 */
    device->seek(device, 2);
    device->read(device, buffer, 2);

    struct ext2_superblock* superblock = (struct ext2_superblock*)buffer;
    printk("ext2: signature is %x, expect 0xEF53\n", superblock->signature);

    if(superblock->signature == 0xEF53) {
        struct ext2_info* fsinfo = (struct ext2_info*)kalloc(sizeof(struct ext2_info));
        fsinfo->block_count = superblock->block_count;
        fsinfo->block_size = superblock->block_size << 11;
        fsinfo->blocks_per_block_group = superblock->block_per_block_group;
        fsinfo->inode_count = superblock->inode_count;
        fsinfo->inodes_per_block_group = superblock->inodes_per_block_group;

        uint32_t block_group_count = fsinfo->block_count / fsinfo->blocks_per_block_group;
        fsinfo->block_group_count = block_group_count;
        fsinfo->sectors_per_block = fsinfo->block_size / 512; /* A sector on drive is 512 bytes right now */

        if(superblock->version_major >= 1) fsinfo->inode_size = superblock->extended.inode_struct_size;
        else fsinfo->inode_size = 128;

        device->extended = (void*)fsinfo;

        printk("ext2: block size %d, inode count %d, block count %d, blocks per group %d, inodes per block group %d, block group count %d, inode size %d, %d sectors per block\n",
            superblock->block_size << 11,
            superblock->inode_count,
            superblock->block_count,
            superblock->block_per_block_group,
            superblock->inodes_per_block_group,
            fsinfo->block_group_count,
            fsinfo->inode_size,
            fsinfo->sectors_per_block
        );

        /*int dirInode = ext2_find_inode(device, 2, "some_dir/some_deeper_dir", 1);
        printk("some_deeper_dir inode %d\n", dirInode);

        dirInode = ext2_find_inode(device, 2, "", 1);
        printk("root inode %d\n", dirInode);

        dirInode = ext2_find_inode(device, 2, "some_dir/some_deeper_dir/other.txt", 0);
        printk("file inode %d\n", dirInode);*/
        /*struct ext2_block_group_descriptor* desc = (struct ext2_block_group_descriptor*)kalloc(sizeof(struct ext2_block_group_descriptor));
        ext2_read_block_group_descriptor(device, 30, desc);
        printk("block group 30 descriptor: usage bitmap %d, inode usage bitmap %d, inode table address %d, %d unallocated blocks, %d unallocated inodes, %d directories\n",
            desc->block_usage_bitmap_address,
            desc->inode_usage_bitmap_address,
            desc->inode_table_address,
            desc->unallocated_blocks,
            desc->unallocated_inodes,
            desc->directory_count);*/

        return 0;
    }
    else return -1;
}

int ext2_read_block_group_descriptor(struct riku_devfs_node* device, int block_group_index, struct ext2_block_group_descriptor* buffer)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    int sector = ext2_lba_block_address(device, 1);
    char* device_buffer = (char*)kalloc(info->sectors_per_block * 512); // We read 1 block !
    device->seek(device, sector);
    device->read(device, device_buffer, info->sectors_per_block);

    struct ext2_block_group_descriptor* array = (struct ext2_block_group_descriptor*)device_buffer;
    memcpy(buffer, (array + block_group_index), sizeof(struct ext2_block_group_descriptor));

    kfree((void*)device_buffer);
    return 0;
}

int ext2_lba_block_address(struct riku_devfs_node* device, int block_address)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    return block_address * info->sectors_per_block;
}