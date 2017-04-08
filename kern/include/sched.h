#ifndef __SCHED__
#define __SCHED__

#include "idt64.h"

/* Include file for R2DS, Riku's Really Dumb Scheduler */
void handle_timer(registers_t* regs);
void do_schedule(); /* Self-explicit uh ? */
void setup_sched();

#endif
