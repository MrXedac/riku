#include "stream.h"
#include <stdbool.h>
#include <stdint.h>
#include "printk.h"
#include "string.h"

void stream_init(riku_stream_t* stream)
{
    printk("stream: init IO stream at %x\n", stream);
    stream->read_ptr = 0;
    stream->write_ptr = 0;
    stream->state = STREAM_EMPTY;
    memset(stream->data, 0, STREAM_SIZE);
}

void stream_write(riku_stream_t* stream, char c)
{
    if(stream->state == STREAM_FULL) 
    {
        printk("stream: cannot write %c in stream %x, stream is full\n", c, stream);
        return;
    }

    /* Write data */
    stream->data[stream->write_ptr] = c;
    stream->write_ptr++;

    /* Roll ! */
    if(stream->write_ptr == STREAM_SIZE)
        stream->write_ptr = 0;

    /* Update stream state */
    if(stream->write_ptr == stream->read_ptr)
        stream->state = STREAM_FULL;
    else
        stream->state = STREAM_NONEMPTY;
}

char stream_read(riku_stream_t* stream, bool blocking)
{
    /* Okay, this is insane. But right now, we freeze stuff until something happens */
    /* But as I freeze interrupts in the kernel, duh */
    /* Crap */
    if(blocking && stream->state == STREAM_EMPTY) {
        __asm volatile("sti");
    }

    while(blocking && stream->state == STREAM_EMPTY)
    {};

    /* Refreeze interrupts */
    __asm volatile("cli");

    /* Non-blocking read : return 0 if stream is empty */
    if(stream->state == STREAM_EMPTY)
        return (char)0;

    /* Read data */
    char res = stream->data[stream->read_ptr];

    /* DO A BARREL ROLL */
    stream->read_ptr++;
    if(stream->read_ptr == STREAM_SIZE)
        stream->read_ptr = 0;

    /* If we were full, say we are not full anymore */
    if(stream->state == STREAM_FULL)
        stream->state = STREAM_NONEMPTY;
    
    /* Did we empty the stream ? */
    if(stream->read_ptr == stream->write_ptr)
        stream->state = STREAM_EMPTY;

    return res;
}