#ifndef __TASK__
#define __TASK__

#include "idt64.h"


enum task_state { READY, ACTIVABLE, SLEEPING, TERMINATED };

struct riku_task {
	char name[32]; /* Task name */
	struct registers regs; /* Interrupted task registers */
	uintptr_t kernel_rsp; /* Kernel RSP to store into TSS */
	uintptr_t vm_root; /* CR3 register for task */
	struct riku_task *prev; /* Previous task into linked list */
	struct riku_task *next; /* Next task into linked list */
	enum task_state state; /* Task state */
};

struct riku_task *current_task;

/* Initialize a new task */
void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)());

/* Spawn a task into kernel mode */
void run_kernel_task(struct riku_task* task);

/* Spawn a task into user mode */
void run_user_task(struct riku_task* task, uintptr_t* vmroot);

/* Task switch */
void switch_to_task(struct riku_task* task, struct registers *interrupt_ctx);

/* Save interrupt registers into current task */
void save_registers(struct registers* regs);

/* Restore interrupt registers from task */
void restore_registers(struct registers* regs);

#endif