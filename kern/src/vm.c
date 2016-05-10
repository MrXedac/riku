#include "vm.h"
#include <stdint.h>

void vm_init()
{
	uint64_t* pdpt = (uint64_t*)0x3000; // PDPT pointer
	uint64_t* pdptk = (uint64_t*)0x2000; // Kernel PDPT
	
	uint64_t gbOffset = 0x40000000; // Address offset corresponding to one PDPT entry
	uint64_t i = 0;
	
	// Fill-in every entry of the PDPT with 1-1 mapping of the address space
	for(i = 0; i < 512; i++)
	{
		*(pdpt) = (i * gbOffset) | 0x83; // Target address + large page flag
		*(pdptk) = (i * gbOffset) | 0x83;
		pdpt = (uint64_t*)((uint64_t)pdpt + 0x8); // Next entry
		pdptk = (uint64_t*)((uint64_t)pdptk + 0x8);
	}
	
}