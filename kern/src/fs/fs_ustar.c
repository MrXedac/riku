#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>
#include <string.h>
#include "elf64.h"

int oct2bin(unsigned char *str, int size) {
    int n = 0;
    unsigned char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

int ustarfs_init(struct riku_mountpoint* self)
{
    printk("ustarfs_init: checking sanity of ustarfs.\n");
    unsigned char* ptr = (unsigned char*)(self->device->resources[0].begin);
    if(!memcmp((const char*)ptr+257, "ustar", 5))
        return 0;
    else
        return EFSERR;
}

int ustarfs_opendir(struct riku_mountpoint* self, const char* directory, struct riku_fileinfo* desc)
{
    /* TODO : add support for subdirectories */
    desc->handle = 0x0;
    desc->flags = FLAGS_DIRECTORY; // Directory
    desc->diroff = 0x0;
    desc->state = 0x0;
    desc->cache = 0x0;
    desc->extended = (void*)self;
    strcpy(desc->name, directory);
    desc->type = 0x0;

    return 0;
}

int ustarfs_identify(unsigned char* ptr)
{
    if(ptr[0] == '\x7f' && 
		ptr[1] == 'E' && 
		ptr[2] == 'L' && 
		ptr[3] == 'F') return 0x2;
    
    else return 0x1;
}

int ustarfs_readdir(struct riku_mountpoint* self, struct riku_fileinfo* desc, uint32_t offset, struct riku_fileinfo* result)
{
    uint64_t trueOffset;
    if(desc)
        trueOffset = desc->diroff;
    else
        trueOffset = offset;

    uint32_t i = 0;
    unsigned char *ptr = (unsigned char*)(self->device->resources[0].begin); /* DevFS node resource 0 is base address of ustar archive in memory */
    char name[32];
    while(i < trueOffset)
    {
        if (!memcmp((const char*)ptr + 257, "ustar", 5)) {
            int filesize = oct2bin(ptr + 0x7c, 11);
            ptr += (((filesize + 511) / 512) + 1) * 512;
        } else return ENMFIL;

        i++;
    }
    if(!memcmp((const char*)ptr + 257, "ustar", 5)) {
        memset(name, 0, sizeof(name));
        strcpy(name, (const char*)ptr);
        result->extended = (void*)ptr;
        strcpy(result->name, name);
        result->size = oct2bin(ptr + 0x7c, 11);
        result->type = ustarfs_identify(ptr+512); // File
    } else return ENMFIL;

    /* If we're readdir'ing through a descriptor, increment directory offset */
    if(desc)
        desc->diroff++;

    return 0;
}

int ustarfs_write(struct riku_mountpoint* self, struct riku_fileinfo* file, const char*  buffer, uint64_t length, uint64_t offset)
{
    printk("ustarfs_write\n");
    return 0;
}

int ustarfs_read(struct riku_mountpoint* self, struct riku_fileinfo* file, char*  buffer, uint64_t length, uint64_t offset)
{
    unsigned char* ptr = (unsigned char*)file->extended;
    int filesize = oct2bin(ptr + 0x7c, 11);
    int trueLength;
    
    /* Compute maximum size of data */
    if(offset > filesize) return 0;
    
    if(offset + length > filesize)
        trueLength = filesize - offset;
    else
        trueLength = length;

    unsigned char* data = ptr + 512;
    memcpy(buffer, data, trueLength);
    
    return trueLength;
}

int ustarfs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{
    struct riku_fileinfo ramfs_dir, ramfs_node;
    memset(&ramfs_dir, 0, sizeof(ramfs_dir));
    while(!ustarfs_readdir(self, &ramfs_dir, 0, &ramfs_node))
    {
        /* Extended -> ptr in ustar */
        /* While we don't have a fs_stat yet to find name, size etc, do it manually : ptr refers to name, ptr + 512 = pointer to data */
        char* name = (char*)ramfs_node.extended;
        uintptr_t data = (uintptr_t)ramfs_node.extended + 512;
        if(!strcmp(name, (char*)file))
        {
            /* File found - no additional operation required. Set address of data in extended ptr, and return */
            result->extended = (void*)name; /* Base address of file in archive */
            return 0;
        }
    }
    return -ENOENT; /* Nothing found ? Return failure */
}

int ustarfs_close(struct riku_mountpoint* self, struct riku_fileinfo* file)
{
    printk("ustarfs_close\n");
    return 0;
}

struct riku_filesystem fs_ustarfs =
{
    .init = ustarfs_init,
    .opendir = ustarfs_opendir,
    .readdir = ustarfs_readdir,
    .write = ustarfs_write,
    .read = ustarfs_read,
    .open = ustarfs_open,
    .close = ustarfs_close
};
