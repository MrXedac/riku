#ifndef __STDIO__
#define __STDIO__

#include <stdint.h>

/* size_t and signed size_t defines */
typedef int64_t ssize_t;
typedef uint64_t size_t;

/* Standard kernel I/O */
int open (const char *filename, int flags);
int close (int filedes);
ssize_t read (int filedes, void *buffer, size_t size);
ssize_t write (int filedes, const void *buffer, size_t size);

struct rlibc_file {
  uint64_t fd;
  uint8_t modeflags; /* Read, write */
  uint64_t position;
};

typedef struct rlibc_file FILE;

/* Defined in stream.c */
FILE f_stdin;
FILE f_stdout;
FILE f_stderr;

#define stdin (&f_stdin)
#define stdout (&f_stdout)
#define stderr (&f_stderr)

/* File abstractions */
FILE * fopen(const char* file, const char* mode);
void fclose(FILE * file);
int printf ( const char * format, ... );
int putc ( int character, FILE * stream );

#endif
