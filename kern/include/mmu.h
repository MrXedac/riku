#ifndef __MMU_H__
#define __MMU_H__

#include "vm.h"

uintptr_t *masterTable; /* Kernel master table, at PML4T | PML4T_UPPER */
uintptr_t *current_cr3;
uintptr_t *kernel_cr3;

uint32_t addrIndex(uint32_t level, uintptr_t addr);
void tableWrite(uintptr_t table, uint32_t index, uintptr_t value);
uintptr_t tableRead(uintptr_t table, uint32_t index);

void mmu_init();
void switch_cr3(uintptr_t cr3);
#endif