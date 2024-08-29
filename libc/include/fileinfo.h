#ifndef __FILEINFO__
#define __FILEINFO__

#include <stdint.h>

struct riku_fileinfo {
  uint32_t flags;   /* Sum flags */
  uint64_t handle;  /* File handle for kernel filesystem management internals */
  uint32_t state;   /* File state : 0 if nothing to say, 1 if flushing required */
  uint32_t cache;   /* If filesystem implements a cache, put this to 1 to explicitly require the FS driver to cache the corresponding inode */
  uint64_t diroff;  /* If this represents a directory, offset for readdir(). Irrelevant if not. */
  char name[256];   /* File name */
  int type;         /* File type */
  int size;         /* File size */
  void * extended;  /* Filesystem-dependant extended information - put whatever you need in your FS driver implementation */
};

#endif