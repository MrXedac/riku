#include "mmu.h"
#include "vm.h"
#include "vga.h"
#include "serial.h"
#include "mem.h"
#include <stdint.h>

uint32_t addrIndex(uint32_t level, uintptr_t addr)
{
	/* First check the index value */
	if(level > 4 || level < 1)
		return 0;

	uintptr_t pml4t_idx =	((addr & 0x0000FF8000000000) >> 39) & 0x1FF; // 9 higher bytes of 48-bits address
	uintptr_t pdpt_idx =	((addr & 0x0000007FC0000000) >> 30) & 0x1FF; // 9 middle bytes of 48-bits address
	uintptr_t pd_idx =		((addr & 0x000000003FE00000) >> 21) & 0x1FF; // 9 lower bytes of 48-bits address
	uintptr_t pt_idx =		((addr & 0x00000000001FF000) >> 12) & 0x1FF; // 9 really lower bytes of 48-bits address

	uint64_t idx = 0;
	switch(level)
	{
		case 4:
			idx = ((uint64_t)addr & 0x0000FF8000000000) >> 39;
			break;
		case 3:
			idx = ((uint64_t)addr & 0x0000007FC0000000) >> 30;
			break;
		case 2:
			idx = ((uint64_t)addr & 0x000000003FE00000) >> 21;
			break;
		case 1:
			idx = ((uint64_t)addr & 0x00000000001FF000) >> 12;
			break;
		default:
			idx = 0x0;
	}

	return (uint32_t)(idx & 0xFFFFFFFF);
}

void tableWrite(uintptr_t table, uint32_t index, uintptr_t value)
{
	uintptr_t tableAddr = table | 0xFFFF800000000000; /* Always read in the physical memory section */
	tableAddr |= ((uintptr_t)index * sizeof(uint64_t));

	uintptr_t flags = *(uintptr_t*)tableAddr & 0x0000000000000FFF;
	*(uintptr_t*)tableAddr = value | flags;

}

void tableWriteWithFlags(uintptr_t table, uint32_t index, uintptr_t value)
{
	uintptr_t tableAddr = table | 0xFFFF800000000000; /* Always read in the physical memory section */
	tableAddr |= ((uintptr_t)index * sizeof(uint64_t));

	*(uintptr_t*)tableAddr = value;

}

uintptr_t tableRead(uintptr_t table, uint32_t index)
{
	uintptr_t tableAddr = table | 0xFFFF800000000000; /* Always read in the physical memory section */
	tableAddr |= ((uintptr_t)index * sizeof(uint64_t));

	return *(uintptr_t*)tableAddr & 0xFFFFFFFFFFFFF000;
}

uintptr_t tableReadWithFlags(uintptr_t table, uint32_t index)
{
	uintptr_t tableAddr = table | 0xFFFF800000000000; /* Always read in the physical memory section */
	tableAddr |= ((uintptr_t)index * sizeof(uint64_t));

	return *(uintptr_t*)tableAddr;
}

void switch_cr3(uintptr_t cr3)
{
	__asm volatile("MOV %0, %%CR3"
				   :
				   : "r" (cr3));

	return;
}

/* Builds a new Virtual Memory Environment with the following layout :
 * 0x0 								-> 0xFFFF800000000000 : available
 * 0xFFFFFFFF80000000 -> 0xFFFFFFFFFFFFFFFF : kernel
 */
uintptr_t build_new_vme()
{
	/* Allocate a new PML4T */
	uintptr_t pml4t = (uintptr_t)alloc_page();

	/* Clear new PML4T */
	memset((void*)pml4t, 0x0, PAGE_SIZE);

	/* Put the kernel in the appropriate areas */
	tableWriteWithFlags(pml4t, PML4T_UPPER, (uintptr_t)masterTable | FLAGS_PML4T);

	/* Return our fresh, new pml4t */
	return pml4t;
}

void vme_map(uintptr_t vmet, uintptr_t phys, uintptr_t va, uint8_t user)
{
	uintptr_t flags;
	if(user)
		flags = FLAGS_PML4T | FLAG_USER;
	else
		flags = FLAGS_PML4T;

	uintptr_t vme = PHYS(vmet);

	/* Get PDPT */
	uintptr_t pml4t_idx = addrIndex(4, va);
	uintptr_t pdpt = tableRead(vme, pml4t_idx);
	if(!pdpt)
	{
		pdpt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pdpt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(vme, pml4t_idx, (uintptr_t)pdpt | flags);
	}

	/* Same thing with PD */
	uintptr_t pdpt_idx = addrIndex(3, va);
	uintptr_t pd = tableRead(pdpt, pdpt_idx);
	if(!pd)
	{
		pd = (uintptr_t)alloc_page();
		memset((void*)PHYS(pd), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pdpt, pdpt_idx, (uintptr_t)pd | flags);
	}

	/* Same thing with PT */
	uintptr_t pd_idx = addrIndex(2, va);
	uintptr_t pt = tableRead(pd, pd_idx);
	if(!pt)
	{
		pt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pd, pd_idx, (uintptr_t)pt | flags);
	}

	/* Map page into PT */
	uintptr_t pt_idx = addrIndex(1, va);
	tableWriteWithFlags(pt, pt_idx, (uintptr_t)phys | flags);
}

void vme_unmap(uintptr_t vme, uintptr_t va)
{

}

void mmu_init()
{
	/* Prepare kernel Master Table */
	masterTable = alloc_page(); /* Allocate a new, fresh PDPT for kernel */
	uint64_t gbOffset = 0x40000000; // Address offset corresponding to one PDPT entry
	uint64_t i = 0;

	// Fill-in every entry of the PDPT with 1-1 mapping of the address space
	for(i = 0; i < 512; i++)
		tableWriteWithFlags((uintptr_t)masterTable, i, (uintptr_t)(i * gbOffset) | FLAGS_PDPT_LARGE);

	/* Prepare kernel task pml4t */
	kernel_cr3 = alloc_page();
	current_cr3 = kernel_cr3;

	tableWriteWithFlags((uintptr_t)kernel_cr3, PML4T_UPPER, (uintptr_t)masterTable | FLAGS_PML4T);
	tableWriteWithFlags((uintptr_t)kernel_cr3, 0, (uintptr_t)masterTable | FLAGS_PML4T);
	printk("mmu: kernel cr3 %x, master table %x\n", kernel_cr3, masterTable);
	/* Kernel PML4T should be ready now. Switch to it. */
	switch_cr3((uintptr_t)kernel_cr3);

}
