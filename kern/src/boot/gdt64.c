#include "gdt64.h"
#include "vga.h"
#include "vm.h"
#include <stdint.h>

/* Our GDT, with 7 entries, and finally our special GDT pointer */
struct gdt_entry *gdt;
struct gdt_ptr *gp;
tss_entry_t tss_entry; //!< Generic TSS entry for userland-to-kernel switch

extern void gdt_flush(uintptr_t gdtptr);
extern void tss_flush(); //!< ASM method to flush the TSS entry

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
	
	/* Allright, our GDT is not x64-compliant, we have to build it from scratch here... */
	/*gdtInstallGate(0, 0x0, 0x0, 0x0, 0x0);
	gdtInstallGate(1, 0x0, 0x0, 0x9A, 0x20);
	gdtInstallGate(2, 0x0, 0x0, 0x92, 0x00);
	gdtInstallGate(3, 0x0, 0x0, 0xFA, 0x20);
	gdtInstallGate(4, 0x0, 0x0, 0xF2, 0x00);
	
	gp->base = (uintptr_t)gdt;
	gp->limit = sizeof(struct gdt_entry) * 6 - 1;

	gdt_flush((uintptr_t)gp); */
	write_tss(0x5, 0xFFFF800000500000);
	tss_flush();
	
	/* Our GDT has already been setup by RikuLdr, do nothing */
	return;
}

void write_tss(int32_t num, uint64_t rsp0)
{
	uintptr_t base = (uintptr_t)((uintptr_t)&tss_entry);
	uint32_t limit = sizeof(tss_entry);
	
	//gdt_set_gate(num, base, limit, 0xE9, 0x00);
	struct gdt_extended_entry* tss = (struct gdt_extended_entry*)&gdt[num];
	tss->base_low = (base & 0xFFFF);
	tss->base_middle = (base >> 16) & 0xFF;
	tss->base_high = (base >> 24) & 0xFF;
	tss->limit_low = limit;
	tss->access = 0xE9; // Clear the System byte : 16-bytes descriptor
	tss->limit_hi = 0;
	tss->flags = 0;
	tss->base_rlyhigh = ((base & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF;
	tss->sbz = 0x00000000;
	
	//memset(&tss_entry, 0, sizeof(tss_entry));
	
	tss_entry.rsp0 = rsp0;
	
	/*tss_entry.cs = 0x0B;
	 tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;*/
}