#include "task.h"
#include "idt64.h"
#include "gdt64.h"
#include "mmu.h"

void init_task(struct riku_task* task, char* name, uintptr_t* stack, uintptr_t* kernrsp, void (*entrypoint)())
{
	/* Store task name */
	uint32_t i = 0;
	while(i < 32 && name[i] != '\0') {
		task->name[i] = name[i];
		i++;
	}
	
	/* Nullify task registers */
	task->regs.rax = 0;
	task->regs.rbx = 0;
	task->regs.rcx = 0;
	task->regs.rdx = 0;
	task->regs.rsi = 0;
	task->regs.rdi = 0;
	task->regs.r8 = 0;
	task->regs.r9 = 0;
	task->regs.r10 = 0;
	task->regs.r11 = 0;
	task->regs.r12 = 0;
	task->regs.r13 = 0;
	task->regs.r14 = 0;
	task->regs.r15 = 0;
	
	/* Get EFLAGS */
	uintptr_t flags;
	__asm volatile("PUSHF; \
					POP %%RAX; \
					MOV %%RAX, %0"
				   : "=r" (flags));
	task->regs.eflags = flags;
	
	/* Store entrypoint */
	task->regs.rip = (uint64_t)entrypoint;
	
	/* Store stack */
	task->regs.userrsp = (uint64_t)stack;
	task->regs.rbp = (uint64_t)stack;
	task->regs.rsp = (uint64_t)kernrsp;
	task->kernel_rsp = (uintptr_t)kernrsp;
	
	/* Task is ready */
	task->state = READY;
	
	/* We're done */
	return;
}

/* Spawn a task into kernel mode */
void run_kernel_task(struct riku_task* task)
{
	/* Setup kernel cr3 as VM root */
	task->vm_root = (uintptr_t)kernel_cr3;
	
	/* Setup kernel segments */
	task->regs.cs = 0x8;
	task->regs.ss = 0x10;
	
	/* All right, spawn the task into linked list */
	if(!current_task)
	{
		/* Only element in linked list */
		task->prev = task;
		task->next = task;
		current_task = task;
	} else {
		/* Insert it properly */
		task->prev = current_task;
		task->next = current_task->next;
		/* Set-up the old next task to include our newly-created task */
		current_task->next->prev = task;
		current_task->next = task;
	}
	
	/* We're done */
	return;
}

/* Spawn a task into user mode */
void run_user_task(struct riku_task* task, uintptr_t* vmroot)
{
	/* Setup kernel cr3 as VM root */
	task->vm_root = (uintptr_t)vmroot;
	
	/* Setup user segments */
	task->regs.cs = 0x18;
	task->regs.ss = 0x20;
	
	/* All right, spawn the task into linked list */
	if(!current_task)
	{
		/* Only element in linked list */
		task->prev = task;
		task->next = task;
		current_task = task;
	} else {
		/* Insert it properly */
		task->prev = current_task;
		task->next = current_task->next;
		/* Set-up the old next task to include our newly-created task */
		current_task->next->prev = task;
		current_task->next = task;
	}
	
	/* We're done */
	return;
}

/* Save interrupt registers into current task */
void save_registers(struct registers* regs)
{
	current_task->regs.rax = regs->rax;
	current_task->regs.rbx = regs->rbx;
	current_task->regs.rcx = regs->rcx;
	current_task->regs.rdx = regs->rdx;
	current_task->regs.rsi = regs->rsi;
	current_task->regs.rdi = regs->rdi;
	current_task->regs.rbp = regs->rbp;
	current_task->regs.rsp = regs->rsp;
	current_task->regs.r8 = regs->r8;
	current_task->regs.r9 = regs->r9;
	current_task->regs.r10 = regs->r10;
	current_task->regs.r11 = regs->r11;
	current_task->regs.r12 = regs->r12;
	current_task->regs.r13 = regs->r13;
	current_task->regs.r14 = regs->r14;
	current_task->regs.r15 = regs->r15;
	current_task->regs.userrsp = regs->userrsp;
	return;
}

/* Restore interrupt registers from task */
void restore_registers(struct registers* regs_target)
{
	regs_target->rax = current_task->regs.rax;
	regs_target->rbx = current_task->regs.rbx;
	regs_target->rcx = current_task->regs.rcx;
	regs_target->rdx = current_task->regs.rdx;
	regs_target->rsi = current_task->regs.rsi;
	regs_target->rdi = current_task->regs.rdi;
	regs_target->rbp = current_task->regs.rbp;
	regs_target->rsp = current_task->regs.rsp;
	regs_target->r8 = current_task->regs.r8;
	regs_target->r9 = current_task->regs.r9;
	regs_target->r10 = current_task->regs.r10;
	regs_target->r11 = current_task->regs.r11;
	regs_target->r12 = current_task->regs.r12;
	regs_target->r13 = current_task->regs.r13;
	regs_target->r14 = current_task->regs.r14;
	regs_target->r15 = current_task->regs.r15;
	regs_target->userrsp = current_task->regs.userrsp;
	return;
}

/* Switches to another stack, given a stack and an interrupt context */
void switch_to_task(struct riku_task* task, struct registers *interrupt_ctx)
{
	/* Save current task state */
	save_registers(interrupt_ctx);
	
	/* Task is interrupted, not ready */
	current_task->state = ACTIVABLE;
	
	/* We're doing some nifty-playing with the interrupt stack, so don't bother with changing kernel stack or whatever... */
	task->regs.rsp = interrupt_ctx->rsp;
	tss_set_kern_stack(task->kernel_rsp);
	
	/* Switch contexts */
	current_task = task;
	
	/* Switch CR3 */
	__asm volatile("MOV %0, %%CR3"
				   :: "r" (task->vm_root));
	
	/* Restore registers */
	restore_registers(interrupt_ctx);
	
	/* Done ! The task switch should occur on interrupt return. */
	/* If the task has already started, we should restore its previous interrupt context. If it didn't, we should have initialized its registers on spawn so it should be okay. */
	return;
}