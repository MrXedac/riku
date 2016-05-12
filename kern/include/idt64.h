#ifndef __IDT64_H__
#define __IDT64_H__

#include <stdint.h>

/* Defines an IDT entry */
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
	uint32_t base_higher;
	uint32_t sbz;
} __attribute__((packed));

/* Defines the IDT pointer */
struct idt_ptr
{
    unsigned short limit;
    uintptr_t base;
} __attribute__((packed));

void panic(char* message, uintptr_t* regs);

#endif