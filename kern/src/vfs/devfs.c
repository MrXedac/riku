#include <stdint.h>
#include <string.h>
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
	printk("devfs: creating node devfs:/%s\n", node->name);
	struct riku_devfs_node *end = devfs_last_node(); /* Get last node */

	/* Next node */
	end->next = node;
	node->next = 0x0;

	return;
}

struct riku_devfs_node *devfs_find_node(char* name)	{
	struct riku_devfs_node *start = nullDev;
	do{
		if(strcmp(start->name, name) == 0)
			return start;

		start = start->next;
	} while(start != 0x0);

	return 0x0;
}
