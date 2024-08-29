#ifndef __SYSCALL__
#define __SYSCALL__

#define SYSCALL1(ret, id, arg1) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; SYSCALL; MOV %%RAX, %0" : "=r"(ret) : "r"(id), "r"(arg1) : "rax", "rbx")
#define SYSCALL2(ret, id, arg1, arg2) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; MOV %3, %%RDX; SYSCALL; MOV %%RAX, %0" : "=r"(ret) : "r"(id), "r"(arg1), "r"(arg2) : "rax", "rbx", "rdx")
#define SYSCALL3(ret, id, arg1, arg2, arg3) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; MOV %3, %%RDX; MOV %4, %%RSI; SYSCALL; MOV %%RAX, %0" : "=r"(ret) : "r"(id), "r"(arg1), "r"(arg2), "r"(arg3) :"rax", "rbx", "rdx", "rsi")

#endif
