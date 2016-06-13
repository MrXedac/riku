#ifndef __IDT64_H__
#define __IDT64_H__

#include <stdint.h>

/* Defines an IDT entry */
struct idt_entry_struct
{
	uint16_t base_lo; //!< Lower bytes of handler address
	uint16_t sel; //!< Selector
	uint8_t  always0; //!< ?
	uint8_t  flags; //!< Interrupt handler flags (Required Privilege Level etc)
	uint16_t base_mid; //!< Middle bytes of handler address
	uint32_t base_high; //!< Higher bytes of handler address
	uint32_t always0too; //!< Always zero aswell
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

/* IDT Pointer */
struct idt_ptr_struct
{
	uint16_t limit; //!< Address limit
	uint64_t base; //!< IDT pointer base
} __attribute__((packed));

typedef struct registers
{
	uint64_t ds; //!< Data segment
	uint64_t rdi; //!< General register EDI
	uint64_t rsi; //!< General register ESI
	uint64_t rbp; //!< EBP
	uint64_t rsp; //!< Stack pointer
	uint64_t rbx; //!< General register EBX
	uint64_t rdx; //!< General register EDX
	uint64_t rcx; //!< General register ECX
	uint64_t rax; //!< General register EAX
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t int_no; //!< Interrupt number
	uint64_t err_code; //!< Interrupt error code
	uint64_t rip; //!< Execution pointer
	uint64_t cs; //!< Code segment
	uint64_t eflags; //!< CPU flags
	uint64_t userrsp; //!< User-mode ESP
	uint64_t ss; //!< Stack segment
} registers_t;

typedef struct idt_ptr_struct idt_ptr_t;

/* Defines the assembly handler for generic interrupts */
extern void isr0 (); //!< ISR 0
extern void isr1 (); //!< ISR 1
extern void isr2 (); //!< ISR 2
extern void isr3 (); //!< ISR 3
extern void isr4 (); //!< ISR 4
extern void isr5 (); //!< ISR 5
extern void isr6 (); //!< ISR 6
extern void isr7 (); //!< ISR 7
extern void isr8 (); //!< ISR 8
extern void isr9 (); //!< ISR 9
extern void isr10(); //!< ISR 10
extern void isr11(); //!< ISR 11
extern void isr12(); //!< ISR 12
extern void isr13(); //!< ISR 13
extern void isr14(); //!< ISR 14
extern void isr15(); //!< ISR 15
extern void isr16(); //!< ISR 16
extern void isr17(); //!< ISR 17
extern void isr18(); //!< ISR 18
extern void isr19(); //!< ISR 19
extern void isr20(); //!< ISR 20
extern void isr21(); //!< ISR 21
extern void isr22(); //!< ISR 22
extern void isr23(); //!< ISR 23
extern void isr24(); //!< ISR 24
extern void isr25(); //!< ISR 25
extern void isr26(); //!< ISR 26
extern void isr27(); //!< ISR 27
extern void isr28(); //!< ISR 28
extern void isr29(); //!< ISR 29
extern void isr30(); //!< ISR 30
extern void isr31(); //!< ISR 31
extern void irq0(); //!< IRQ 0
extern void irq1(); //!< IRQ 1
extern void irq2(); //!< IRQ 2
extern void irq3(); //!< IRQ 3
extern void irq4(); //!< IRQ 4
extern void irq5(); //!< IRQ 5
extern void irq6(); //!< IRQ 6
extern void irq7(); //!< IRQ 7
extern void irq8(); //!< IRQ 8
extern void irq9(); //!< IRQ 9
extern void irq10(); //!< IRQ 10
extern void irq11(); //!< IRQ 11
extern void irq12(); //!< IRQ 12
extern void irq13(); //!< IRQ 13
extern void irq14(); //!< IRQ 14
extern void irq15(); //!< IRQ 15
extern void isr254(); //!< ISR 255 : Riku emergency halt/debug panic

typedef void (*isr_t)(registers_t); //!< Definition of interrupt handler

void panic(char* message, registers_t* regs);
void idt_init();

#endif