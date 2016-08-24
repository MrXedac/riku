#ifndef __DEVFS__
#define __DEVFS__

/* Some defines for Riku's device virtual filesystem.
 * Please note that DEVFS is SEPARATED from Riku's main filesystem.
 * Every device lives in a totally different space than mountpoints.
 * This might not be the best design choice, but right now it's
 * implemented this way. */

enum riku_resource_type { PORTIO, MMIO, UNKNOWN };

/* Describes a device resource range */
struct riku_devfs_resource {
	uintptr_t begin; /* Resource begin */
	uintptr_t size; /* Resource size */
	enum riku_resource_type type; /* Resource type */
};

struct riku_devfs_node {
	char name[16]; /* Device node name */
	struct riku_devfs_resource resources[6]; /* PCI devices has a maximum of 6 BAR in PCI configuration space. We align on this. */
	/* TODO : Device handlers here */
	struct riku_devfs_node* next; /* Next node in device space */
};

struct riku_devfs_node *devfs_last_node();
void devfs_add(struct riku_devfs_node* node);

struct riku_devfs_node *nullDev; /* The NULL device. Reads NULL, writes NULL. A typical /dev/null. */

#endif