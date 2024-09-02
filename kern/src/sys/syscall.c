#include <stdint.h>
#include "sys.h"
#include "idt64.h"
#include "printk.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "vfs/openclose.h"
#include "vfs/readwrite.h"
#include "vfs/dup2.h"
#include "vfs/putgetc.h"
#include "vfs/dir.h"
#include "task.h"
#include "mem.h"

void syscall_entry();

uint64_t console_raw_print(char* a)
{
	/*printk("raw console write requested, char at %x, prints %s\n", a, a);*/
	console->write(console, a, strlen(a));
	return 0;
}

void *syscall_table[SYSCALL_COUNT] =
{
	&console_raw_print,
	&open,
	&close,
	&read,
	&write,
	&putc,
	&dup2,
	&getpid,
	&getppid,
	&fork,
	&execve,
	&exit,
	&wait,
	&opendir,
	&readdir,
	&sbrk,
	&cwd,
	&gwd
};

void init_sysenter()
{
	/* First setup SYSENTER CS */
	__asm volatile("MOV $0x174, %RCX;\
				   MOV $0x0, %RDX;\
				   MOV $0x8, %RAX;\
				   WRMSR;");

	/* Setup SYSENTER RSP */
	__asm volatile("MOV $0x175, %RCX; \
				   MOV $0x00000000FFFF8000, %RDX; \
				   MOV $0x0000000002080FFF, %RAX; \
				   WRMSR;");

	/* Finally setup SYSENTER RIP */
	extern void syscall_ep();
	uint64_t rax, rdx;
	rdx = ((uint64_t)(&syscall_ep) >> 32) & 0x00000000FFFFFFFF;
	rax = (uint64_t)(&syscall_ep) & 0x00000000FFFFFFFF;
	printk("SYSENTER entry at %x:%x\n", rdx, rax);

	__asm volatile("MOVQ $0x176, %%RCX; \
				   MOVQ %0, %%RDX; \
				   MOVQ %1, %%RAX; \
				   WRMSR;"
				   :
				   : "r"(rdx), "r"(rax)
				   : "rax", "rdx", "rcx");

	/* Now configure AMD's SYSCALL/SYSRET */
	__asm volatile("MOV $0xC0000081, %RCX; \
				   MOV $0x00100008, %RDX; \
				   MOV $0xCAFEBABE, %RAX; \
				   WRMSR;");


	__asm volatile("MOVQ $0xC0000082, %%RCX; \
					MOVQ %0, %%RDX; \
					MOVQ %1, %%RAX; \
					WRMSR;"
				   :
				   : "r"(rdx), "r"(rax)
				   : "rax", "rdx", "rcx");


		 /* Clear IF in flags */
	 	__asm volatile("MOV $0xC0000084, %%RCX; \
	 					RDMSR; \
	 					MOV %%RAX, %0; \
	 					MOV %%RDX, %1"
	 				   : "=r"(rax), "=r"(rdx)
	 				   :: "rax", "rdx", "rcx");

	 	printk("Flags MSR %x\n", rax);
	 	rax |= 0x0000000000000200;
	 	printk("Now %x\n", rax);

	 	__asm volatile("MOV $0xC0000084, %%RCX; \
	 					MOV %0, %%RAX; \
	 					MOV %1, %%RDX; \
	 					WRMSR;"
	 				   :: "r"(rax), "r"(rdx)
	 				   : "rax", "rdx", "rcx");


	/* At last, enable SCE in MSR 0xC0000080 */
	__asm volatile("MOV $0xC0000080, %%RCX; \
					RDMSR; \
					MOV %%RAX, %0; \
					MOV %%RDX, %1"
				   : "=r"(rax), "=r"(rdx)
				   :: "rax", "rdx", "rcx");

	printk("EFER MSR %x\n", rax);
	rax |= 0x0000000000000001;
	printk("Now %x\n", rax);

	__asm volatile("MOV $0xC0000080, %%RCX; \
					MOV %0, %%RAX; \
					MOV %1, %%RDX; \
					WRMSR;"
				   :: "r"(rax), "r"(rdx)
				   : "rax", "rdx", "rcx");

	printk("SYSENTER setup complete %x\n", &console_raw_print);
}
