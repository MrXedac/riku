#include "vm.h"
#include "multiboot.h"
#include "vga.h"
#include <stdint.h>

void vm_init()
{
	uint64_t* pdpt = (uint64_t*)0x3000; // PDPT pointer
	uint64_t* pdptk = (uint64_t*)0x2000; // Kernel PDPT
	
	uint64_t gbOffset = 0x40000000; // Address offset corresponding to one PDPT entry
	uint64_t i = 0;
	
	// Fill-in every entry of the PDPT with 1-1 mapping of the address space
	for(i = 0; i < 512; i++)
	{
		*(pdpt) = (i * gbOffset) | 0x83; // Target address + large page flag
		*(pdptk) = (i * gbOffset) | 0x83;
		pdpt = (uint64_t*)((uint64_t)pdpt + 0x8); // Next entry
		pdptk = (uint64_t*)((uint64_t)pdptk + 0x8);
	}
	
}

void paging_init(uintptr_t* base, uint64_t length)
{
	if(base >= (uintptr_t*)0x100000) /* RikuLdr loading address */
	{
		uint64_t i = 0;
		extern uintptr_t __end;
		puts("vm init: base ");
		puthex((uintptr_t)base);
		puts(", length ");
		puthex(length);
		puts("\n");
		for(uintptr_t i = (uintptr_t)base; i < ((uintptr_t)base + length); i+=0x1000)
		{
			/* Don't overwrite the kernel + Haskell heap stuff, and avoid erasing high-memory kernel space */
			if(i > ((uintptr_t)&__end & 0x000000FFFFFFFFFF) && i < 0x800000000000) { /* Ignore higher half, we're working on physical address space right now */
				*(uintptr_t*)i = (uintptr_t)first_free_page;
				first_free_page = (uintptr_t*)i;
				max_pages++;
			}
		}
	} else {
		puts("skipped low-memory paging area\n");
	}
}


void mmap_init(struct multiboot_tag_mmap *mmap_tag_ptr)
{
	max_pages = 0;
	first_free_page = (uintptr_t*)0x0;
	multiboot_memory_map_t* mmap;
	struct multiboot_tag *tag = (struct multiboot_tag *)mmap_tag_ptr;
	for (mmap = (mmap_tag_ptr)->entries;
		 (multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size;
		 mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size))
	{
		puts("region ");
		puthex((uint64_t) mmap->addr);
		puts(", length ");
		puthex((uint64_t) mmap->len);
		puts(", type ");
		puthex((uintptr_t)mmap->type);
		puts(": ");
		switch(mmap->type)
		{
			case MULTIBOOT_MEMORY_AVAILABLE:
				puts("AVAILABLE\n");
				/* Update the free page list */
				paging_init((uintptr_t*)mmap->addr, (uint64_t)mmap->len);
				break;
			case MULTIBOOT_MEMORY_RESERVED:
				puts("RESERVED\n");
				break;
			case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
				puts("ACPI RECLAIMABLE\n");
				break;
			case MULTIBOOT_MEMORY_NVS:
				puts("NVS\n");
				break;
			case MULTIBOOT_MEMORY_BADRAM:
				puts("BADRAM\n");
				break;
		}
	}
	puts("vm initialization complete, ");
	putdec(max_pages);
	puts(" free pages (");
	putdec(max_pages * 4);
	puts("kb total)\n");
	return;
}
