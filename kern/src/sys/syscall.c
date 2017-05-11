#include <stdint.h>
#include "sys.h"
#include "idt64.h"
#include "printk.h"
#include "serial.h"

void syscall_entry();

static void *syscall[SYSCALL_COUNT] =
{
	&slputs,
};

void sysenter_entry()
{
	/* Before doing anything else, store our system call arguments somewhere safe with interrupts disabled */
	uint64_t sysc_id, ret_addr;
	__asm volatile("CLI;");
	__asm volatile("MOV %%RAX, %0"
				   : "=r"(sysc_id));
	__asm volatile("MOV %%RCX, %0"
				   : "=r"(ret_addr));
	
	/* At this point, we should switch to the kernel stack (or keep using the user stack ? Good question here */
	
	printk("SYSENTER called.\n");
	
	/* Get system call id */
	if(sysc_id > SYSCALL_COUNT)
	{
		printk("Invalid system call %d.\n", sysc_id);
		goto sysret;
	}
	
	
sysret:
	/* If we switched to a kernel stack, we should restore the user stack here, and push return RIP into RCX */
	
	/* Return to userland/saved context */
	__asm volatile("STI; SYSRET;");
}

void syscall_entry(uint64_t sysc_id)
{
	/* Get system call location */
	void *location = syscall[sysc_id];
	
	/* Pass arguments to system call and run call */
	uint32_t ret;
	/*__asm volatile("MOVQ %0, %%RDI; \
					MOVQ %1, %%RSI; \
					MOVQ %2, %%RDX; \
					MOVQ %3, %%RCX; \
					MOVQ %4, %%R8; \
					MOVQ %5, %%R9; \
					CALL *%6;"
				   : "=a"(ret)
				   : "r"(regs->rdi), "r"(regs->rsi), "r"(regs->rdx), "r"(regs->rcx), "r"(regs->rbx), "r"(location));
	
	regs->rax = ret;*/
	return;
}

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
				   MOV $0x0000000000500000, %RAX; \
				   WRMSR;");
	
	/* Finally setup SYSENTER RIP */
	uint64_t rax, rdx;
	rdx = ((uint64_t)(&sysenter_entry) >> 32) & 0x00000000FFFFFFFF;
	rax = (uint64_t)(&sysenter_entry) & 0x00000000FFFFFFFF;
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
				   MOV $0x001B0008, %RDX; \
				   MOV $0xCAFEBABE, %RAX; \
				   WRMSR;");
	
	
	__asm volatile("MOVQ $0xC0000082, %%RCX; \
					MOVQ %0, %%RDX; \
					MOVQ %1, %%RAX; \
					WRMSR;"
				   :
				   : "r"(rdx), "r"(rax)
				   : "rax", "rdx", "rcx");
	
	__asm volatile("MOVQ $0xC0000083, %%RCX; \
				   MOVQ %0, %%RDX; \
				   MOVQ %1, %%RAX; \
				   WRMSR;"
				   :
				   : "r"(rdx), "r"(rax)
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
	
	printk("SYSENTER setup complete\n");
}
