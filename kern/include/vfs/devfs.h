#ifndef __DEVFS__
#define __DEVFS__

#include "iotypes.h"

/* Some defines for Riku's device virtual filesystem.
 * Please note that DEVFS is SEPARATED from Riku's main filesystem.
 * Every device lives in a totally different space than mountpoints.
 * This might not be the best design choice, but right now it's
 * implemented this way. */

enum riku_resource_type { PORTIO, MMIO, UNKNOWN };
enum riku_device_type { HIDDevice, StorageDevice, NetworkDevice, SpecialDevice, UnknownDevice };
struct riku_devfs_node;

/* For device write functions
 * buffer : input buffer
 * count : size of buffer
 * Returns error code, or 0 if everything is OK
 */
typedef int (*write_t)(struct riku_devfs_node* self, const char* buffer, uint32_t count);
typedef int (*writechar_t)(struct riku_devfs_node* self, char c);

/* For device write functions
 * buffer : buffer to store read data
 * count : amount of bytes to read
 * Returns error code, or 0 if everything is OK
 */
typedef int (*read_t)(struct riku_devfs_node* self, const char* buffer, uint32_t count);
typedef int (*readchar_t)(struct riku_devfs_node* self, char* buffer);

/* Seek function, to put the reader at the given address */
typedef int (*seek_t)(struct riku_devfs_node* self, uint64_t position);

/* Describes a device resource range */
struct riku_devfs_resource {
	uintptr_t begin; /* Resource begin */
	uintptr_t size; /* Resource size */
	enum riku_resource_type type; /* Resource type */
};

struct riku_devfs_node {
	char name[16]; /* Device node name */
	enum riku_device_type type;
	uint64_t position;
	struct riku_devfs_resource resources[6]; /* PCI devices has a maximum of 6 BAR in PCI configuration space. We align on this. */
	/* TODO : Device handlers here */
	write_t write;
	read_t read;
	writechar_t putch;
	readchar_t getch;
	seek_t seek;
	struct riku_devfs_node* next; /* Next node in device space */
	struct riku_devfs_node* parent; /* Parent, if node is a subset of an existing device (e.g. partition) */
	void* extended; /* Extended info for device or filesystem */
};

struct riku_devfs_node *devfs_last_node();
void devfs_add(struct riku_devfs_node* node);
struct riku_devfs_node *devfs_find_node(char* name);
struct riku_devfs_node* hardware_create_node(const char* name);
void hardware_add_resource(struct riku_devfs_node* node, enum riku_resource_type type, uintptr_t begin, uintptr_t size);

struct riku_devfs_node *nullDev; /* The NULL device. Reads NULL, writes NULL. A typical /dev/null. */
struct riku_devfs_node *console;
struct riku_devfs_node *kconsole;
struct riku_devfs_node *kinput;
struct riku_devfs_node *devfsVirtPtr; /* "Virtual" pointer to devfs, which implements nothing. For cosmetic issues. */
struct riku_devfs_node *ramfsVirtPtr;

#endif
