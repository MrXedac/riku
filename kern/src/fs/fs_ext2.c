#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>
#include <string.h>
#include "elf64.h"
#include "heap.h"
#include "idt64.h"
#include "fs/ext2/ext2.h"

int ext2fs_init(struct riku_mountpoint* self)
{
    if(ext2_check_superblock(self->device) == 0)
    {
        printk("ext2: found ext2 filesystem on %s\n", self->device->name);
        return 0;
    } else return EFSERR;
}

int ext2fs_opendir(struct riku_mountpoint* self, const char* directory, struct riku_fileinfo* desc)
{
    int directoryInodeNumber = ext2_find_inode(self->device, 2, directory, 1);

    if(directoryInodeNumber == -1) return ENOENT;

    desc->handle = directoryInodeNumber;
    desc->flags = FLAGS_DIRECTORY; // Directory
    desc->diroff = 0x0;
    desc->state = 0x0;
    desc->cache = 0x0;
    desc->extended = (void*)self;
    strcpy(desc->name, directory);
    desc->type = 0x0;

    return 0;
}

int ext2fs_readdir(struct riku_mountpoint* self, struct riku_fileinfo* desc, uint32_t offset, struct riku_fileinfo* result)
{
    struct ext2_info* info = (struct ext2_info*)self->device->extended;

    int inode_number = desc->handle;
    if(ext2_get_direntry(self->device, inode_number, desc->diroff, result) == -1) return -ENMFIL;
    
    desc->diroff++;
    return 0;
}

int ext2fs_write(struct riku_mountpoint* self, struct riku_fileinfo* file, const char*  buffer, uint64_t length, uint64_t offset)
{
    printk("ext2fs_write\n");
    return 0;
}

/* TODO : add offset support */
int ext2fs_read(struct riku_mountpoint* self, struct riku_fileinfo* file, char*  buffer, uint64_t length, uint64_t offset)
{
    struct ext2_info* info = (struct ext2_info*)self->device->extended;
    char* inodeData = (char*)kalloc(info->inode_size);
    struct ext2_inode* inode = (struct ext2_inode*)inodeData;

    if(offset + length > 12 * info->block_size) {
        panic("ext2: attempting to read too much data", (void*)0);
    }

    ext2_read_inode(self->device, file->handle, inode);

    int size = inode->size_lower;
    int lengthToRead = length;
    if(offset + length > size)
        lengthToRead = size - offset;
    int blocksToRead = offset + lengthToRead;

    int curBlockIndex = 0;   
    int curOffset = offset;
    int curLength = length;
    int curLengthToRead = lengthToRead;

    if(lengthToRead > info->block_size) curLengthToRead = info->block_size;

    while(blocksToRead > 0)
    {
        int dataPage = inode->direct_block_pointers[curBlockIndex];

        printk("size %d, offset %d, length %d, lengthToRead %d\n", size, offset, length, curLengthToRead);
        
        char* block = (char*)kalloc(info->sectors_per_block * 512);
        ext2_read_block(self->device, dataPage, block);

        memcpy(buffer + (curBlockIndex * info->block_size), block, curLengthToRead);
        kfree((void*)block);

        /* Decrease block size from length */
        blocksToRead -= info->block_size;
        
        /* If less than a block to read, read only remaining data */
        if(blocksToRead <= info->block_size) curLengthToRead = blocksToRead;

        /* Next index */
        curBlockIndex++;
    }
    kfree((void*)inodeData);
    
    return lengthToRead;
}

int ext2fs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{ 
    struct ext2_info* info = (struct ext2_info*)self->device->extended;
    int inodeNumber = ext2_find_inode(self->device, 2, file, 0);
    if(inodeNumber == -1) {
        printk("couldnt find file at %s\n", file);
        return -ENOENT;
    } 

    result->handle = inodeNumber;
    result->flags = 0x0; // Directory
    result->diroff = 0x0;
    result->state = 0x0;
    result->cache = 0x0;
    result->extended = (void*)0;
    strcpy(result->name, file);
    result->type = 0x0;
    
    char* buf = (char*)kalloc(info->inode_size);
    struct ext2_inode* inode = (struct ext2_inode*)buf;

    ext2_read_inode(self->device, inodeNumber, inode);
    printk("ext2: inode %d size %d\n", inodeNumber, inode->size_lower);
    result->size = inode->size_lower;
    
    return 0;
}

int ext2fs_close(struct riku_mountpoint* self, struct riku_fileinfo* file)
{
    printk("ext2fs_close\n");
    return 0;
}

struct riku_filesystem fs_ext2fs =
{
    .name = "ext2fs",
    .fullname = "ext2 filesystem driver for Riku kernel",
    .init = ext2fs_init,
    .opendir = ext2fs_opendir,
    .readdir = ext2fs_readdir,
    .write = ext2fs_write,
    .read = ext2fs_read,
    .open = ext2fs_open,
    .close = ext2fs_close
};
