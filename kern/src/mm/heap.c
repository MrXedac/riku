#include "heap.h"
#include "idt64.h"
#include "vga.h"
#include "vm.h"

#include <stdint.h>

extern uintptr_t __end;

block_header_t* next_block(block_header_t* block)
{
	block_header_t* next = (block_header_t*)((uintptr_t)block + (block->size));
	
	if((uintptr_t)next >= (uintptr_t)kernel_heap->heap_begin + kernel_heap->heap_size) {
		return 0;
	}
	
	if(next->magic == HEAP_HEADER_MAGIC)
		return next;
	else
		return 0;
}

block_header_t* prev_block(block_header_t* block)
{
	block_footer_t* prev = (block_footer_t*)((uintptr_t)block - sizeof(block_footer_t));
	if(prev->magic == HEAP_FOOTER_MAGIC)
		return prev->header;
	else
		return 0;
}

block_footer_t* get_footer(block_header_t* block)
{
	block_footer_t* ftr = (block_footer_t*)((uintptr_t)block + block->size - sizeof(block_footer_t));
	if(ftr->magic == HEAP_FOOTER_MAGIC)
		return ftr;
	else {
		panic("Kernel heap corrupted, failed to get heap footer", 0);
		return 0;
	}
}

uintptr_t* block_data_space(block_header_t* block)
{
	return (uintptr_t*)((uintptr_t)block + sizeof(block_header_t));
}

heap_t* build_heap(uintptr_t *heap_vaddr, uint8_t is_kernel, uint8_t is_read_only)
{
	// Allocate the structure for our heap
	heap_t *heap = (heap_t*)HEAP_BEGIN;
	
	// Our heap size if the diff between the two addresses, minus the sizes of the structures
	uint32_t heap_size = HEAP_END - HEAP_BEGIN - sizeof(heap_t) - sizeof(block_header_t) - sizeof(block_footer_t);
	puts("building kernel heap: heap, ");
	
	// Configure it
	heap->kernel_heap = is_kernel;
	heap->read_only = is_read_only;
	heap->heap_size = heap_size + sizeof(heap_t) + sizeof(block_header_t) + sizeof(block_footer_t);
	
	puts("header, ");
	
	// Create first element
	block_header_t *hdr = (block_header_t*)(HEAP_BEGIN + sizeof(heap_t)); //alloc_page_to_vaddr(heap_vaddr);
	hdr->flags = 0;
	hdr->magic = HEAP_HEADER_MAGIC;
	hdr->size = heap_size + sizeof(block_header_t) + sizeof(block_footer_t);
	
	puts("footer, ");
	
	block_footer_t *ftr = (block_footer_t*)((uintptr_t)hdr + hdr->size - sizeof(block_footer_t) /*- sizeof(block_footer_t)*/);
	ftr->magic = HEAP_FOOTER_MAGIC;
	ftr->header = hdr;
	
	// Insert first element into the heap
	heap->heap_begin = hdr;
	
	puts("all good\n");
	
	puts("built heap at ");
	puthex((uintptr_t)heap);
	puts(", header at ");
	puthex((uintptr_t)hdr);
	puts(", footer at ");
	puthex((uintptr_t)ftr);
	puts(", heap size ");
	putdec(heap_size / PAGE_SIZE * 4);
	puts("kio \n");
	// We're done, our heap is ready !
	return heap;
}

void init_kheap()
{
	kernel_heap = build_heap((uintptr_t*)HEAP_BEGIN, 1, 0);
	
	// Self-test
	puts("heap self-test\n");
	uint32_t *test_alloc = (uint32_t*)kalloc(sizeof(uint32_t));
	uint32_t *test_alloc_2 = (uint32_t*)kalloc(sizeof(uint32_t));
	
	puts("var1(uint32_t)=");
	puthex((uintptr_t)test_alloc);
	puts(", var2(uint32_t)=");
	puthex((uintptr_t)test_alloc_2);
	puts("\n");
	
	*test_alloc = 0x1234ABCD;
	*test_alloc_2 = 0xCAFEBEEF;
	
	kfree((uintptr_t*)test_alloc);
	kfree((uintptr_t*)test_alloc_2);
	
	uint32_t* test_alloc_3 = (uint32_t*)kalloc(sizeof(uint64_t));
	puts("var1 and var2 freed, var3(uint64_t)=");
	puthex((uintptr_t)test_alloc_3);
	puts(", should be ");
	puthex((uintptr_t)test_alloc);
	puts("\n");
	
	if(test_alloc_3 == test_alloc)
		puts("self-test succeeded\n");
	
	kfree((uintptr_t*)test_alloc_3);
}

