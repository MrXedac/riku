#ifndef __EXT2__
#define __EXT2__

#include <stdint.h>
#include "vfs/devfs.h"

struct ext2_extended_superblock {
    uint32_t first_free_inode;                  /* First free inode in file system (if version lower that 1.0, fixed to 11 )*/
    uint16_t inode_struct_size;                 /* Size of inode structure, in bytes (version < 1.0, fixed to 128) */
    uint16_t superblock_block_group;            /* Block group this superblock is part of */
    uint32_t optional_features;                 /* Optional features not mandatory to r/w */
    uint32_t required_features;                 /* Features mandatory to r/w */
    uint32_t not_supported_features;            /* Not supported features which require the volume to be read only if it is not supported */
    uint32_t filesystem_id[4];                  /* Filesystem identifier */
    char volume_name[16];                       /* Volume name */
    char last_mount_path[64];                   /* Path the FS was last mounted to */
    uint32_t compression_used;                  /* Compression algorithm used */
    uint8_t prealloc_blocks_files;              /* Number of blocks to preallocate for files */
    uint8_t prealloc_blocks_directories;        /* Number of blocks to preallocate for directories */
    uint16_t unused;                            /* Unused. */
    uint32_t journal_id[4];                     /* Journal ID */
    uint32_t journal_inode;                     /* Journal inode number */
    uint32_t journal_device;                    /* Journal device */
    uint32_t orphan_inode_list;                 /* Head of orphan inode list */
}__attribute__((packed));

struct ext2_superblock {
    uint32_t inode_count;                       /* Inode count in filesystem */
    uint32_t block_count;                       /* Block count in filesystem */
    uint32_t reserved_blocks;                   /* Blocks reserved for superuser */
    uint32_t unallocated_blocks;                /* Unallocated blocks in filesystem */
    uint32_t unallocated_inodes;                /* Unallocated inodes in filesystem */
    uint32_t superblock_block;                  /* Block number of the block containing the superblock */
    uint32_t block_size;                        /* log2 block_size - 10 (shift 1024 to the left) */
    uint32_t fragment_size;                     /* log2 fragment_size - 10 (shift 1024 to the left) */
    uint32_t block_per_block_group;             /* Blocks per block group */
    uint32_t fragments_per_block_group;         /* Fragments per block group */
    uint32_t inodes_per_block_group;            /* Inodes per block group */
    uint32_t last_mount_time;                   /* Last mount time */
    uint32_t last_written_time;                 /* Last written time */
    uint16_t mounted_since_fsck;                /* Amount of times volume has been mounted since last fsck */
    uint16_t mounts_before_fsck;                /* Amount of times volume can be mounted before fsck is required */
    uint16_t signature;                         /* Ext2 signature. 0xEF53 */
    uint16_t fs_state;                          /* File system state (1: clean, 2: error) */
    uint16_t error_behaviour;                   /* What to do on error (1: ignore, 2: remount r/o, 3: panic) */
    uint16_t version_minor;                     /* Minor part of version */
    uint32_t fsck_time;                         /* POSIX time of last fsck */
    uint32_t fsck_interval;                     /* POSIX time interval between forced fsck */
    uint32_t creation_os;                       /* Creation operating system (0: linux, 1: hurd, 2: masix, 3: freebsd, 4: other) */
    uint32_t version_major;                     /* Major part of version */
    uint16_t root_uid;                          /* UID allowed to use reserved blocks */
    uint16_t root_gid;                          /* GID allowed to use reserved blocks */
    struct ext2_extended_superblock extended;   /* Extended superblock data (present if version_major > 1) */
}__attribute__((packed));

struct ext2_info {
    uint32_t block_size;
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t blocks_per_block_group;
    uint32_t inodes_per_block_group;
    uint32_t block_group_count;
    uint32_t sectors_per_block;
    uint32_t inode_size;
};

struct ext2_block_group_descriptor {
    uint32_t block_usage_bitmap_address;        /* Block address of block usage bitmap */
    uint32_t inode_usage_bitmap_address;        /* Block address of inode usage bitmap */
    uint32_t inode_table_address;               /* Starting block address of inode table */
    uint16_t unallocated_blocks;                /* Number of unallocated blocks */
    uint16_t unallocated_inodes;                /* Number of unallocated inodes */
    uint16_t directory_count;                   /* Number of directories */
    char unused[14];                            /* Padding */
}__attribute__((packed));

