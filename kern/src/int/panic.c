#include "idt64.h"
#include "vga.h"
#include "task.h"
#include <stdint.h>

void panic(char* message, registers_t* regs)
{
	puts("Riku panic: ");
	puts(message);
	puts(", registers:\n");
	if(regs)
	{
		puts("int_no=");
		puthex(regs->int_no);
		puts("\n");
		puts("rax=");
		puthex(regs->rax);
		puts("\trbx=");
		puthex(regs->rbx);
		puts("\trcx=");
		puthex(regs->rcx);
		puts("\trdx=");
		puthex(regs->rdx);
		puts("\nrsi=");
		puthex(regs->rsi);
		puts("\trdi=");
		puthex(regs->rdi);
		puts("\trsp=");
		puthex(regs->userrsp);
		puts("\trbp=");
		puthex(regs->rbp);
		puts("\nrip=");
		puthex(regs->rip);
		puts("\tss=");
		puthex(regs->ss);
		puts("\tcs=");
		puthex(regs->cs);
		puts("\tds=");
		puthex(regs->ds);
		puts("\n");
	} else {
		puts("\t<no register data found>\n");
	}

    uintptr_t cr2;
    __asm volatile("MOV %%CR2, %0"
            : "=r" (cr2));
    puts("cr2=");
    puthex(cr2);
    puts("\n");
		puts("errcode=");
		puthex(regs->err_code);
		puts("\n");
		/*
			0xB = 1011
		*/

	puts("current_task: ");
	putdec(current_task->pid);
	puts("\n");
	puts("Halting system.\n");
	__asm volatile("CLI;");
	for(;;);
}
