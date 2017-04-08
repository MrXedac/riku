#include "sched.h"
#include "idt64.h"
#include "task.h"
#include "serial.h"
#include "vga.h"
#include <stdint.h>

void setup_sched()
{
  register_irq(0, &handle_timer);
	tasking_ready = 1;
  KTRACE("Initialized scheduler.\n");
  enable_interrupts();
}

void handle_timer(registers_t* regs)
{
  do_schedule();
  return;
}

void do_schedule()
{
  if(current_task) {
    if(current_task->next) {
      switch_to_task(current_task->next);
    } else {
      switch_to_task(task_list);
    }
  }
}
