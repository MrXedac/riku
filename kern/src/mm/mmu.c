#include "mmu.h"
#include "vm.h"
#include "vga.h"
#include "serial.h"
#include "mem.h"
#include "task.h"
#include <stdint.h>

uintptr_t pmt; /* Page Master Table address */

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

void vme_right(uintptr_t vmet, uintptr_t va, uint8_t user, uint8_t rw)
{
	uintptr_t flags = 0x0;
	if(rw && user)
		flags = FLAGS_PML4T | FLAG_USER;
	else if(rw)
		flags = FLAGS_PML4T;
	else
		flags = FLAG_PRESENT;

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
	uintptr_t vad = tableRead(pt, pt_idx);
	tableWriteWithFlags(pt, pt_idx, (uintptr_t)vad | flags);
	printk("vme: updated flags of page %x to %x\n", vad, flags);
}

void vme_map(uintptr_t vmet, uintptr_t phys, uintptr_t va, uint8_t user)
{
	printk("vme: map %x to %x\n", phys, va);
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
	pmt_inc(phys);
}

void vme_unmap(uintptr_t vmet, uintptr_t va)
{
	/* First get physical address of faulty page */

	uintptr_t vme = PHYS(vmet);

	/* Get PDPT */
	uintptr_t pml4t_idx = addrIndex(4, va);
	uintptr_t pdpt = tableRead(vme, pml4t_idx);
	if(!pdpt)
	{
		return;
	}

	/* Same thing with PD */
	uintptr_t pdpt_idx = addrIndex(3, va);
	uintptr_t pd = tableRead(pdpt, pdpt_idx);
	if(!pd)
	{
		return;
	}

	/* Same thing with PT */
	uintptr_t pd_idx = addrIndex(2, va);
	uintptr_t pt = tableRead(pd, pd_idx);
	if(!pt)
	{
		return;
	}

	/* Map page into PT */
	uintptr_t pt_idx = addrIndex(1, va);
	uintptr_t phys = tableRead(pt, pt_idx);
	tableWriteWithFlags(pt, pt_idx, 0x0);

	/* Decrease the page reference count */
	pmt_dec(phys);
	printk("unmapped page %x\n", va);
}

/* Increase the page counter for a physical address into the Page Master Table */
void pmt_inc(uintptr_t phys)
{
	uintptr_t ppmt = PHYS(pmt);

	/* Get PDPT */
	uintptr_t pml4t_idx = addrIndex(4, phys);
	uintptr_t pdpt = tableRead(ppmt, pml4t_idx);
	if(!pdpt)
	{
		pdpt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pdpt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(ppmt, pml4t_idx, (uintptr_t)pdpt);
	}

	/* Same thing with PD */
	uintptr_t pdpt_idx = addrIndex(3, phys);
	uintptr_t pd = tableRead(pdpt, pdpt_idx);
	if(!pd)
	{
		pd = (uintptr_t)alloc_page();
		memset((void*)PHYS(pd), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pdpt, pdpt_idx, (uintptr_t)pd);
	}

	/* Same thing with PT */
	uintptr_t pd_idx = addrIndex(2, phys);
	uintptr_t pt = tableRead(pd, pd_idx);
	if(!pt)
	{
		pt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pd, pd_idx, (uintptr_t)pt);
	}

	/* Map page into PT */
	uintptr_t pt_idx = addrIndex(1, phys);
	uintptr_t count = tableReadWithFlags(pt, pt_idx);
	count++;
	tableWriteWithFlags(pt, pt_idx, (uintptr_t)count);
	printk("PMT: increased counter of page %x to %d\n", phys, count);
}

/* Decrease the page counter for a physical address into the Page Master Table */
void pmt_dec(uintptr_t phys)
{
	uintptr_t ppmt = PHYS(pmt);

	/* Get PDPT */
	uintptr_t pml4t_idx = addrIndex(4, phys);
	uintptr_t pdpt = tableRead(ppmt, pml4t_idx);
	if(!pdpt)
	{
		pdpt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pdpt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(ppmt, pml4t_idx, (uintptr_t)pdpt);
	}

	/* Same thing with PD */
	uintptr_t pdpt_idx = addrIndex(3, phys);
	uintptr_t pd = tableRead(pdpt, pdpt_idx);
	if(!pd)
	{
		pd = (uintptr_t)alloc_page();
		memset((void*)PHYS(pd), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pdpt, pdpt_idx, (uintptr_t)pd);
	}

	/* Same thing with PT */
	uintptr_t pd_idx = addrIndex(2, phys);
	uintptr_t pt = tableRead(pd, pd_idx);
	if(!pt)
	{
		pt = (uintptr_t)alloc_page();
		memset((void*)PHYS(pt), 0x0, PAGE_SIZE);
		tableWriteWithFlags(pd, pd_idx, (uintptr_t)pt);
	}

	/* Map page into PT */
	uintptr_t pt_idx = addrIndex(1, phys);
	uintptr_t count = tableReadWithFlags(pt, pt_idx);
	if(count == 0) {
		printk("PMT: counter already 0\n");
		return;
	}

	count--;
	tableWriteWithFlags(pt, pt_idx, (uintptr_t)count);
	printk("PMT: decreased counter of page %x to %d\n", phys, count);
	if(count == 0)
	{
		printk("PMT: counter fell to zero, freeing page\n");
		free_page((uintptr_t*)(PHYS(phys)));
	}
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
	pmt = (uintptr_t)alloc_page();
	memset((void*)pmt, 0x0, PAGE_SIZE);
	printk("mmu: page master table at %x\n", pmt);
}

