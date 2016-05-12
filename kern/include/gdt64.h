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
	uint8_t limit_hi : 4;
	uint8_t flags : 4;
    uint8_t base_high;
	uint32_t base_rlyhigh;
	uint32_t sbz;
} __attribute__((packed));

/* Defines a pointer to the GDT */
struct gdt_ptr
{
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed));

/* Defines a Task State Segment */
struct tss_entry_struct {
	uint32_t reserved;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved3;
	uint16_t reserved4;
	uint16_t iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t; //!< TSS entry for kernel-mode switch

void gdt_init(uintptr_t gdt_addr, uintptr_t gptptr_addr);
void write_tss(int32_t num, uint64_t rsp0);

#endif