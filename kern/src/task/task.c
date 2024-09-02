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
#include "errno.h"
#include "ioport.h"
#include "fs_vfat.h"
#include "vfs/devfs.h"
#include "vfs/fs.h"
#include "vfs/openclose.h"
#include "vfs/readwrite.h"
#include "vfs/stat.h"
#include <stdint.h>

uint64_t next_pid = 0;
uintptr_t user_rsp = 0;
uintptr_t user_rbp = 0;

const char *__env[1] = { 0 };

uint64_t getpid()
{
    return current_task->pid;
}

uint64_t getppid()
{
    return current_task->ppid;
}

int cwd(char* wd)
{
    memset(current_task->cwd, 0, 128);
    strcpy(current_task->cwd, wd);
    return 0;
}

int gwd(char* buffer)
{
    strcpy(buffer, current_task->cwd);
    return 0;
}

void wake_up_tasks()
{
    /* Iterate on all tasks and wake suspended ones */
    struct riku_task* task = current_task;
    printk("task %d waking up pending tasks\n", current_task->pid);
    do {
        task = task->next;
        if(task == 0x0)
        {
            printk("null task next pointer, bug?\n");
            return;
        } 
        
        if(task != 0x0 && task->waiting_on == current_task)
        {
            task->waiting_on = (void*)0x0;
            task->state = ACTIVABLE;
        }
    } while(task != current_task);
}

void exit(int exitcode) 
{
    /* Set task as terminated, kernel should free tasks when scheduling */
    current_task->exit_code = exitcode;
    current_task->state = TERMINATED;

    if(current_task->pid == 0x0)
        panic("init process exited", (void*)0);

    printk("terminated task %s\n", current_task->name);
    wake_up_tasks();
    switch_to_task(current_task->next);
    return;
}

const char** environ()
{
    /* TODO : add environment in task structure */
    return __env;
}

int execve(char* name, char** argv, char** env)
{
    printk("execve: running %s\n", name);
    /* Find init in initramfs; TODO : use correct API instead of doing everything by hand */
	int fd_init = open(name, 1);
	if(fd_init < 0) {
        printk("execve: not found\n");
        return -EBINFMT;
	}

    printk("binary opened with fd %d\n", fd_init);

    uintptr_t init_addr = 0x0, init_size = 0x0;
	struct riku_stat fileInfo;
	int ret = stat(fd_init, &fileInfo);
    printk("execve: stat returned %d\n");

	// This is our binary - we need to map contiguous memory to store the binary 
	int curSize = fileInfo.size;
    printk("execve: bin size %d\n", curSize);
	int map_addr = 0x3000000;
	while(curSize > 0)
	{
		uintptr_t* page = alloc_page();

		printk("execve: vme: map %x to %x\n",LIN((uintptr_t)page), map_addr);
		vme_map(current_task->vm_root, LIN((uintptr_t)page), map_addr, 1);

		curSize -= PAGE_SIZE;
		map_addr += PAGE_SIZE;
	}

	read(fd_init, (char*)0x3000000, fileInfo.size);	
	init_addr = 0x3000000;
	close(fd_init);

    if(init_addr == 0x0) return -1;

	printk("%s binary at %x\n", name, init_addr);
    uint64_t rip = spawn_init(init_addr, init_size, current_task->vm_root);

    if(rip == -1)
    {
        printk("file is not ELF64 binary\n");
        return -EBINFMT;
    }

    /* Binary spawned, move argv somewhere safe */
    /* Map heap, put argv in it */
    current_task->heap = INIT_HEAP + 0x1000;
    uintptr_t* argv_page = alloc_page();
    uintptr_t* heap_initial_page = alloc_page();

    /* TODO : BAD !! We map TLS at 0x0 because we didnt set up FS... */
    uintptr_t* tls = alloc_page();

    vme_unmap(current_task->vm_root, INIT_HEAP);
    vme_map(current_task->vm_root, LIN((uintptr_t)argv_page), INIT_HEAP, 1);
    vme_map(current_task->vm_root, LIN((uintptr_t)heap_initial_page), INIT_HEAP + 0x1000, 1);
    vme_map(current_task->vm_root, LIN((uintptr_t)tls), 0x0, 1);
    printk("mapped new heap, user argv at %x\n", argv);
    int argv_index = 0;
    int offset = 0;
    char** argvptr = (char**)INIT_HEAP;
    while(argv[argv_index] != 0)
    {
        strcpy((char*)INIT_HEAP + offset, argv[argv_index]);
        offset += strlen(argv[argv_index]) + 1; /* Add null termination byte to offset... */
        argv_index++;
    }

    /* Build new argv structure */
    argv_index = 0;
    uintptr_t* argv_ptr = (uintptr_t*)INIT_HEAP + offset;
    uintptr_t* argv_ptr_for_userland = argv_ptr;
    int base_offset = 0;
    int argc = 0;
    while(argv[argv_index] != 0)
    {
        *argv_ptr = (uintptr_t)(INIT_HEAP + base_offset);
        argv_ptr++;
        base_offset += (strlen(argv[argv_index]) + 1);
        argv_index++;
        argc++;
    }

	/* Map user stack somewhere safe */
    uintptr_t* stack = alloc_page();
    vme_unmap(current_task->vm_root, INIT_STACK);
    vme_map(current_task->vm_root, LIN((uintptr_t)stack), INIT_STACK, 1);

	current_task->task_rsp = INIT_STACK + PAGE_SIZE - sizeof(uintptr_t);
	current_task->task_rbp = INIT_STACK + PAGE_SIZE - sizeof(uintptr_t);

    strcpy(current_task->name, name);

    /* Drop kernel context, restart from userland */
	extern void enter_userland(uint64_t rip, uint64_t rsp, uint64_t argv, uint64_t argc);
	enter_userland(rip, INIT_STACK + PAGE_SIZE - sizeof(uintptr_t), (uint64_t)argv_ptr_for_userland, argc);

    return 0;
}

