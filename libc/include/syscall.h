#ifndef __SYSCALL__
#define __SYSCALL__

#define SYSCALL1(ret, id, arg1) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; SYSCALL;" : "=a"(ret) : "r"(id), "r"(arg1) : "rbx")
#define SYSCALL2(ret, id, arg1, arg2) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; MOV %3, %%RDX; SYSCALL;" : "=a"(ret) : "r"(id), "r"(arg1), "r"(arg2) : "rbx", "rdx")
#define SYSCALL3(ret, id, arg1, arg2, arg3) __asm volatile("MOV %1, %%RAX; MOV %2, %%RBX; MOV %3, %%RDX; MOV %4, %%RSI; SYSCALL;" : "=a"(ret) : "r"(id), "r"(arg1), "r"(arg2), "r"(arg3) :"rbx", "rdx", "rsi")

#endif
