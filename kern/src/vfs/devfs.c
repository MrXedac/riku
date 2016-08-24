#include <stdint.h>
#include "hw.h"
#include "vfs/devfs.h"
#include "serial.h"
#include "vga.h"

struct riku_devfs_node *devfs_last_node()
{
	struct riku_devfs_node *start = nullDev;
	while(start->next != 0x0)
		start = start->next;
	
	return start;
}

void devfs_add(struct riku_devfs_node* node)
{
	KTRACE("devfs: creating node /dev/");
	KTRACERAW(node->name);
	KTRACERAW("\n");
	struct riku_devfs_node *end = devfs_last_node(); /* Get last node */
	
	/* Next node */
	end->next = node;
	node->next = 0x0;
	
	return;
}