int isatty(int descriptor)
{
    return 1; /* Let's consider everything is a terminal */
    /* TODO : add support for files ! */
}

uintptr_t sbrk(int incr)
{
    if(current_task->heap == 0x0) current_task->heap = INIT_HEAP;

    if(incr == 0x0) return current_task->heap;

    printk("sbrk: incr %x\n", incr);
    uintptr_t heap = current_task->heap;

    int size = incr;
    while(size <= PAGE_SIZE)
    {
        uintptr_t* newpage = alloc_page();
        vme_map(current_task->vm_root, PHYS((uintptr_t)newpage), current_task->heap, 1);
        size -= PAGE_SIZE;
        current_task->heap += PAGE_SIZE;
    }

    return heap;
}

int wait(int pid)
{
    /* Make current task sleep until task is finished */
    struct riku_task* task = current_task;
    do {
        task = task->next;
        if(task->pid == pid)
        {
            printk("sched: task pid %d waiting on task pid %d\n", current_task->pid, task->pid);
            current_task->waiting_on = task;
            current_task->state = SLEEPING;
            break;
        }
    } while(task != current_task);

    /* Schedule */
    if(task->state == TERMINATED) {
        printk("task %d already terminated with exit code %d, resuming\n", pid, task->exit_code);
        current_task->state = ACTIVABLE;
        current_task->waiting_on = (void*)0x0;
        return task->exit_code;
    }

    /* Switch to another context */
    switch_to_task(current_task->next);

    /* TODO : BUG ? Send ack signal to PIC */
    outb(0x20, 0x20);

    /* Returned from schedule, task should have finished, grab exit code */
    return task->exit_code;
}

extern void ret_from_fork();
uintptr_t fork_stack;
uintptr_t fork_rbp;

uint64_t fork()
{
    /* Keep R14 / Fork stack safe */
    __asm volatile("MOV %%R14, %0":"=r"(fork_stack));
    __asm volatile("MOV %%R15, %0":"=r"(fork_rbp));

    printk("RSP %x, RBP %x\n", fork_stack, fork_rbp);
    uintptr_t* forkedKernRsp = alloc_page();
    printk("New stack at %x\n", forkedKernRsp);
    /* Duplicate stack */
    /*uintptr_t offset = fork_rbp - fork_stack;
    uintptr_t* forkedStack = alloc_page();

    
    memcpy((uintptr_t*)PHYS((uintptr_t)forkedStack), (uintptr_t*)INIT_STACK, 0x1000);
*/
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
    forked_tsk->task_rbp = fork_rbp;

    /* Use the newly-created VME for the task */
    //printk("new stack page at %x\n", LIN((uintptr_t)forkedStack));
    //
    //forked_tsk->kernel_rsp = 0x0000001000000000 + 0x1000 - sizeof(uintptr_t);
    memcpy((uintptr_t*)PHYS((uintptr_t)forkedKernRsp), (uintptr_t*)(fork_stack & 0xFFFFFFFFFFFFF000), PAGE_SIZE);
    printk("copied kernel stack into %x from %x\n", (uintptr_t*)PHYS((uintptr_t)forkedKernRsp), (uintptr_t*)(fork_stack & 0xFFFFFFFFFFFFF000));
    vme_map(new_vme, LIN((uintptr_t)forkedKernRsp), 0x2080000, 0);
    update_task_vme(forked_tsk, new_vme);

    /* Increase file descriptor active clients */
    for(uint64_t i = 0; i < MAX_FILES; i++)
    {
        if(forked_tsk->files[i])
            forked_tsk->files[i]->clients++;
    }

    current_task->next = forked_tsk;

    /* We "should" be good. Forking should be done now. */
    printk("forking done");
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

        printk("Registered task %s into task list\n", name);
    } else {
        /* Build task list */
        task_list = task;
        task->next = task;
    }

    task->vm_root = (uintptr_t)cr3;
    task->heap = 0x0; /* Did not SBRK anything yet */

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
    /* Ignore terminated or sleeping tasks */
    struct riku_task* target = task;
    while(target->state == TERMINATED || target->state == SLEEPING) target = target->next;

    /* Save current RSP into current task */
    if(current_task)
    {
        __asm volatile("MOV %%RSP, %0; \
                MOV %%RBP, %1"
                : "=r" (current_task->task_rsp), "=r" (current_task->task_rbp));
    }

    tss_set_kern_stack(target->kernel_rsp);

    /* Switch contexts */
    current_task = target;

    /* Switch CR3 and RSP */
    __asm volatile("MOV %0, %%CR3; \
            MOV %1, %%RSP; \
            MOV %2, %%RBP"
            :: "r" (target->vm_root), "r" (target->task_rsp), "r" (target->task_rbp));

    if(current_task->state == READY)
    {
        current_task->state = ACTIVABLE;
        outb(0x20, 0x20);
        __asm volatile("MOV %0, %%RAX; \
                JMP *%%RAX"
                :: "r"(current_task->entrypoint));
    }

    /* Done ! The task switch should occur on interrupt return. */
    outb(0x20, 0x20);
    return;
}

/* Parse the multiboot header to find some relevant data */
uint64_t spawn_init(uintptr_t init_addr, uintptr_t init_size, uintptr_t vme)
{
    Elf64_Ehdr* hdr = (Elf64_Ehdr*)init_addr;
    return elf64_load_binary(hdr, init_size, vme);
}

uintptr_t find_initramfs(uintptr_t mbi)
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
            return PHYS(((uintptr_t)(mod->mod_start)));
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
