#ifndef __HW__
#define __HW__

#include "vfs/devfs.h"

/* This is the hell of a put-anything-in-it function.
 * Basically it probes any available hardware on the machine.
 * Right now it should probe the hardware we're already aware of,
 * as well as PCI devices. The remaining devices should be probed
 * in kernel modules. */
void probe_hardware();

struct riku_devfs_node* hardware_create_node(const char* name);
void hardware_add_resource(struct riku_devfs_node* node, enum riku_resource_type type, uintptr_t begin, uintptr_t size);
void probe_pci();

#endif