#include "idt64.h"
#include "vga.h"
#include <stdint.h>

void panic(char* message, uintptr_t* regs)
{
	puts("Riku panic: ");
	puts(message);
	puts(", registers:");
	if(regs)
	{
		puts("...\n");
	} else {
		puts("<no register data found>\n");
	}
	
	puts("Halting system.\n");
	__asm volatile("CLI;");
	for(;;);
}