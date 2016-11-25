/* Include a kernel header */
#include "vga.h"

/**
 * This is a dummy kernel module for Riku.
 * This is crap.
 */

void module_init()
{
    puts("This is the dummy kernel module speaking!\n");
    return;
}