void copy_and_remap_page(uintptr_t fault_addr)
{
	printk("copy-on-write request from task %d\n", current_task->pid);
	/* First get physical address of faulty page */

	uintptr_t vme = PHYS((uintptr_t)current_task->vm_root);

	/* Get PDPT */
	uintptr_t pml4t_idx = addrIndex(4, fault_addr);
	uintptr_t pdpt = tableRead(vme, pml4t_idx);
	if(!pdpt)
	{
		printk("BUG: page seemed to be mapped, but CR3 is inconsistent\n");
		return;
	}

	/* Same thing with PD */
	uintptr_t pdpt_idx = addrIndex(3, fault_addr);
	uintptr_t pd = tableRead(pdpt, pdpt_idx);
	if(!pd)
	{
		printk("BUG: page seemed to be mapped, but CR3 is inconsistent\n");
		return;
	}

	/* Same thing with PT */
	uintptr_t pd_idx = addrIndex(2, fault_addr);
	uintptr_t pt = tableRead(pd, pd_idx);
	if(!pt)
	{
		printk("BUG: page seemed to be mapped, but CR3 is inconsistent\n");
		return;
	}

	/* Map page into PT */
	uintptr_t pt_idx = addrIndex(1, fault_addr);
	uintptr_t phys = tableRead(pt, pt_idx);

	/* Now that we have both physical and virtual address, copy the page into a fresh new one */
	uintptr_t target = ((uintptr_t)alloc_page()) & 0x00007FFFFFFFFFFF; /* Ignore higher-half stuff for mapping purposes */
	memcpy((void*)PHYS(target), (void*)PHYS(phys), PAGE_SIZE);
	printk("mmu: copied into %x from %x\n", PHYS(target), PHYS(phys));

	/* Page has been copied. Remove the mapping in the current CR3, and remap stuff in a read-write fashion */
	vme_unmap((uintptr_t)current_task->vm_root, fault_addr);
	vme_map((uintptr_t)current_task->vm_root, target, fault_addr, 0x1);

	printk("mmu: remapped page %x (phys %x) to new phys %x\n", fault_addr, phys, target);

	__asm volatile("MOV %0, %%RAX; MOV %%RAX, %%CR3" :: "r"(current_task->vm_root));
	/* It's all good ! */
	return;
}

void do_pagefault(registers_t* regs)
{
	/* Get faulty address */
	uintptr_t cr2;
	__asm volatile("MOV %%CR2, %0" : "=r"(cr2));

	/* Choose whether the fault is legitimate or not */
	if((regs->err_code & PF_ERR_PRESENT) && (regs->err_code & PF_ERR_READONLY))
	{
		/* Perform copy-and-write operation */
		copy_and_remap_page(cr2);
		return;
	} else {
		printk("Unhandled page fault in task %d at %x flags %x rip %x rsp %x\n", current_task->pid, cr2, regs->err_code, regs->rip, regs->rsp);
		uint64_t err_code = regs->err_code;
		uint64_t present, rw, supervisor;
		present = err_code & 0x00000001;
		rw = err_code & 0x00000002;
		supervisor = err_code & 0x00000004;
		printk("page is %s, tried to %s from %s land\n", 
			present?"present":"missing",
			rw?"write":"read",
			supervisor?"user":"kernel");
		for(;;);
	}
}

