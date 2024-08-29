#ifndef __X86_PCI__
#define __X86_PCI__

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

struct pci_config_address {
	uint8_t null_entry : 2;
	uint8_t reg_number : 6;
	uint8_t fun_number : 3;
	uint8_t dev_number : 5;
	uint8_t bus_number : 8;
	uint8_t reserved : 7;
	uint8_t enable : 1;
};

static char* PCI_CLASSES[18] = {
	"Old PCI device",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controller",
	"Base System Peripherals",
	"Input Device",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
	"Intelligent I/O Controller",
	"Satellite Communication Controller",
	"Encryption/Decryption Controller",
	"Data Acquisition and Signal Processing Controller",
};

static char* PCI_CLASSES_SHORT[18] = {
	"pci0",
	"pcistor0",
	"pcinet0",
	"pcivga0",
	"pcimmc0",
	"pcimc0",
	"pcibrd0",
	"pcismc0",
	"pcibsp0",
	"pciio0",
	"pcidock0",
	"pcicpu0",
	"pcisl0",
	"pciwlc0",
	"pciiioc0",
	"pciscc0",
	"pciedc0",
	"pcidaspc0",
};

typedef struct pci_config_address PCI_CONFIG_ADDRESS_STRUCT;

// Read a word from a PCI bus
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

// Extract a vendor from a PCI bus/slot
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);

// Debug : enumerate connected PCI devices
void enumeratePci();

// Get vendor name from id
char* getVendorName(uint32_t vendor);

#endif