#include <stdint.h>
#include "vga.h"
#include "vm.h"
#include "ldrinfo.h"
#include "ioport.h"

extern void enter_long_mode(uintptr_t mboot_ptr);
extern void enable_sse();
extern int long_mode_check();
extern int large_page_check();

/* Early-boot console init */
void init_terminal()
{
	cls();

    /* Disables cursor */
   	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

/* Fill in the loader info structure */
void fill_ldrinfo(uintptr_t* mboot_info)
{
	struct rikuldr_info* info = (struct rikuldr_info*)LDRINFO_ADDR;
	info->mbi_addr = (uint32_t)mboot_info;
	extern uint32_t __code;
	extern uint32_t __kernel;
	extern uint32_t GDT64;
	extern uint32_t GDT64Pointer;
	info->ldr_paddr = (uint32_t)&__code;
	info->krn_paddr = (uint32_t)&__kernel;
	info->gdt_paddr = (uint32_t)&GDT64;
	info->gdtptr_paddr = (uint32_t)&GDT64Pointer;
}

/* Loader entry-point. */
void main(uintptr_t* mboot_info)
{
	/* Initialize some stuff related to early-boot IO */
	init_terminal();

	/* Check 64 bits support */
	if(long_mode_check() == 1) 
	{
		puts("Long mode supported. Continuing boot.");
		puts("\n");
	} else {
		puts("64 bits not supported by CPU. Aborting boot.\n");
		__asm volatile("CLI; HLT;");
		for(;;);
	}

	/* Check 1gb page support */
	if(large_page_check() == 1) 
	{
		puts("Large pages supported. Continuing boot.");
		puts("\n");
	} else {
		puts("1gb large pages not supported by CPU. Aborting boot.\n");
		__asm volatile("CLI; HLT;");
		for(;;);
	}


	/* Show we are alive ! */
	puts("Riku Loader - The Riku Operating System\n");
	puts("MBI at ");
	puthex((uintptr_t)mboot_info);
	puts("\n");
	puts("Kernel at ");
	extern uintptr_t kernel;
	puthex((uintptr_t)&kernel);
	puts("\n");

	vm_init();
	fill_ldrinfo(mboot_info);
	enable_sse();
    	
	extern void fixvga();
	fixvga();

	enter_long_mode((uintptr_t)mboot_info);
	for(;;);
}
