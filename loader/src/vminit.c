#include <stdint.h>
#include "vm.h"
#include "vga.h"
#include "mem.h"

void vm_init()
{
	puts("Initializing x64 translation tables.\n");
	puts("\tPML4T... ");
	/* Clear the PML4T and link physical and kernel PDPT */
	uintptr_t pml4t = EARLY_PML4T;
	memset((void*)pml4t, 0, PAGE_SIZE);
	*(uintptr_t*)pml4t = EARLY_PDPT_KERN | FLAGS_PML4T;
	puthex(*(uintptr_t*)pml4t);
	puts(" ");
	*(uintptr_t*)(pml4t + PML4T_UPPER * 0x8) = EARLY_PDPT_PHYS | FLAGS_PML4T;
	puthex(*(uintptr_t*)(pml4t + PML4T_UPPER * 0x8));
	puts(" Ok\n\tPDPT_PHYS... ");
	
	/* Same stuff for higher-mem PDPT */
	uintptr_t pdpt_phys = EARLY_PDPT_PHYS;
	memset((void*)pdpt_phys, 0, PAGE_SIZE);
	*(uintptr_t*)pdpt_phys = 0x0 | FLAGS_PDPT_LARGE;
	puthex(*(uintptr_t*)pdpt_phys);
	
	puts(" Ok\n\tPDPT_KERN... ");
	/* Same stuff for kernel PDPT */
	uintptr_t pdpt_kern = EARLY_PDPT_KERN;
	memset((void*)pdpt_kern, 0, PAGE_SIZE);
	*(uintptr_t*)pdpt_kern = 0x0 | FLAGS_PDPT_LARGE;
	puthex(*(uintptr_t*)pdpt_kern);
	
	puts(" Ok\n");
}
