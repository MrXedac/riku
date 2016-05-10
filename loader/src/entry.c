#include <stdint.h>
#include "vga.h"
#include "vm.h"
#include "ldrinfo.h"

extern void enter_long_mode(uintptr_t mboot_ptr);

/* Early-boot console init */
void init_terminal()
{
	cls();
}

/* Fill in the loader info structure */
void fill_ldrinfo(uintptr_t* mboot_info)
{
	struct rikuldr_info* info = (struct rikuldr_info*)LDRINFO_ADDR;
	info->mbi_addr = (uint32_t)mboot_info;
	extern uint32_t __code;
	extern uint32_t __kernel;
	info->ldr_paddr = (uint32_t)&__code;
	info->krn_paddr = (uint32_t)&__kernel;
}

/* Loader entry-point. */
void main(uintptr_t* mboot_info)
{
	/* Initialize some stuff related to early-boot IO */
	init_terminal();

	/* Show we are alive ! */
	puts("Riku Loader - The Riku Operating System\n");
	puts("MBI at ");
	puthex((uintptr_t)mboot_info);
	puts("\n");
	vm_init();
	fill_ldrinfo(mboot_info);
	enter_long_mode((uintptr_t)mboot_info);
	for(;;);
}
