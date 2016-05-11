#ifndef __VM_H__
#define __VM_H__

#include "multiboot.h"
#include <stdint.h>

/* Table index defines */
#define TABLE_PML4T	1
#define TABLE_PDPT	2
#define TABLE_PD	3
#define TABLE_PT	4

/* Early-boot x64 mmu configuration tables */
#define EARLY_PML4T		0x1000
#define EARLY_PDPT_PHYS	0x2000
#define EARLY_PDPT_KERN	0x3000

/* MMU flags */
#define FLAG_PRESENT		0x1
#define FLAG_RW				0x2
#define	FLAG_LARGE_PAGE		0x80
#define FLAGS_PML4T			(FLAG_PRESENT | FLAG_RW)
#define FLAGS_PDPT_LARGE	(FLAG_PRESENT | FLAG_RW | FLAG_LARGE_PAGE)

/* Special PML4T indexes */
#define PML4T_UPPER			256

/* 4kb pages */
#define PAGE_SIZE	0x1000

/* Early-boot MMU initialization */
void vm_init();

/* Memory map initialization */
void mmap_init(struct multiboot_tag_mmap *mmap_tag_ptr);

/* Pagination */
uintptr_t *first_free_page; //!< First free available page.
uint64_t max_pages;
uint64_t allocated_pages;

uintptr_t* alloc_page();
void free_page(uintptr_t *page);

#endif
