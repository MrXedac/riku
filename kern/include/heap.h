#ifndef __HEAP_H__
#define __HEAP_H__

#include <stdint.h>

extern uintptr_t __end;

#define HEAP_BEGIN			(((uintptr_t)&__end | 0xFFFF800000000000))
#define HEAP_END			(((uintptr_t)&__end | 0xFFFF800000000000) + 0x400000)
#define HEAP_HEADER_MAGIC	0xCAFEABCD
#define HEAP_FOOTER_MAGIC	0xBEEFDCBA

typedef struct
{
    uint32_t magic; // = HEAP_HEADER_MAGIC
    uint32_t flags; // 0 = free, 1 = in use
    uint32_t size; // size of the block, including header & footer
} block_header_t;

typedef struct
{
    uint32_t magic; // = HEAP_FOOTER_MAGIC
    block_header_t* header; // = Pointer to the header of the block
} block_footer_t;

typedef struct
{
    block_header_t *heap_begin; // First block of the heap
    uint32_t heap_size; // Size of the heap
    uint8_t kernel_heap; // Is it the kernel heap ?
    uint8_t read_only; // Do the extra pages mapped by this heap are read-only ?
} heap_t;

heap_t *kernel_heap; /* Kernel main heap */

uintptr_t* kalloc(uint32_t asize);
void kfree(uintptr_t* ptr);
void init_kheap();

#endif