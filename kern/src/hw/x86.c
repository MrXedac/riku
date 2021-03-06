#include <stdint.h>
#include <string.h>
#include "hw.h"
#include "vga.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"

struct riku_devfs_node* hardware_create_node(const char* name)
{
	struct riku_devfs_node* ret = (struct riku_devfs_node*)kalloc(sizeof(struct riku_devfs_node)); /* Allocate node in heap */
	memset(ret, 0x0, sizeof(struct riku_devfs_node)); /* Nullify node */
	strcpy(ret->name, name); /* Write name */
	ret->next = 0x0;

	return ret;
}

void hardware_add_resource(struct riku_devfs_node* node, enum riku_resource_type type, uintptr_t begin, uintptr_t size)
{
	uint8_t i = 0;
	struct riku_devfs_resource* res = 0x0;
	while(i < 6 && res == 0x0)
	{
		if(node->resources[i].begin == 0x0)
			res = &(node->resources[i]);

		i++;
	}

	if(res == 0x0)
	{
		printk("hardware_add_resource: couldn't find an available slot in device descriptor\n");
		return;
	}

	res->type = type;
	res->begin = begin;
	res->size = size;

	printk("added resource range %x length %x type %s for device devfs:/%s\n", res->begin, res->size, res->type==PORTIO?"ioport":"mmio", node->name);

}

void probe_hardware()
{
	printk("Beginning hardware probe.\n");
	puts("Now beginning hardware probe.\n");

	/* Create some space for nullDev. In driver/special/nulldev.c */
	extern void nulldev_init();
	nulldev_init();

	/* TODO : Once we have a working virtual filesystem
	 * (and I should implement this rather quickly, eh),
	 * we should create a node for each device,
	 * e.g. /dev/vgaterm for 0xB8000 VGA terminal, or
	 * /dev/kbd for interrupt 1 keyboard input.
	 * Right now we don't have a VFS, so this is a
	 * big placeholder. */
	struct riku_devfs_node* kbd = hardware_create_node("kbd");
	devfs_add(kbd);

	puts("Probing PCI.\n");
	probe_pci();

	/*printk("Probed devices: ");
	struct riku_devfs_node* devnode = nullDev;
	while(devnode != 0x0)
	{
		printk("devfs:/%s\t", devnode->name);
		devnode = devnode->next;
	}
	printk("\n");*/
}
