#ifndef __LDRINFO__
#define __LDRINFO__

#define LDRINFO_ADDR	0x4000

struct rikuldr_info {
	uint32_t mbi_addr;	/* Multiboot header address */
	uint32_t ldr_paddr; /* Loader physical address */
	uint32_t krn_paddr; /* Kernel physical address */
	uint32_t gdt_paddr; /* GDT physical address */
	uint32_t gdtptr_paddr; /* GDT pointer physical address */
};

#endif