#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>
#include <string.h>

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
    return 0;
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
    printk("ustarfs_read\n");
    return 0;
}

int ustarfs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{
    printk("ustarfs_open\n");
    return 0;
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
