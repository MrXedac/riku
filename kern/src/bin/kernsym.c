#include <string.h>
#include <stdint.h>
#include "kernsym.h"
#include "heap.h"
#include "vga.h"

void init_ksym(uint32_t size)
{
	/* Initialize table for size entries */
	puts("Initializing kernel symbol table for ");
	putdec(size);
	puts(" symbols (size=");
	puthex(sizeof(Riku_Symbol) * size);
	puts("\n");
	symTable = (Riku_Symbol*)kalloc(sizeof(Riku_Symbol) * size);
}