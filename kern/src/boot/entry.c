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
#include "heap.h"
#include "task.h"
#include "hw.h"
#include "kernsym.h"
#include "sched.h"
#include "driver.h"
#include "printk.h"
#include "sys.h"

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
	puts("\t(c) Quentin \"MrXedac\"/Mk. Bergougnoux - 2014-2017\n");
	puts("\tThis product is open-source, released under the GNU GPL licence.\n");
	puts("\tThis software is a pre-release software and isn't ready for an everyday usage yet.\n");
	puts("\n");
}

void dummy2()
{
	puts("Dummy task started.\n");
	__asm volatile("STI");
	while(1)
		puts("2");
}

void return_from_sysenter()
{
	printk("Returned from sysenter. Everything is fine.\n");
	for(;;);
}

#define INIT_TASK(s, f) { printk("-> Init task \"%s\"\n", s); f(); }
void late_init_tasks()
{
	INIT_TASK("disclaimer", showDisclaimer);
	INIT_TASK("setup_sched", setup_sched);
	INIT_TASK("probe_hardware", probe_hardware);

	/* Initialize some required drivers. */
	/* x86serial is responsible for kconsole. */
	DRIVER_INIT(x86serial);
	/* x86vga is responsible for console. */
	DRIVER_INIT(x86vga);

	INIT_TASK("init_sysenter", init_sysenter);

	/*printk("Running SYSCALL test.\n");*/
	/* Set RCX to RSP, RDX to return RIP, RAX to system call id, and SYSENTER */
	/*__asm volatile("MOV %%RSP, %%RCX; \
					MOV %0, %%RDX; \
					MOV	$0x3, %%RAX; \
					SYSCALL;"
				   :: "r"(&return_from_sysenter)
				   : "rax", "rcx", "rdx");*/

	/*panic("Shouldn't be here.\n", 0);*/

	if(!printk_enabled)
		panic("Couldn't find a suitable device for printk().\n", 0);
}

/* When we enter this function, we should have a proper page allocator, interrupt handling and working threading.
 * We can initialize the "later" boot tasks, and then spawn our init task. */
void late_init()
{
	/* Some day I'll put something really extensible here. Right now I don't care, so... */
	late_init_tasks();

	/* At this point we have a working printk() if everything went fine */
	printk("Entered boot stage 2\n");

	/* Allright, let's build a proper environment for the init process */
	uintptr_t init_vme = build_new_vme();
	switch_cr3(init_vme);

	printk("Switched to init vme\n");

	/* Grab init from Multiboot2, put it in an appropriate location and start in userland */
	printk("Preparing to spawn init\n");
	spawn_init(((struct rikuldr_info*)(PHYS(LDRINFO_ADDR)))->mbi_addr, init_vme);

	for(;;);
}

/* Loader entry-point. */
void main()
{
	vm_init();

	/* Initialize some stuff related to early-boot IO */
	init_serial();
	printk("64-bits entrypoint reached\n");
	printk("The Riku Operating System - MrXedac(c)/Mk.(c) 2017\n");
	printk("Initializing early-boot console\n");
	init_terminal();
  // BgaSetVideoMode(BGA_WIDTH, BGA_HEIGHT, 32, 1, 1);

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

	puts("\nOkay, everything is fine. \nThanks RikuLdr, I'm taking care of the remaining stuff...\n");

	/* Enter platform initialization */
	puts("gdt init ");
	gdt_init((uintptr_t)info->gdt_paddr, (uintptr_t)info->gdtptr_paddr);
	puts("complete\n");
	puts("idt init ");
	idt_init();
	puts("complete\n");
	//__asm volatile("INT $0x14;");
	/* Parse the multiboot info */
	puts("\nNow parsing Multiboot2 header info.\n");
	printk("Parsing MB2I\n");
	parse_mbi((uintptr_t)info->mbi_addr | 0xFFFF800000000000);

	puts("mmu init ");
	mmu_init();
	puts("complete\n");
	printk("GDT, IDT and MMU initialization complete\n");
	puts("kernel pml4t at ");
	puthex((uintptr_t)kernel_cr3);
	puts("\nkernel master table/pdpt at ");
	puthex((uintptr_t)masterTable);
	puts("\n");

	init_kheap();

	/* We should start modules at a later point */
	/* puts("starting kernel modules\n");
	start_modules((uintptr_t)info->mbi_addr | 0xFFFF800000000000); */

	puts("Early-boot complete.\n");

	printk("Preparing to enter boot stage 2\n");
	/* Some tasking */
	struct riku_task* dummyTask = (struct riku_task*)kalloc(sizeof(struct riku_task));
	uintptr_t* usrstack = alloc_page();
	uintptr_t* krnstack = alloc_page();
	puts("Riku dummy task at ");
	puthex((uintptr_t)dummyTask);
	puts("\n");
	printk("Stage 2 boot context RSP %x, USP %x\n", krnstack, usrstack);
	/* First prepare task */
	init_task(dummyTask, "Riku init", (uintptr_t*)((uintptr_t)(usrstack) | 0xFFFF800000000000), (uintptr_t*)((uintptr_t)(krnstack) | 0xFFFF800000000000), &late_init, (uintptr_t*)kernel_cr3);
	switch_to_task(dummyTask);

	panic("I shouldn't have returned here.", 0x0);
}
