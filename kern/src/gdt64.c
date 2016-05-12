#include "gdt64.h"
#include <stdint.h>

/* Our GDT, with 7 entries, and finally our special GDT pointer */
struct gdt_entry *gdt;
struct gdt_ptr *gp;

extern void gdt_flush(uint32_t gdtptr);

/* Sets up a gate into the Global Descriptor Table */
void gdtInstallGate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

/* Initializes the Global Descriptor Table */
void gdt_init(uintptr_t gdt_addr, uintptr_t gptptr_addr)
{
	gdt  = (struct gdt_entry*)gdt_addr;
	gp = (struct gdt_ptr*)gptptr_addr;

	/* Our GDT has already been setup by RikuLdr, do nothing */
	return;
}
