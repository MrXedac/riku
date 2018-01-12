#include "task.h"
#include "idt64.h"
#include "gdt64.h"
#include "mmu.h"
#include "vga.h"
#include "multiboot.h"
#include "printk.h"
#include "elf64.h"
#include "heap.h"
#include "mem.h"
#include "vm.h"
#include "ioport.h"
#include "fs_vfat.h"
#include <stdint.h>

uint64_t next_pid = 0;

uint64_t getpid()
{
	return current_task->pid;
}

uint64_t getppid()
{
	return current_task->ppid;
}

extern void ret_from_fork();
uintptr_t fork_stack;

uint64_t fork()
{
	/* Keep R14 / Fork stack safe */
	__asm volatile("MOV %%R14, %0":"=r"(fork_stack));
	
	printk("Requested fork from current task %x\n", current_task);
	/* First thing to do is to duplicate the current task's VME and set it as read-only */
	uintptr_t new_vme = clone_vme(current_task->vm_root);
	set_vme_as_ro(new_vme);

	printk("Created new VME for forked task at %x\n", new_vme);
	/* Now that our new VME is ready, we can safely set the current one as read-only */
	set_vme_as_ro(current_task->vm_root);

	/* Reload VME, invalidating TLB */
	__asm volatile("MOV %0, %%CR3" :: "r"(current_task->vm_root));

	printk("Current VME is now read-only\n");

	/* Okay, now both VMEs are read-only, and we should get some copy-on-write stuff now. Create a new task structure for our forked task */
	struct riku_task* forked_tsk = (struct riku_task*)kalloc(sizeof(struct riku_task));
	memcpy(forked_tsk, current_task, sizeof(struct riku_task));

	/* That being said, we have a few changes to make in both the current and new task in order to get things working */
	/* Update PID and PPID */
	forked_tsk->pid = next_pid;
	next_pid++;
	forked_tsk->ppid = current_task->pid;

	/* Correctly fix scheduling ring */
	/* Just so I remember, there is a HUGE problem with scheduling, as the process has no "interrupt return" structure available, and therefore crashes everything.
	 * I don't know yet how I'll fix this. Next thing to do */
	forked_tsk->entrypoint = &ret_from_fork;
	forked_tsk->state = READY;
	forked_tsk->task_rsp = fork_stack;
	forked_tsk->task_rbp = fork_stack;

	/* Use the newly-created VME for the task */
	update_task_vme(forked_tsk, new_vme);

	/* Increase file descriptor active clients */
	for(uint64_t i = 0; i < MAX_FILES; i++)
	{
		if(forked_tsk->files[i])
			forked_tsk->files[i]->clients++;
	}

	current_task->next = forked_tsk;
	
	/* We "should" be good. Forking should be done now. */
	return forked_tsk->pid;
}

void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)(), uintptr_t* cr3)
{
	memset(task, 0x0, sizeof(struct riku_task));

	task->pid = next_pid;
	next_pid++;

	/* Store task name */
	uint32_t i = 0;
	while(i < 32 && name[i] != '\0') {
		task->name[i] = name[i];
		i++;
	}
	task->name[i] = '\0';

	/* Store entrypoint */
	task->entrypoint = entrypoint;

	/* Store stack */
	task->task_rsp = (uintptr_t)stack + PAGE_SIZE;
	task->task_rbp = (uintptr_t)stack + PAGE_SIZE;
	task->kernel_rsp = (uintptr_t)kernrsp + PAGE_SIZE;

	/* Task is ready */
	task->state = READY;

	/* All right, spawn the task into linked list */
	if(current_task)
	{
		/* Insert it properly */
		task->next = current_task->next;
		current_task->next = task;

		puts("Registered task ");
		puts(name);
		puts(" into task list.\n");
	} else {
		/* Build task list */
		task_list = task;
		task->next = 0;
	}

	task->vm_root = (uintptr_t)cr3;

	/* We're done */
	return;
}

void start_task()
{
	/* Start task ! */
	printk("Starting task.\n");

}

/* Switches to another stack, given a stack and an interrupt context */
void switch_to_task(struct riku_task* task)
{
	/* Save current RSP into current task */
	if(current_task)
	{
		__asm volatile("MOV %%RSP, %0; \
					    MOV %%RBP, %1"
					   : "=r" (current_task->task_rsp), "=r" (current_task->task_rbp));
	}

	tss_set_kern_stack(task->kernel_rsp);

	/* Switch contexts */
	current_task = task;

	/* Switch CR3 and RSP */
	__asm volatile("MOV %0, %%CR3; \
					MOV %1, %%RSP; \
					MOV %2, %%RBP"
				   :: "r" (task->vm_root), "r" (task->task_rsp), "r" (task->task_rbp));

	if(current_task->state == READY)
	{
		current_task->state = ACTIVABLE;
		outb(0x20, 0x20);
		__asm volatile("MOV %0, %%RAX; \
					   JMP *%%RAX"
					   :: "r"(current_task->entrypoint));
	}

	/* Done ! The task switch should occur on interrupt return. */
	return;
}

/* Parse the multiboot header to find some relevant data */
uint64_t spawn_init(uintptr_t mbi, uintptr_t vme)
{
	struct multiboot_header* header = (struct multiboot_header*)PHYS(mbi);
	struct multiboot_tag *tag;
	uint64_t addr = (uint64_t)header;

	for (tag = (struct multiboot_tag *) (addr + 8);
		 tag->type != MULTIBOOT_TAG_TYPE_END;
		 tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
										 + ((tag->size + 7) & ~7)))
	{
		if (tag->type == MULTIBOOT_TAG_TYPE_MODULE)
		{
			struct multiboot_tag_module* mod = (struct multiboot_tag_module*)tag;
			printk("boot process module2 type %d, size %x, start %x, end %x, cmdline %s\n", mod->type, mod->size, mod->mod_start, mod->mod_end, mod->cmdline);
            Elf64_Ehdr* hdr = (Elf64_Ehdr*)PHYS(((uintptr_t)(mod->mod_start)));
            /* Parse ramdrive entry */
            /* TODO : do this properly vfat_readHeader((uintptr_t*)PHYS(((uintptr_t)(mod->mod_start)))); */
			return elf64_load_binary(hdr, mod->size, vme);
		}
	}
	return 0x0;
}

/* Change a task's VME */
void update_task_vme(struct riku_task* task, uintptr_t vme)
{
	task->vm_root = vme;
	return;
}
