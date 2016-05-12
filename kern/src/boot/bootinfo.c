#include "bootinfo.h"
#include "multiboot.h"
#include "vga.h"
#include "vm.h"

/* Parse the multiboot header to find some relevant data */
void parse_mbi(uintptr_t mbi)
{
	struct multiboot_header* header = (struct multiboot_header*)mbi;
	struct multiboot_tag *tag;
	uint64_t addr = (uint64_t)header;
	uint64_t size;
	size = *(unsigned *) header;
	puts("\tAnnounced header size ");
	puthex(size);
	puts("\n");
	for (tag = (struct multiboot_tag *) (addr + 8);
		 tag->type != MULTIBOOT_TAG_TYPE_END;
		 tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
										 + ((tag->size + 7) & ~7)))
	{
		switch (tag->type)
		{
			case MULTIBOOT_TAG_TYPE_CMDLINE:
				puts("\t\tBoot command line = ");
				puts(((struct multiboot_tag_string *) tag)->string);
				puts("\n");
				break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
				puts("\t\tBoot loader = ");
				puts(((struct multiboot_tag_string *) tag)->string);
				puts("\n");
				break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
				puts("\t\tLegacy: lower memory ");
				putdec(((struct multiboot_tag_basic_meminfo *) tag)->mem_lower);
				puts("kio, upper memory ");
				putdec(((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
				puts("kio\n");
				break;
			case MULTIBOOT_TAG_TYPE_MMAP:
			{
				puts("\tFound memory map, initializing\n");
				//dump_mmap((struct multiboot_tag_mmap *) tag);
				mmap_init((struct multiboot_tag_mmap *) tag);
				break;
			}
			default:
			{
				break;
			}
		}
	}
	tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
									+ ((tag->size + 7) & ~7));
	puts("\tTotal multiboot2 info size : ");
	puthex((uint64_t)tag - addr);
	puts("\n");
}