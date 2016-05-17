#include <stdint.h>
#include "vga.h"
#include "vm.h"
#include "serial.h"
#include "version.h"
#include "bootinfo.h"
#include "ldrinfo.h"
#include "gdt64.h"
#include "idt64.h"
#include "bga.h"
#include "mmu.h"

/* Early-boot console init */
void init_terminal()
{
	extern uint16_t *video_memory;
	video_memory = (uint16_t*)0xFFFF8000000B8000;
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

void showDisclaimer()
{
	puts("\n");
	puts("The Riku Operating System\n");
	puts("\t(c) Quentin \"MrXedac\" Bergougnoux - 2014-2016\n");
	puts("\tThis product is open-source, released under the GNU GPL licence.\n");
	puts("\tThis software is a pre-release software and isn't ready for an everyday usage yet.\n");
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
	BgaSetVideoMode(BGA_WIDTH, BGA_HEIGHT, 32, 1, 1);
	
	/* Show we are alive ! */
	puts("Welcome to the Riku Operating System\n");
	puts("Early console: ");
	putdec(BGA_WIDTH);
	puts("x");
	putdec(BGA_HEIGHT);
	puts(", ");
	putdec(BGA_CXMAX);
	puts("x");
	putdec(BGA_CYMAX);
	puts(" console\n");
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
	puts("idt init ");
	idt_init();
	puts("complete\n");
	//__asm volatile("INT $0x14;");
	puts("mmu init ");
	mmu_init();
	puts("complete\n");
	puts("WARNING: pure long-mode gdt madness here, looks like gdt HAS to be mapped into MMU for the system to work. this is insanity\n");
	
	puts("kernel pml4t at ");
	puthex((uintptr_t)kernel_cr3);
	puts("\nkernel master table/pdpt at ");
	puthex((uintptr_t)masterTable);
	puts("\n");
	puts("early-boot complete, now starting kernel tasks\n");
	
	showDisclaimer();
	
	//__asm volatile("INT $0x11");
	panic("threading initialization failed: not implemented", 0x0);
}
