#ifndef __DRIVER__
#define __DRIVER__

#define DRIVER_INIT(x) { extern void x##_init(); x##_init(); }

#endif
