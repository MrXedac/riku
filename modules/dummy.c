/* Include a kernel header */
#include "vga.h"

char* module_name()
{
	return "dummy";
}

char* module_description()
{
	return "A dummy kernel module for Riku.";
}
/**
 * This is a dummy kernel module for Riku.
 * This is crap.
 */

void kputs(char* a)
{
	puts("\t[dummy] ");
	puts(a);
}

void module_init()
{
    kputs("This is the dummy kernel module speaking!\n");
    return;
}

