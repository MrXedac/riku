/* Include a kernel header */
#include "vga.h"
#include "heap.h"

char* module_name()
{
	return "kernalloc";
}

char* module_description()
{
	return "A test kernel module which uses the kernel heap allocator";
}

void kputs(char* a)
{
	puts("\t[kernalloc] ");
	puts(a);
}

/**
 * This is a module which uses kalloc().
 * For testing purposes :)
 */

void module_init()
{
    	kputs("I should now allocate some space in kernel...\n");
	uintptr_t krn_alloc = (uintptr_t)kalloc(4 * sizeof(uintptr_t));
	kputs("Kernel gave me some space at ");
	puthex(krn_alloc);
	puts("!\nExiting module_init...\n");
	return;
}

