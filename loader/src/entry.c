#include <stdint.h>
#include "vga.h"

/* Early-boot console init */
void init_terminal()
{
	cls();
}

/* Loader entry-point. */
void main(uintptr_t* mboot_info)
{
	/* Initialize some stuff related to early-boot IO */
	init_terminal();

	/* Show we are alive ! */
	puts("Riku Loader - The Riku Operating System\n");
	for(;;);
}
