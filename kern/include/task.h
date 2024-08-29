#ifndef __TASK__
#define __TASK__

#include "idt64.h"
#include "vfs/devfs.h"
#include "vfs/descriptor.h"

uint8_t tasking_ready;
uint64_t next_pid;

#define INIT_STACK		0x1080000
#define INIT_KERN_STACK	0x2080000
#define	NODE_READ		0x1
#define NODE_WRITE	0x2
#define MAX_FILES		16
#define INIT_HEAP	0x20000000

enum task_state { READY, ACTIVABLE, SLEEPING, TERMINATED };

struct riku_task {
	uint64_t pid;
	uint64_t ppid;
	char name[32]; /* Task name */
	uintptr_t task_rsp, task_rbp; /* Interrupted RSP */
	void (*entrypoint)(); /* Entrypoint function pointer */
	uintptr_t kernel_rsp; /* Kernel RSP to store into TSS */
	uintptr_t vm_root; /* CR3 register for task */
	struct riku_task *next; /* Next task into linked list */
	enum task_state state; /* Task state */
	struct riku_descriptor* files[MAX_FILES]; /* Pointers to descriptors associated to task (in kernel heap) */
	uintptr_t heap; /* Heap base address */
	struct riku_task* waiting_on; /* Task this task is waiting completion */
	int exit_code; /* Exit code of the task */
};

struct riku_task *current_task;
struct riku_task *task_list;

/* Initialize a new task */
void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)(), uintptr_t* cr3);

/* Task switch */
void switch_to_task(struct riku_task* task);
void start_task();
uint64_t spawn_init(uintptr_t init_addr, uintptr_t init_size, uintptr_t vme);
uintptr_t find_initramfs(uintptr_t mbi);
void update_task_vme(struct riku_task* task, uintptr_t vme);
uint64_t getpid();
uint64_t getppid();
uint64_t fork(); /* Forks a task into another one */
void exit(); /* Exits current process */
const char** environ(); /* Pointer to environment variables */
int execve(char* name, char** argv, char** env);
int isatty(int descriptor);
uintptr_t sbrk(int incr); /* Increases process heap */
int wait(int pid);

#endif
