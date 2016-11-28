#ifndef __MODULE__
#define __MODULE__

/** 
 * Some defines for kernel modules.
 */
#define kputs(a) { puts("["); puts(module_name()); puts("] "); puts(a); }

#endif