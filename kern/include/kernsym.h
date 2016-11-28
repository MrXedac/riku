#ifndef __KSYM__
#define __KSYM__

/** 
 * Kernel symbol definitions for module loading
 */
 
struct Riku_KSym {
	char name[64]; /* Symbol name */
	uintptr_t addr; /* Symbol address */
};

typedef struct Riku_KSym Riku_Symbol;

Riku_Symbol* symTable;

void init_ksym(uint32_t size);

#endif