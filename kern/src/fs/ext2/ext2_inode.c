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

int ext2_inode_block_group(struct riku_devfs_node* device, int inode)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    return (inode - 1) / info->inodes_per_block_group;
}

int ext2_inode_block_group_inode_table_index(struct riku_devfs_node* device, int inode)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    return (inode - 1) % info->inodes_per_block_group;
}

int ext2_inode_block(struct riku_devfs_node* device, int index)
{
    struct ext2_info* info = (struct ext2_info*)device->extended;
    return (index * info->inode_size) / info->block_size;
}

int ext2_read_inode(struct riku_devfs_node* device, int inode, struct ext2_inode* buffer)
{
    /* Get block group */
    struct ext2_info* info = (struct ext2_info*)device->extended;
    int block_group = ext2_inode_block_group(device, inode);

    /* Get block group descriptor */
    char* buf = (char*)kalloc(sizeof(struct ext2_block_group_descriptor));
    struct ext2_block_group_descriptor* desc = (struct ext2_block_group_descriptor*)buf;

    ext2_read_block_group_descriptor(device, block_group, desc);

    /* Read inode table for this group */
    int inode_table = desc->inode_table_address;
    int sector = info->sectors_per_block * inode_table;
    char* block = (char*)kalloc(info->sectors_per_block * 512);

    ext2_read_block(device, inode_table, block);

    int inode_size = info->inode_size;    
    int index = ext2_inode_block_group_inode_table_index(device, inode);
    int offset = inode_size * index;

    struct ext2_inode* inode_data = (struct ext2_inode*)(block + offset);
    memcpy(buffer, inode_data, inode_size);

    kfree((void*)buf);
    kfree((void*)block);

    return 0;
}

int ext2_find_inode(struct riku_devfs_node* device, int rootInode, const char* dirname, uint8_t dirFlag)
{
    struct ext2_info* fsinfo = (struct ext2_info*)device->extended;

    char nameBuffer[256];
    char* folder = nameBuffer, *remaining = "\0";
    strcpy(nameBuffer, dirname);

    if(nameBuffer[0] == '\0')
    {
        // No dirname, returning root inode 2
        return 2;
    }

    int inode = -1;

    for(int i = 0; i < 256; i++)
    {
        if(nameBuffer[i] == '/')
        {
            nameBuffer[i] = '\0';
            folder = nameBuffer;
            remaining = &nameBuffer[i+1];
            //printk("ext2_find_inode: folder %s, remaining %s\n", folder, remaining);
            break;
        }
    }

     // Read root inode
     char* rootBuffer = (char*)kalloc(fsinfo->inode_size);
    struct ext2_inode *root = (struct ext2_inode*)rootBuffer;
    ext2_read_inode(device, rootInode, root);
    char* datablock = (char*)kalloc(fsinfo->sectors_per_block * 512);
    memset(datablock, 0, 512 * fsinfo->sectors_per_block);
    ext2_read_block(device, root->direct_block_pointers[0], datablock);

    struct ext2_direntry* root_dir = (struct ext2_direntry*)datablock;
    while(root_dir->size != 0)
    {
        char* inode_name = (char*)root_dir + sizeof(struct ext2_direntry);
        *(inode_name + root_dir->name_length_lower) = '\0';
        //printk("inode name: %s\n", inode_name);
        if(strcmp(inode_name, folder) == 0 && (dirFlag == 0 || (root_dir->type & EXT2_DIRENTRY_TYPE_DIR) == EXT2_DIRENTRY_TYPE_DIR))
        {
            if(*remaining == '\0') // End of traversal
            {
                //printk("found: %d\n", root_dir->inode);
                inode = root_dir->inode;
            } else {
                /* Found it, recursive traversal */
                //printk("found folder: %d\n", root_dir->inode);
                inode = ext2_find_inode(device, root_dir->inode, remaining, dirFlag);
            }
            break;
        }
        root_dir = (struct ext2_direntry*)((uintptr_t)root_dir + root_dir->size);
    }

    kfree((void*)rootBuffer);
    kfree((void*)datablock);

    return inode;
}

int ext2_direntry_flags_to_riku_flags(uint8_t flags)
{
    if(flags & EXT2_DIRENTRY_TYPE_FILE) return 0x1;
    if(flags & EXT2_DIRENTRY_TYPE_DIR) return 0x2;

    return 0x0;
}

int ext2_get_direntry(struct riku_devfs_node* device, int dirInode, int offset, struct riku_fileinfo* result)
{
    struct ext2_info* fsinfo = (struct ext2_info*)device->extended;

    // Read root inode
    char* rootBuffer = (char*)kalloc(fsinfo->inode_size);
    struct ext2_inode *root = (struct ext2_inode*)rootBuffer;
    ext2_read_inode(device, dirInode, root);
    char* datablock = (char*)kalloc(fsinfo->sectors_per_block * 512);
    memset(datablock, 0, 512 * fsinfo->sectors_per_block);
    ext2_read_block(device, root->direct_block_pointers[0], datablock);

    struct ext2_direntry* root_dir = (struct ext2_direntry*)datablock;
    int index = 0;
    while(root_dir->size != 0)
    {
        char* inode_name = (char*)root_dir + sizeof(struct ext2_direntry);

        if(index == offset)
        {
            strcpy(result->name, inode_name);

            // Get size
            ext2_read_inode(device, root_dir->inode, root);

            result->size = root->size_lower; 
            result->type = ext2_direntry_flags_to_riku_flags(root_dir->type);

            break;
        }
        root_dir = (struct ext2_direntry*)((uintptr_t)root_dir + root_dir->size);
        index++;
    }

    kfree((void*)rootBuffer);
    kfree((void*)datablock);

    if(root_dir->size == 0) return -1;
    return 0;
}