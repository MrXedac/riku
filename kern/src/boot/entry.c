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
#include "vfs/openclose.h"
#include "vfs/readwrite.h"
#include "vfs/dup2.h"
#include "fs_vfat.h"
#include "vfs/mount.h"
#include "vfs/fs.h"
#include <string.h>
#include "kconfig.h"

uintptr_t initramfs_begin = 0x0;

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

void init_vfs()
{
	printk("Initializing virtual filesystem. Mounting devfs properly.\n");
	/* Declare devfs filesystem driver */
	extern struct riku_filesystem fs_devfs;
    extern struct riku_filesystem fs_ustarfs;
	extern struct riku_filesystem fs_ext2fs;
	extern struct riku_devfs_node* devfsVirtPtr;

    /* Find initramfs devnode */
    struct riku_devfs_node* ramfsVirtPtr = devfs_find_node("initramfs");

    /* Mount /dev as A: */
	mount_internal(devfsVirtPtr, &fs_devfs);

    /* Mount /dev/initramfs as B: */
    mount_internal(ramfsVirtPtr, &fs_ustarfs);

	/* Attempt to mount ata0p1 as C: */
	struct riku_devfs_node* root_device = devfs_find_node("ata0a");
	if(root_device != 0x0)
	{
		mount_internal(root_device, &fs_ext2fs);
	}

	/* Try the devfs mountpoint */
	struct riku_fileinfo devfs_dir, devfs_node;
	/* TODO : implement generic readdir, opendir and stuff... right now we'll call the driver directly */
	fs_devfs.opendir(&mounts[0], "/", &devfs_dir);

	printk("Contents for mountpoint A:/ (devfs):\n")
	while(!fs_devfs.readdir(&mounts[0], &devfs_dir, 0, &devfs_node))
	{
		printk("-> A:/%s\n", ((struct riku_devfs_node*)(devfs_node.extended))->name);
	}

	printk("Contents for mountpoint B:/ (initramfs):\n")
    struct riku_fileinfo ramfs_dir, ramfs_node;
    memset(&ramfs_dir, 0, sizeof(ramfs_dir));
    while(!fs_ustarfs.readdir(&mounts[1], &ramfs_dir, 0, &ramfs_node))
    {
        /* Extended -> ptr in ustar */
        /* While we don't have a fs_stat yet to find name, size etc, do it manually : ptr refers to name, ptr + 512 = pointer to data */
        char* name = (char*)ramfs_node.extended;
        uintptr_t data = (uintptr_t)ramfs_node.extended + 512;
        printk("-> B:/%s\n", name, data);
        if(!strcmp(name, "banner"))
            printk("%s", (char*)data);
    }
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
    DRIVER_INIT(initramfs);

	/* ps2kb is responsible for default input */
	DRIVER_INIT(ps2kb);
	
	/* ata_pio is broken yet */
	#ifdef CONFIG_ENABLE_ATA
	DRIVER_INIT(ata_pio);
	#ifdef CONFIG_ENABLE_MBR
	DRIVER_INIT(mbr);
	#endif
	#endif

	INIT_TASK("init_sysenter", init_sysenter);

	INIT_TASK("init_vfs", init_vfs);

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

void kidle()
{
	for(;;)
	{
		__asm volatile("sti");
	}
}

/* When we enter this function, we should have a proper page allocator, interrupt handling and working threading.
 * We can initialize the "later" boot tasks, and then spawn our init task. */
void late_init()
{
	struct riku_task* idleTask = (struct riku_task*)kalloc(sizeof(struct riku_task));
	uintptr_t* idlestack = alloc_page();
	uintptr_t* idlekrnstack = alloc_page();
 	init_task(idleTask, "Riku idle", (uintptr_t*)((uintptr_t)(idlestack) | 0xFFFF800000000000), (uintptr_t*)((uintptr_t)(idlekrnstack) | 0xFFFF800000000000), &kidle, (uintptr_t*)kernel_cr3);
	
    /* Find initramfs */
    initramfs_begin = find_initramfs(((struct rikuldr_info*)(PHYS(LDRINFO_ADDR)))->mbi_addr);
    printk("ramfs: found initramfs at %x\n", initramfs_begin);

	/* Some day I'll put something really extensible here. Right now I don't care, so... */
	late_init_tasks();

	/* At this point we have a working printk() if everything went fine */
	printk("Entered boot stage 2\n");

	/* Allright, let's build a proper environment for the init process */
	uintptr_t init_vme = build_new_vme();
	switch_cr3(init_vme);
	update_task_vme(current_task, init_vme);

	printk("Switched to init vme\n");

	/* Grab init from Multiboot2, put it in an appropriate location and start in userland */
	printk("Preparing to spawn init\n");
    uintptr_t init_addr = 0x0, init_size = 0x0;

    /* Find init in initramfs; TODO : use correct API instead of doing everything by hand */
    extern struct riku_filesystem fs_ustarfs;
    struct riku_fileinfo ramfs_dir, ramfs_node;
    memset(&ramfs_dir, 0, sizeof(ramfs_dir));
    while(!fs_ustarfs.readdir(&mounts[1], &ramfs_dir, 0, &ramfs_node))
    {
        char* name = (char*)ramfs_node.extended;
        extern int oct2bin(unsigned char*, int);

        init_size = (uintptr_t)oct2bin(ramfs_node.extended + 0x7c, 11);
        if(!strcmp(name, CONFIG_INIT_PROCESS))
            init_addr = (uintptr_t)ramfs_node.extended + 512;
    }
	printk("init binary at %x\n", init_addr);
    uint64_t rip = spawn_init(init_addr, init_size, init_vme);

	/* Map user stack and kernel stack somewhere safe */
	printk("user rsp %x, kern rsp %x\n", LIN(current_task->task_rsp), LIN(current_task->kernel_rsp));
	
	uintptr_t* fresh_user_stack = alloc_page();
	vme_map(init_vme, LIN((uintptr_t)fresh_user_stack), INIT_STACK, 1);
	vme_map(init_vme, LIN(current_task->kernel_rsp), INIT_KERN_STACK, 0);
	
	current_task->task_rsp = INIT_STACK + PAGE_SIZE - sizeof(uintptr_t);
	current_task->task_rbp = INIT_STACK + PAGE_SIZE - sizeof(uintptr_t);
	current_task->kernel_rsp = INIT_KERN_STACK + PAGE_SIZE - sizeof(uintptr_t);

	/* Open stdin, stdout, stderr to devfs:/null, devfs:/vga0 and devfs:/vga0 */
	uint32_t stdin = open("A:/kb0", 0x1);
	uint32_t stdout = open("A:/vga0", 0x1);
	uint32_t stderr = dup2(stdout);
	printk("Files descriptor opened : %d, %d, %d\n", stdin, stdout, stderr);
	printk("Switching in userland (rip=%x) and dropping kernel boot context\n", rip);

	extern void enter_userland(uint64_t rip, uint64_t rsp);
	enter_userland(rip, INIT_STACK + PAGE_SIZE - sizeof(uintptr_t));

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
    
	#ifdef CONFIG_ENABLE_BOCHS_VGA
	BgaSetVideoMode(BGA_WIDTH, BGA_HEIGHT, 32, 1, 1);
	#endif

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
	struct riku_task* idleTask = (struct riku_task*)kalloc(sizeof(struct riku_task));
	uintptr_t* usrstack = alloc_page();
	uintptr_t* krnstack = alloc_page();
	uintptr_t* idlestack = alloc_page();
	uintptr_t* idlekrnstack = alloc_page();
	puts("Riku dummy task at ");
	puthex((uintptr_t)dummyTask);
	puts("\n");
	printk("Stage 2 boot context RSP %x, USP %x\n", krnstack, usrstack);
	/* First prepare task */
	init_task(dummyTask, "Riku init", (uintptr_t*)((uintptr_t)(usrstack) | 0xFFFF800000000000), (uintptr_t*)((uintptr_t)(krnstack) | 0xFFFF800000000000), &late_init, (uintptr_t*)kernel_cr3);
	switch_to_task(dummyTask);

	panic("I shouldn't have returned here.", 0x0);
}
