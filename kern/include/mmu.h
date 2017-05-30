#ifndef __MMU_H__
#define __MMU_H__

#include "vm.h"
#include "idt64.h"

#define PF_ERR_PRESENT  0x1
#define PF_ERR_READONLY 0x2
#define PF_ERR_KERN     0x4

uintptr_t *masterTable; /* Kernel master table, at PML4T | PML4T_UPPER */
uintptr_t *current_cr3;
uintptr_t *kernel_cr3;

uint32_t addrIndex(uint32_t level, uintptr_t addr);
void tableWrite(uintptr_t table, uint32_t index, uintptr_t value);
uintptr_t tableRead(uintptr_t table, uint32_t index);

void mmu_init();
void switch_cr3(uintptr_t cr3);
uintptr_t build_new_vme(); /* Builds a new virtual memory environment */
void vme_map(uintptr_t vme, uintptr_t phys, uintptr_t va, uint8_t user); /* Maps a page into a VME */
void vme_right(uintptr_t vme, uintptr_t va, uint8_t user, uint8_t rw); /* Changes access rights of a VME's page */
void vme_unmap(uintptr_t vme, uintptr_t va); /* Removes a page from a VME */
void pmt_inc(uintptr_t phys); /* Increase the page counter for a physical address into the Page Master Table */
void pmt_dec(uintptr_t phys); /* Decrease the page counter for a physical address into the Page Master Table */

void do_pagefault(registers_t* regs);
void set_vme_as_ro(uintptr_t vme);
uintptr_t clone_vme(uintptr_t vme);

#endif
