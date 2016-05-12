#ifndef __GDT64_H__
#define __GDT64_H__

#include <stdint.h>

#define GDT_ENTRIES 6

/* Defines a GDT entry */
struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* Defines a GDT extended entry (ie. TSS) */
struct gdt_extended_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
	uint32_t base_higher;
	uint32_t reserved;
} __attribute__((packed));

/* Defines a pointer to the GDT */
struct gdt_ptr
{
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed));

void gdt_init(uintptr_t gdt_addr, uintptr_t gptptr_addr);

#endif