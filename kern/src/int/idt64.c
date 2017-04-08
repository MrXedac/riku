#include "idt64.h"
#include "ioport.h"
#include "vga.h"
#include "mem.h"
#include "task.h"
#include "serial.h"
#include <stdint.h>

extern void idt_flush(uint64_t idt_ptr);

/* IRQ handlers, better hook your drivers here */
irq_t IRQHANDLERS[16];

idt_entry_t idt_entries[256]; //!< Interrupt Descriptor Table
idt_ptr_t   idt_ptr; //!< Pointer to the IDT

/* Register a hardware interrupt handler */
void register_irq(uint8_t int_no, irq_t handler)
{
	if(int_no >= 0 && int_no < 16)
		IRQHANDLERS[int_no] = handler;

	return;
}

void enable_interrupts()
{
	__asm volatile("sti");
}

void disable_interrupts()
{
	__asm volatile("cli");
}

void irq_handler(registers_t* regs)
{
	/* Clear master (and slave) PIC */
	if (regs->int_no >= 40)
	{
		outb(0xA0, 0x20);
	}

	outb(0x20, 0x20);

	if(IRQHANDLERS[regs->int_no - 0x20]){
		irq_t handler = IRQHANDLERS[regs->int_no - 0x20]; /* IRQs begin at 32 */
		handler(regs);
	} else {
		KTRACE("WARNING: unhandled interrupt %x\n", regs->int_no);
	}
}

void isr_handler(registers_t* regs)
{
	if(regs->int_no==14) {
		puts("page fault : ");
		uint64_t err_code = regs->err_code;
		uint64_t present, rw, supervisor;
		present = err_code & 0x00000001;
		rw = err_code & 0x00000002;
		supervisor = err_code & 0x00000004;
		if(present) {
			puts("page is present, ");
		} else {
			puts("page is missing, ");
		}
		if(rw) {
			puts("tried to write ");
		} else {
			puts("tried to read ");
		}
		if(supervisor) {
			puts("from user land");
		} else {
			puts("from kernel land");
		}

		puts(") at address ");
		uint64_t addr;
		__asm volatile ("mov %%cr2, %0"
						:"=r" (addr)
						);
		puthex(addr);
		puts("\n");
	}
	if(regs->int_no == 254)
		panic("riku emergency halt/debug panic", regs);
	else
		panic("unhandled fault in kernel land", regs);
}


/* Add an interrupt entry into IDT */
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags)
{
	uintptr_t base_hhalf = (uintptr_t)(base | 0xFFFF800000000000); /* Higher kernel memory */
	idt_entries[num].base_lo = base_hhalf & 0xFFFF;
	idt_entries[num].base_mid = (base_hhalf >> 16) & 0xFFFF;
	idt_entries[num].base_high = (base_hhalf >> 32) & 0xFFFFFFFF;

	idt_entries[num].sel     = sel;
	idt_entries[num].always0 = 0x0;
	idt_entries[num].always0too = 0x00000000;

	idt_entries[num].flags   = flags;
}

/* IRQs are interrupts 0-15 on boot. Remap them to 32-47 */
void remap_irq()
{
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

/* Bind IRQ in IDT */
void bind_irq()
{
	idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);
	idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);
	idt_set_gate(34, (uint64_t)irq2, 0x08, 0x8E);
	idt_set_gate(35, (uint64_t)irq3, 0x08, 0x8E);
	idt_set_gate(36, (uint64_t)irq4, 0x08, 0x8E);
	idt_set_gate(37, (uint64_t)irq5, 0x08, 0x8E);
	idt_set_gate(38, (uint64_t)irq6, 0x08, 0x8E);
	idt_set_gate(39, (uint64_t)irq7, 0x08, 0x8E);
	idt_set_gate(40, (uint64_t)irq8, 0x08, 0x8E);
	idt_set_gate(41, (uint64_t)irq9, 0x08, 0x8E);
	idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
	idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
	idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
	idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
	idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
	idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);

	idt_flush((uint64_t)&idt_ptr);
}

/* Bind faults in IDT */
void bind_isr()
{
	idt_set_gate( 0, (uint64_t)isr0 , 0x08, 0xEE);
	idt_set_gate( 1, (uint64_t)isr1 , 0x08, 0xEE);
	idt_set_gate( 2, (uint64_t)isr2 , 0x08, 0xEE);
	idt_set_gate( 3, (uint64_t)isr3 , 0x08, 0xEE);
	idt_set_gate( 4, (uint64_t)isr4 , 0x08, 0xEE);
	idt_set_gate( 5, (uint64_t)isr5 , 0x08, 0xEE);
	idt_set_gate( 6, (uint64_t)isr6 , 0x08, 0xEE);
	idt_set_gate( 7, (uint64_t)isr7 , 0x08, 0xEE);
	idt_set_gate( 8, (uint64_t)isr8 , 0x08, 0xEE);
	idt_set_gate( 9, (uint64_t)isr9 , 0x08, 0xEE);
	idt_set_gate(10, (uint64_t)isr10, 0x08, 0xEE);
	idt_set_gate(11, (uint64_t)isr11, 0x08, 0xEE);
	idt_set_gate(12, (uint64_t)isr12, 0x08, 0xEE);
	idt_set_gate(13, (uint64_t)isr13, 0x08, 0xEE);
	idt_set_gate(14, (uint64_t)isr14, 0x08, 0xEE);
	idt_set_gate(15, (uint64_t)isr15, 0x08, 0xEE);
	idt_set_gate(16, (uint64_t)isr16, 0x08, 0xEE);
	idt_set_gate(17, (uint64_t)isr17, 0x08, 0xEE);
	idt_set_gate(18, (uint64_t)isr18, 0x08, 0xEE);
	idt_set_gate(19, (uint64_t)isr19, 0x08, 0xEE);
	idt_set_gate(20, (uint64_t)isr20, 0x08, 0xEE);
	idt_set_gate(21, (uint64_t)isr21, 0x08, 0xEE);
	idt_set_gate(22, (uint64_t)isr22, 0x08, 0xEE);
	idt_set_gate(23, (uint64_t)isr23, 0x08, 0xEE);
	idt_set_gate(24, (uint64_t)isr24, 0x08, 0xEE);
	idt_set_gate(25, (uint64_t)isr25, 0x08, 0xEE);
	idt_set_gate(26, (uint64_t)isr26, 0x08, 0xEE);
	idt_set_gate(27, (uint64_t)isr27, 0x08, 0xEE);
	idt_set_gate(28, (uint64_t)isr28, 0x08, 0xEE);
	idt_set_gate(29, (uint64_t)isr29, 0x08, 0xEE);
	idt_set_gate(30, (uint64_t)isr30, 0x08, 0xEE);
	idt_set_gate(31, (uint64_t)isr31, 0x08, 0xEE);
	idt_flush((uint64_t)&idt_ptr);
}

void idt_init()
{
	idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
	idt_ptr.base  = (uint64_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
	bind_isr();
	remap_irq();
	bind_irq();
	current_task = 0;
	task_list = 0;
	tasking_ready = 0;
}
