#include <stdint.h>
#include <string.h>

#include "serial.h"
#include "vga.h"
#include "pci.h"
#include "ioport.h"
#include "mem.h"
#include "hw.h"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;
	
	address = (uint32_t)((lbus << 16) | (lslot << 11) |
						 (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
	
	outl(PCI_CONFIG_ADDRESS, address);
	
	tmp = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
	
	return tmp;
}

void pci_config_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data)
{
	uint32_t address;
	uint32_t lbus  = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint16_t tmp = 0;
	
	address = (uint32_t)((lbus << 16) | (lslot << 11) |
						 (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
	
	outl(PCI_CONFIG_ADDRESS, address);
	outl(PCI_CONFIG_DATA, data);
	//tmp = (uint16_t)((inportl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
	
	//return tmp;
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot)
{
	uint16_t vendor, device;
	if((vendor = pciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
		return vendor;
	}
	else {
		return 0;
	}
}

char* getVendorName(uint32_t vendor)
{
	switch(vendor)
	{
		case 0x8086:
			return "Intel Corporation ";
			break;
		case 0x1013:
			return "Cirrus Logic ";
			break;
		case 0x1234:
			return "Technical Corp ";
			break;
		case 0x10EC:
			return "Realtek ";
			break;
		default:
			return "Unknown ";
	}
}

uint8_t pci_idx = 0;

void register_pci_device(uint8_t bus, uint8_t slot, uint16_t vendor)
{
	enum riku_device_type type = UnknownDevice;

	char* vendorstr = getVendorName(vendor);
	
	// Get device id
	uint16_t device = pciConfigReadWord(bus,slot,0,2);
	/*puts("\tDevice ID : ");
	 puthex(device);
	 puts("\n");*/
	
	// Class and subclass
	uint16_t class_subclass = pciConfigReadWord(bus,slot,0,10);
	uint8_t class_id;
	class_id = (uint8_t)((class_subclass >> 8) & 0xF);
	
	char device_name[64];
	strcpy(device_name, vendorstr);
	char devname[16];
	memset(devname, 0x0, 16 * sizeof(char));
	// Print class string
	if(class_id == 0xFF) {
		strcat(device_name, " Unknown class device");
		strcat(devname, "pciunk");
	} else if(class_id > 0x12 && class_id < 0xFE) {
		strcat(device_name, " Reserved class device");
		strcat(devname, "pcires");
	} else {
		strcat(device_name, PCI_CLASSES[class_id]);
		strcat(devname, PCI_CLASSES_SHORT[class_id]);
	}
	
	/* Enable bus mastering for network devices */
	if(class_id == 0x2)
	{
		printk("Found network device, enabling bus mastering.\n");
		uint16_t pciconf = pciConfigReadWord(bus,slot,0,4);
		/* pciconf contains the command register for the PCI device. Write bus mastering bit and write it on the bus */
		pciconf = pciconf | 0x0004; /* Set bit 2 (Bus Master) into command register */
		pci_config_write_word(bus,slot, 0, 4, pciconf);
		type = NetworkDevice;
	}
	
	uint16_t pci_hdr_type = pciConfigReadWord(bus, slot, 0, 14);
	uint8_t hdr_type = (uint8_t)(pci_hdr_type & 0x00FF);
	/* If header type & 0x80 != 0 : multifunction device */
	if((hdr_type & 0x80) != 0)
	{
		printk("multifunction PCI device detected, needs further parsing\n");
	}
	
	/* Find free PCI device index */
	/* TODO : this will cause trouble when we have > 10 devices */
	uint8_t pci_dev_index = 0;
	char pci_dev_index_str[3];
	while(devfs_find_node(devname))
	{
		pci_dev_index++;
		sprintdec(pci_dev_index, pci_dev_index_str);
		uint8_t devnameLen = strlen(devname);
		devname[devnameLen - 1] = '\0';
		strcat(devname, pci_dev_index_str);
	}
	
	/* Create temporary PCI device info */
	struct riku_devfs_node* n = hardware_create_node(devname);
	n->type = type;
	
	uint8_t tmp = 0;
	for(tmp=0; tmp<6; tmp++)
		n->resources[tmp].begin = 0x0;
	
	uint8_t dev_idx = 0;
	/* Parse each BAR from 0 to 5 */
	for(uint32_t i=0; i<5; i++)
	{
		/* Parse Base Address Register */
		uint16_t bar_lower, bar_upper;
		bar_lower = pciConfigReadWord(bus,slot,0,16 + 4*i);
		bar_upper = pciConfigReadWord(bus,slot,0,18 + 4*i);
		bar_lower = pciConfigReadWord(bus,slot,0,16 + 4*i);
		bar_upper = pciConfigReadWord(bus,slot,0,18 + 4*i);
		uint32_t bar = (bar_upper << 16) | bar_lower;
		
		if(bar != 0x00000000)
		{
			if(class_id == 0x03 && i == 0)
			{
				printk("Found VGA linear framebuffer...\n");
				type = HIDDevice;
				/*extern uintptr_t vgaframebuffer;
				vgaframebuffer = (uintptr_t)(bar & 0xFFFFFFF0);*/
			}
			if((bar & 0x00000001) != 0)
			{
				hardware_add_resource(n, PORTIO, (uintptr_t)((bar & 0xFFFFFFFC) & 0x0000FFFF), 0xFFFF);
			} else {
				hardware_add_resource(n, MMIO, (uintptr_t)(bar & 0xFFFFFFF0), 0xFFFF);
			}
			dev_idx++;
		}
	}
	
	devfs_add(n);
	
	return;
}

void probe_pci()
{
	uint16_t bus;
	uint16_t device;
	uint16_t vendor;
	printk("Enumerating PCI devices.\n");
	for(bus = 0; bus < 256; bus++) {
		for(device = 0; device < 32; device++) {
			vendor = pciCheckVendor((uint8_t)bus, (uint8_t)device);
			if(vendor != 0)
				register_pci_device(bus, device, vendor);
		}
	}
	printk("Finished PCI probe.\n");
}