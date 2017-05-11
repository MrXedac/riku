#include "task.h"
#include "idt64.h"
#include "gdt64.h"
#include "mmu.h"
#include "vga.h"
#include "multiboot.h"
#include "printk.h"
#include "elf64.h"
#include <stdint.h>

void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)(), uintptr_t* cr3)
{
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
	/* Kernel task */
	if(current_task->vm_root == (uintptr_t)kernel_cr3)
	{
		/* Start task ! */
		puts("Starting task.\n");
		current_task->state = ACTIVABLE;
		current_task->entrypoint();
	}
}

/* Switches to another stack, given a stack and an interrupt context */
void switch_to_task(struct riku_task* task)
{
	/* Disable interrupts */
	__asm volatile("CLI");

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
		start_task(current_task);
	}

	__asm volatile("STI");

	/* Done ! The task switch should occur on interrupt return. */
	return;
}

/* Parse the multiboot header to find some relevant data */
void spawn_init(uintptr_t mbi, uintptr_t vme)
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
			elf64_load_binary(hdr, mod->size, vme);
		}
	}
}
