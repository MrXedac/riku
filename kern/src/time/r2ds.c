#include "sched.h"
#include "idt64.h"
#include "task.h"
#include "serial.h"
#include "vga.h"
#include <stdint.h>

/* Kernel jiffies, youpi */
uint64_t jiffies;

void setup_sched()
{
  register_irq(0, &handle_timer);
  jiffies = 0;
	tasking_ready = 1;
  printk("Initialized scheduler.\n");
  enable_interrupts();
}

void handle_timer(registers_t* regs)
{
  if(jiffies >= 0xFFFFFFFFFFFFFFFE)
    jiffies = 0;
  else
    jiffies++;
  do_schedule();
  return;
}

void do_schedule()
{
	/* printk("sched: cur=%x, next=%x, list=%x\n", current_task, current_task->next, task_list); */
	if(current_task) {
		if(current_task->next) {
			switch_to_task(current_task->next);
		} else {
			switch_to_task(task_list);
		}
	}
}
