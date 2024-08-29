#include "vm.h"
#include "multiboot.h"
#include "vga.h"
#include "serial.h"
#include <stdint.h>

/* Initializes the MMU during early-boot */
void vm_init()
{
	uint64_t* pdpt = (uint64_t*)EARLY_PDPT_KERN; // PDPT pointer
	uint64_t* pdptk = (uint64_t*)EARLY_PDPT_PHYS; // Kernel PDPT

	uint64_t gbOffset = 0x40000000; // Address offset corresponding to one PDPT entry
	uint64_t i = 0;

	// Fill-in every entry of the PDPT with 1-1 mapping of the address space
	for(i = 0; i < 512; i++)
	{
		*(pdpt) = (i * gbOffset) | FLAGS_PDPT_LARGE; // Target address + large page flag
		*(pdptk) = (i * gbOffset) | FLAGS_PDPT_LARGE;
		pdpt = (uint64_t*)((uint64_t)pdpt + 0x8); // Next entry
		pdptk = (uint64_t*)((uint64_t)pdptk + 0x8);
	}

}

/* Adds a memory range to the pagination list */
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
			if(i > (((uintptr_t)&__end & 0x000000FFFFFFFFFF) + 0x400000) && i < 0x800000000000) { /* Ignore higher half, we're working on physical address space right now */
				*(uintptr_t*)i = (uintptr_t)first_free_page;
				first_free_page = (uintptr_t*)i;
				max_pages++;
			}
		}
		printk("[%x-%x] : Paging area\n", (uintptr_t)base, (uintptr_t)base + length);
	} else {
		puts("skipped low-memory paging area\n");
	}
}

/* Given a multiboot2 memory map, initializes memory space */
void mmap_init(struct multiboot_tag_mmap *mmap_tag_ptr)
{
	max_pages = 0;
	first_free_page = (uintptr_t*)0x0;
	allocated_pages = 0;
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

/* Page allocator and deallocator */
uintptr_t* alloc_page()
{
	uintptr_t* res = first_free_page;
	first_free_page = (uintptr_t*)(*(uintptr_t*)((uintptr_t)res | 0xFFFF800000000000));
	uintptr_t i = 0;
	for(i = 0; i < 512; i++)
		*(uintptr_t*)(((uintptr_t)res + i * 0x8) | 0xFFFF800000000000) = 0x0;

	allocated_pages++;
	return res;
}

void free_page(uintptr_t *page)
{
	*page = (uintptr_t)first_free_page;
	first_free_page = (uintptr_t*)page;

	allocated_pages--;
}
