#include <stdint.h>
#include "vga.h"
#include "vm.h"
#include "serial.h"
#include "version.h"
#include "bootinfo.h"
#include "ldrinfo.h"
#include "gdt64.h"
#include "idt64.h"

/* Early-boot console init */
void init_terminal()
{
	extern uint16_t *video_memory;
	video_memory = (uint16_t*)0xB8000;
	cls();
}

void displayKernelInfo()
{
	puts(KERNEL_NAME);
	puts(" kernel, codename ");
	puts(KERNEL_CODENAME);
	puts(", version ");
	puts(KERNEL_VERSION);
	puts("\n\tRunning on ");
	puts(KERNEL_ARCH);
	puts(" (size of uintptr_t is ");
	putdec(sizeof(uintptr_t) * 8);
	puts(" bits)\n");
	
	extern uintptr_t __code;
	puts("\nKernel code at virtual address ");
	puthex((uintptr_t)&__code);
	puts("\n");
}

/* Loader entry-point. */
void main()
{
	vm_init();
	
	/* Initialize some stuff related to early-boot IO */
	init_serial();
	slputs("Welcome to Riku");
	init_terminal();

	/* Show we are alive ! */
	puts("Welcome to the Riku Operating System\n");
	displayKernelInfo();
	
	/* Get the Riku Loader info structure */
	puts("Parsing RikuLdr info structure.\n");
	struct rikuldr_info* info = (struct rikuldr_info*)LDRINFO_ADDR;
	puts("\tMultiboot header at ");
	puthex((uintptr_t)info->mbi_addr | 0xFFFF800000000000);
	puts("\n\tRikuLoader physical address at ");
	puthex((uintptr_t)info->ldr_paddr);
	puts("\n\tRiku kernel physical address at ");
	puthex((uintptr_t)info->krn_paddr);
	puts("\n\tGlobal descriptor table at ");
	puthex((uintptr_t)info->gdt_paddr);
	puts("\n\tGlobal descriptor table pointer at ");
	puthex((uintptr_t)info->gdtptr_paddr);
	puts("\n");
	
	/* Parse the multiboot info */
	puts("\nNow parsing Multiboot2 header info.\n");

	parse_mbi((uintptr_t)info->mbi_addr | 0xFFFF800000000000);
	
	puts("\nOkay, everything is fine. \nThanks RikuLdr, I'm taking care of the remaining stuff...\n");
	puts("gdt init ");
	gdt_init((uintptr_t)info->gdt_paddr, (uintptr_t)info->gdtptr_paddr);
	puts("complete\n");
	panic("early-boot complete", 0x0);
}