void expand_left(block_header_t* block)
{
	/* Get previous block */
	block_footer_t* prev_ftr = (block_footer_t*)((uintptr_t)block - sizeof(block_footer_t));
	
	if((uintptr_t)prev_ftr <= (uintptr_t)(HEAP_BEGIN + sizeof(heap_t)))
		return;
	
	block_header_t* prev_hdr = prev_ftr->header;
	if(prev_hdr->magic != HEAP_HEADER_MAGIC)
		panic("Kernel heap corrupted", 0);
	
	/* If block is free, expand it */
	if(prev_hdr->flags == 0)
	{
		block_footer_t* ftr = get_footer(block);
		ftr->header = prev_hdr;
		prev_hdr->size += block->size;
		
	}
}

void expand_right(block_header_t* block)
{
	/* Get next block */
	block_footer_t* ftr = get_footer(block);
	block_header_t* next_hdr = (block_header_t*)((uintptr_t)ftr + sizeof(block_footer_t));
	
	if((uintptr_t)next_hdr >= (uintptr_t)(HEAP_END))
		return;
	
	if(next_hdr->magic != HEAP_HEADER_MAGIC)
		panic("Kernel heap corrupted", 0);
	
	/* If block is free, expand it */
	if(next_hdr->flags == 0)
	{
		block_footer_t* next_ftr = get_footer(next_hdr);
		next_ftr->header = block;
		block->size += next_hdr->size;
		
		block_footer_t* oui = get_footer(block);
		/* Routine checks. */
		if(oui->magic != HEAP_FOOTER_MAGIC)
			panic("Kernel heap corrupted during expand", 0);
		
		if(oui->header->magic != HEAP_HEADER_MAGIC)
			panic("Kernel heap corrupted during expand (2)", 0);
		
	}
}

void kfree(uintptr_t* ptr)
{
	/* Get block header */
	block_header_t* hdr = (block_header_t*)((uintptr_t)ptr - sizeof(block_header_t));
	hdr->flags = 0;

	/* Expand block left and right if needed */
	expand_right(hdr);
	expand_left(hdr);
}

uintptr_t* kalloc(uint32_t asize)
{
	uint32_t size;
	if(!kernel_heap)
		return 0;
	
	// Align allocation size on uint32_t
	uint8_t mod = asize % 4;
	if(mod > 0)
		size = asize + 4 - (uint32_t)mod;
	else
		size = asize;
	
	uint32_t required_size = size + 2*(sizeof(block_footer_t) + sizeof(block_header_t));
	
	block_header_t* blk = kernel_heap->heap_begin;
	
	// Iterate in our heap, and stops when we found a block, or there is no remaining blocks
	while((blk->size < required_size ||blk->flags == 1) && next_block(blk))
		blk = next_block(blk);

	// We found a block !
	if(blk->size >= required_size && blk->flags == 0 && blk->magic == HEAP_HEADER_MAGIC)
	{
		if(blk->size == required_size) {
			blk->flags = 1;
			return block_data_space(blk);
		} else {
			// Update block data
			block_footer_t* footer = get_footer(blk);
			uint32_t old_size = blk->size;
			blk->size = size + sizeof(block_header_t) + sizeof(block_footer_t);
			blk->flags = 1;
			
			// Create footer for block
			block_footer_t* new_footer = (block_footer_t*)((uintptr_t)blk + blk->size - sizeof(block_footer_t));
			new_footer->header = blk;
			new_footer->magic = HEAP_FOOTER_MAGIC;

			// Create new block next to it
			block_header_t* new_block = (block_header_t*)((uintptr_t)new_footer + sizeof(block_footer_t));
			new_block->magic = HEAP_HEADER_MAGIC;
			new_block->flags = 0;
			new_block->size = old_size - blk->size;
			
			// Update footer data to link to the newly created block
			footer->header = new_block;
			
			block_footer_t* check = get_footer(new_block);
			block_footer_t* check2 = get_footer(blk);
			return block_data_space(blk);
		}
	} else {
		panic("Kernel heap out of memory\n", 0);
		return 0;
	}
}