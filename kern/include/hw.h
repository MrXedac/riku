#ifndef __HW__
#define __HW__

/* This is the hell of a put-anything-in-it function.
 * Basically it probes any available hardware on the machine.
 * Right now it should probe the hardware we're already aware of,
 * as well as PCI devices. The remaining devices should be probed
 * in kernel modules. */
void probe_hardware();

#endif