struct ext2_inode {
    uint16_t type_permissions;                  /* Type and permissions */
    uint16_t uid;                               /* User ID */
    uint32_t size_lower;                        /* Lower 32 bits of size */
    uint32_t last_access;                       /* Last access time */
    uint32_t creation_time;                     /* Creation time */
    uint32_t last_modification;                 /* Last modification time */
    uint32_t deletion_time;                     /* Deletion time */
    uint16_t gid;                               /* Group ID */
    uint16_t hard_link_count;                   /* Number of hard links (dir entries) to this inode */
    uint32_t sector_count;                      /* Number of hard disk sector count used by this inode, without inode structure or dir entries */
    uint32_t flags;                             /* Flags */
    uint32_t os_specific;                       /* OS Specific values */
    uint32_t direct_block_pointers[12];         /* Direct block pointers */
    uint32_t single_indirect_block_pointer;     /* Singly indirect block pointer */
    uint32_t doubly_indirect_block_pointer;     /* Doubly indirect block pointer */
    uint32_t triply_indirect_block_pointer;     /* Triply indirect block pointer */
    uint32_t generation_number;                 /* Used for NFS */
    uint32_t extended_attribute_block;          /* Extended attribute block if version major >= 1 */
    uint32_t size_upper;                        /* Upper 32 bits of size (if version major >= 1 and feature enabled), or directory acl */
    uint32_t fragment_block_address;            /* Fragment block address */ 
    uint32_t os_specific_extended[3];           /* Other OS-specific values */   
}__attribute__((packed));

struct ext2_direntry {
    uint32_t inode;                             /* Inode */
    uint16_t size;                              /* Entry size */
    uint8_t name_length_lower;                  /* Name length 8 lower bytes */
    uint8_t type;                               /* Type indicator */
}__attribute__((packed));

#define EXT2_INODE_TYPE_FIFO    0x1000
#define EXT2_INODE_TYPE_CHAR    0x2000
#define EXT2_INODE_TYPE_DIR     0x4000
#define EXT2_INODE_TYPE_BLOCK   0x6000
#define EXT2_INODE_TYPE_FILE    0x8000
#define EXT2_INODE_TYPE_SYMLINK 0xA000
#define EXT2_INODE_TYPE_SOCKET  0xC000

#define EXT2_INODE_PERM_OX      0x001
#define EXT2_INODE_PERM_OW      0x002
#define EXT2_INODE_PERM_OR      0x004
#define EXT2_INODE_PERM_GX      0x008
#define EXT2_INODE_PERM_GW      0x010
#define EXT2_INODE_PERM_GR      0x020
#define EXT2_INODE_PERM_UX      0x040
#define EXT2_INODE_PERM_UW      0x080
#define EXT2_INODE_PERM_UR      0x100
#define EXT2_INODE_PERM_STICKY  0x200
#define EXT2_INODE_PERM_SETGID  0x400
#define EXT2_INODE_PERM_SETUID  0x800

#define EXT2_DIRENTRY_TYPE_UNK      0
#define EXT2_DIRENTRY_TYPE_FILE     1
#define EXT2_DIRENTRY_TYPE_DIR      2
#define EXT2_DIRENTRY_TYPE_CHAR     3
#define EXT2_DIRENTRY_TYPE_BLOCK    4
#define EXT2_DIRENTRY_TYPE_FIFO     5
#define EXT2_DIRENTRY_TYPE_SOCKET   6
#define EXT2_DIRENTRY_TYPE_SYMLINK  7

/* Methods and utility functions */
int ext2_check_superblock(struct riku_devfs_node* device);
int ext2_lba_block_address(struct riku_devfs_node* device, int block_address);
int ext2_read_block_group_descriptor(struct riku_devfs_node* device, int block_group_index, struct ext2_block_group_descriptor* buffer);
int ext2_inode_block_group(struct riku_devfs_node* device, int inode);
int ext2_inode_block_group_inode_table_index(struct riku_devfs_node* device, int inode);
int ext2_inode_block(struct riku_devfs_node* device, int index);
int ext2_read_inode(struct riku_devfs_node* device, int inode, struct ext2_inode* buffer);
int ext2_read_block(struct riku_devfs_node* device, int block, char* buffer);
int ext2_find_inode(struct riku_devfs_node* device, int rootInode, const char* dirname, uint8_t dirFlag);
int ext2_get_direntry(struct riku_devfs_node* device, int dirInode, int offset, struct riku_fileinfo* result);
#endif