#include "vfs/mount.h"
#include "vfs/fs.h"
#include "vfs/devfs.h"
#include "errno.h"
#include "printk.h"
#include <stdint.h>
#include <string.h>
#include "elf64.h"
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

int ext2fs_readdir(struct riku_mountpoint* self, struct riku_fileinfo* desc, uint32_t offset, struct riku_fileinfo* result)
{
    return 0;
}

int ext2fs_write(struct riku_mountpoint* self, struct riku_fileinfo* file, const char*  buffer, uint64_t length, uint64_t offset)
{
    printk("ext2fs_write\n");
    return 0;
}

int ext2fs_read(struct riku_mountpoint* self, struct riku_fileinfo* file, char*  buffer, uint64_t length, uint64_t offset)
{
    return 0;
}

int ext2fs_open(struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result)
{
    return -ENOENT; /* Nothing found ? Return failure */
}

int ext2fs_close(struct riku_mountpoint* self, struct riku_fileinfo* file)
{
    printk("ext2fs_close\n");
    return 0;
}

struct riku_filesystem fs_ext2fs =
{
    .init = ext2fs_init,
    .opendir = ext2fs_opendir,
    .readdir = ext2fs_readdir,
    .write = ext2fs_write,
    .read = ext2fs_read,
    .open = ext2fs_open,
    .close = ext2fs_close
};