/* Sets a whole VME as read-only */
void set_vme_as_ro(uintptr_t vme)
{
	/* Level 4 table */
	for(uintptr_t pml4t_idx = 0; pml4t_idx < PML4T_UPPER; pml4t_idx++)
	{
		uintptr_t pdpt_addr = tableRead(vme, pml4t_idx);
		if(pdpt_addr)
		{
			/* Level 3 table */
			for(uintptr_t pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++)
			{
				uintptr_t pd_addr = tableRead(pdpt_addr, pdpt_idx);
				if(pd_addr)
				{
					/* Level 2 table */
					for(uintptr_t pd_idx = 0; pd_idx < 512; pd_idx++)
					{
						uintptr_t pt_addr = tableRead(pd_addr, pd_idx);
						/* Level 1 table */
						if(pt_addr)
						{
								for(uintptr_t pt_idx = 0; pt_idx < 512; pt_idx++)
								{
									uintptr_t phys = tableRead(pt_addr, pt_idx);

									/* We have a page mapped here */
									if(phys) {
										tableWriteWithFlags(pt_addr, pt_idx, phys | FLAG_PRESENT | FLAG_USER);
										printk("physical page %x:%x written as ro (entry %x)\n", vme, phys, phys | FLAG_PRESENT | FLAG_USER);

									}
								}
						}
					}
				}
			}
		}
	}

}

/* Creates a new VME using an existing one */
uintptr_t clone_vme(uintptr_t vme)
{
	uintptr_t new_vme = (uintptr_t)alloc_page();
	memset((void*)(PHYS(new_vme)), 0x0, PAGE_SIZE);
	printk("new vme at %x\n", new_vme);

	tableWriteWithFlags(new_vme, PML4T_UPPER, (uintptr_t)masterTable | FLAGS_PML4T);
	/* Level 4 table */
	for(uintptr_t pml4t_idx = 0; pml4t_idx < PML4T_UPPER; pml4t_idx++)
	{
		uintptr_t pdpt_entry = tableReadWithFlags(PHYS(vme), pml4t_idx);
		uintptr_t pdpt_addr = pdpt_entry & 0xFFFFFFFFFFFFF000;
		uintptr_t pdpt_flags = pdpt_entry & 0x0000000000000FFF;
		if(pdpt_addr)
		{
			printk("-> Found PDPT to clone at index %x, entry %x, addr %x\n", pml4t_idx, pdpt_entry, pdpt_addr);
			/* Create new PDPT in new VME using same flags */
			uintptr_t new_pdpt = (uintptr_t)alloc_page();
			memset((void*)PHYS(new_pdpt), 0x0, PAGE_SIZE);
			tableWriteWithFlags(new_vme, pml4t_idx, new_pdpt | pdpt_flags);
			/* Level 3 table */
			for(uintptr_t pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++)
			{
				uintptr_t pd_entry = tableReadWithFlags(pdpt_addr, pdpt_idx);
				uintptr_t pd_addr = pd_entry & 0xFFFFFFFFFFFFF000;
				uintptr_t pd_flags = pd_entry & 0x0000000000000FFF;
				if(pd_addr)
				{
					printk("\t-> Found PD to clone at index %x, addr %x\n", pdpt_idx, pd_addr);
					/* Create new PD in new VME using same flags */
					uintptr_t new_pd = (uintptr_t)alloc_page();
					memset((void*)PHYS(new_pd), 0x0, PAGE_SIZE);
					tableWriteWithFlags(new_pdpt, pdpt_idx, new_pd | pd_flags);

					/* Level 2 table */
					for(uintptr_t pd_idx = 0; pd_idx < 512; pd_idx++)
					{
						uintptr_t pt_entry = tableReadWithFlags(PHYS(pd_addr), pd_idx);
						uintptr_t pt_addr = pt_entry & 0xFFFFFFFFFFFFF000;
						uintptr_t pt_flags = pt_entry & 0x0000000000000FFF;
						/* Level 1 table */
						if(pt_addr)
						{
								printk("\t\t-> Found PT to clone at %x\n", pt_addr);
								/* Create new PT in new VME using same flags */
								uintptr_t new_pt = (uintptr_t)alloc_page();
								memset((void*)PHYS(new_pt), 0x0, PAGE_SIZE);
								tableWriteWithFlags(new_pd, pd_idx, new_pt | pt_flags);

								for(uintptr_t pt_idx = 0; pt_idx < 512; pt_idx++)
								{
									uintptr_t phys = tableReadWithFlags(PHYS(pt_addr), pt_idx);

									/* We have a page mapped here, insert here, duplicating flags */
									if(phys) {
										tableWriteWithFlags(new_pt, pt_idx, phys);
										pmt_inc(phys & 0xFFFFFFFFFFFFF000); /* Increase Page Master Table count for page */
									}
								}
						}
					}
				}
			}
		}
	}

	printk("mmu: cloned vme %x to new vme %x\n", vme, new_vme);
	return new_vme;
}
