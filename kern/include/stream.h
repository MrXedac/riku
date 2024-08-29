#ifndef __STREAM__
#define __STREAM__

#include <stdint.h>
#include <stdbool.h>

/* Amount of char a stream holds */
#define STREAM_SIZE 10

typedef enum riku_stream_state { STREAM_EMPTY, STREAM_NONEMPTY, STREAM_FULL } riku_stream_state_t;

struct riku_stream {
    char data[STREAM_SIZE];
    int read_ptr;
    int write_ptr;
    riku_stream_state_t state;
};

typedef struct riku_stream riku_stream_t;

void stream_init(riku_stream_t* stream);

/* Writes to a stream, canceling if the stream has not enough space */
void stream_write(riku_stream_t* stream, char c);

/* Reads from a stream, blocking until a data is available - dangerous !! */
char stream_read(riku_stream_t* stream, bool blocking);

#endif