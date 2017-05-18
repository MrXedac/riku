#ifndef __TASK__
#define __TASK__

#include "idt64.h"
#include "vfs/devfs.h"
#include "vfs/descriptor.h"

uint8_t tasking_ready;

#define INIT_STACK	0x1080000
#define	NODE_READ		0x1
#define NODE_WRITE	0x2
#define MAX_FILES		16

enum task_state { READY, ACTIVABLE, SLEEPING, TERMINATED };

/* File descriptor structure */


struct riku_task {
	char name[32]; /* Task name */
	uintptr_t task_rsp, task_rbp; /* Interrupted RSP */
	void (*entrypoint)(); /* Entrypoint function pointer */
	uintptr_t kernel_rsp; /* Kernel RSP to store into TSS */
	uintptr_t vm_root; /* CR3 register for task */
	struct riku_task *next; /* Next task into linked list */
	enum task_state state; /* Task state */
	struct riku_descriptor* files[MAX_FILES]; /* Pointers to descriptors associated to task (in kernel heap) */
};

struct riku_task *current_task;
struct riku_task *task_list;

/* Initialize a new task */
void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)(), uintptr_t* cr3);

/* Task switch */
void switch_to_task(struct riku_task* task);
void start_task();
uint64_t spawn_init(uintptr_t mbi, uintptr_t vme);
void update_task_vme(struct riku_task* task, uintptr_t vme);


#endif
