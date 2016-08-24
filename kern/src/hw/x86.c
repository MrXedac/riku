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
		KTRACE("hardware_add_resource: couldn't find an available slot in device descriptor\n");
		return;
	}
	
	res->type = type;
	res->begin = begin;
	res->size = size;
	
	KTRACE("added resource range for device /dev/");
	KTRACERAW(node->name);
	KTRACERAW("\n");
}

void probe_pci()
{
	/* TODO : Probe PCI devices here */
	KTRACE("probe_pci: not implemented\n");
	return;
}

void probe_hardware()
{
	KTRACE("Beginning hardware probe.\n");
	puts("Now beginning hardware probe.\n");
	
	/* Create some space for nullDev. */
	nullDev = (struct riku_devfs_node*)kalloc(sizeof(struct riku_devfs_node));
	strcpy(nullDev->name, "null");
	nullDev->next = 0;
	
	/* TODO : Once we have a working virtual filesystem
	 * (and I should implement this rather quickly, eh),
	 * we should create a node for each device,
	 * e.g. /dev/vgaterm for 0xB8000 VGA terminal, or
	 * /dev/kbd for interrupt 1 keyboard input.
	 * Right now we don't have a VFS, so this is a
	 * big placeholder. */
	struct riku_devfs_node* vgat = hardware_create_node("vgaterm");
	hardware_add_resource(vgat, MMIO, 0xB8000, 0x4000);
	devfs_add(vgat);
	
	struct riku_devfs_node* kbd = hardware_create_node("kbd");
	devfs_add(kbd);
	
	puts("Probing PCI.\n");
	probe_pci();
	
	KTRACE("Probed devices: ");
	struct riku_devfs_node* devnode = nullDev;
	while(devnode != 0x0)
	{
		KTRACERAW("/dev/");
		KTRACERAW(devnode->name);
		KTRACERAW("\t");
		devnode = devnode->next;
	}
	KTRACERAW("\n");
}