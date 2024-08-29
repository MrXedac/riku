#ifndef __ERRNO__
#define __ERRNO__

#define ENOSYSC 1 /* Undefined system call */
#define ENOFILE 2 /* No such file descriptor */
#define ECLOSED 3 /* File descriptor is closed */
#define ENOENT  4 /* No such file or directory */
#define EMAXFD  5 /* No more file descriptors available */
#define EMAXMP  6 /* No more mountpoints available */
#define EFSERR  7 /* Filesystem or device mismatch */
#define ENMFIL  8 /* No more files in directory */
#define EWRPTR  9 /* Wrong pointer */
#define EBINFMT 10 /* Binary format not recognized */
#endif
