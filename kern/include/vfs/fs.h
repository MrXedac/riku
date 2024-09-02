#ifndef __FS__
#define __FS__

#include "vfs/mount.h"
#include "vfs/devfs.h"
#include <stdint.h>

#define FLAGS_DIRECTORY 0x2

struct riku_fileinfo {
  uint32_t flags;   /* Sum flags */
  uint64_t handle;  /* File handle for kernel filesystem management internals */
  uint32_t state;   /* File state : 0 if nothing to say, 1 if flushing required */
  uint32_t cache;   /* If filesystem implements a cache, put this to 1 to explicitly require the FS driver to cache the corresponding inode */
  uint64_t diroff;  /* If this represents a directory, offset for readdir(). Irrelevant if not. */
  char name[256];   /* File name */
  int type;         /* File type */
  int size;         /* File size, if applicable */
  void * extended;  /* Filesystem-dependant extended information - put whatever you need in your FS driver implementation */
};

struct riku_stat {
  char name[256];
  uint32_t size;
};

/* File system interface
  - self : pointer to riku_mountpoint, which is a structure representing a mountpoint on the system (containing pointers to the device and FS driver)
*/
/* FS initialization */
typedef int (*fs_init_t)     (struct riku_mountpoint* self); /* Initialize filesystem */

/* Directory functions */
typedef int (*fs_opendir_t)  (struct riku_mountpoint* self, const char* directory, struct riku_fileinfo* descriptor); /* Open directory for reading */
typedef int (*fs_readdir_t)  (struct riku_mountpoint* self, struct riku_fileinfo* descriptor, uint32_t offset, struct riku_fileinfo* result); /* Read directory entry */

/* mkdir, mknod... */

/* File functions */
typedef int (*fs_write_t)    (struct riku_mountpoint* self, struct riku_fileinfo* file, const char* buffer, uint64_t length, uint64_t offset);
typedef int (*fs_read_t)     (struct riku_mountpoint* self, struct riku_fileinfo* file, char* buffer, uint64_t length, uint64_t offset);
typedef int (*fs_open_t)     (struct riku_mountpoint* self, const char* file, struct riku_fileinfo* result);
typedef int (*fs_close_t)    (struct riku_mountpoint* self, struct riku_fileinfo* file);
typedef int (*fs_stat_t)     (struct riku_mountpoint* self, struct riku_fileinfo* file);

/* rename, symlink, unlink, flush... */

/* Filesystem driver interface */
struct riku_filesystem {
  char          name[16];
  char          fullname[256];
  fs_init_t     init;
  fs_opendir_t  opendir;
  fs_readdir_t  readdir;
  fs_write_t    write;
  fs_read_t     read;
  fs_open_t     open;
  fs_close_t    close;
  fs_stat_t     stat;
};

#endif
