#ifndef __TASK__
#define __TASK__

#include "idt64.h"

uint8_t tasking_ready;

enum task_state { READY, ACTIVABLE, SLEEPING, TERMINATED };

struct riku_task {
	char name[32]; /* Task name */
	uintptr_t task_rsp, task_rbp; /* Interrupted RSP */
	void (*entrypoint)(); /* Entrypoint function pointer */
	uintptr_t kernel_rsp; /* Kernel RSP to store into TSS */
	uintptr_t vm_root; /* CR3 register for task */
	struct riku_task *next; /* Next task into linked list */
	enum task_state state; /* Task state */
};

struct riku_task *current_task;
struct riku_task *task_list;

/* Initialize a new task */
void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)(), uintptr_t* cr3);

/* Task switch */
void switch_to_task(struct riku_task* task);
void start_task();

#endif