[global enter_userland]
; RIP is in RDI
; RSP in in RSI
; ARGV is in RDX
; ARGC is in RCX
enter_userland:
  mov rax, cr3
  mov cr3, rax
  ; Put argc and argv in R12 and R13
  mov r12, rcx
  mov r13, rdx
  mov rcx, rdi
  mov rsp, rsi
  mov rbp, rsp
  mov ax, 0x1b
  mov ds, ax
  mov rax, 0
  mov rbx, 0
  mov rsi, 0
  mov rdi, rdx
  mov rdx, 0
  mov r11, 0x200
  o64 sysret

; Fixes the TF flag in FLAGS
[global fix_tf]
fix_tf:
  pushfq
  and qword [rsp], 0xFFFFFFFFFFFFFCFF
  popfq
  ret
 
[extern fork_stack]
[global ret_from_fork]
ret_from_fork:
 ; Pop R15 and R14 to preserve them
 pop qword r15
 pop qword r14

 pop qword r11
 or r11, 0x200 ; Re-enable interrupts
 pop qword rcx
 mov bx, 0x23
 mov ds, bx
 mov rax, 0
 pop qword r12
 pop qword r10
 mov rbp, r12
 mov rsp, r10
 o64 sysret
 jmp $
 

 
[global syscall_ep]
[extern syscall_table] ; This struct, defined in syscall.c, contains the syscall address table
; RAX = System call number
; RBX = Arg1
; RCX = User RIP
; RDX = Arg2
; RSI = Arg3
; RDI = Arg4
; R8 = Arg5
; R9 = Arg6
; R11 = RFLAGS
syscall_ep:
  cli

  mov qword r10, rsp
  mov qword r12, rbp

  mov qword rsp, 0x2080FF0
  mov qword rbp, rsp

  push qword r10 ; RSP
  push qword r12 ; RBP
  push qword rcx ; RIP
  push qword r11 ; RFLAGS
  mov cx, 0x10
  mov ds, cx

  cmp rax, 0x12 ; Remember to change this whenever a system call is added
  jle do_syscall
  ; If we end up here, we selected an invalid system call. Return -1 (-ENOSYSC)
  mov rax, -1
  jmp ret_from_call
do_syscall:
  ; Save our arguments and system call number somewhere safe
  push rbx
  push rdx
  push rsi
  push rdi
  push r8
  push r9

  ; Put the system call table base address into RAX
  mov rbx, syscall_table

  ; Pops RAX from stack into RBX to get our syscall offset
  ; Remember that syscalls begin from 1, bur our table indexes it from 0
  ; Each address is expressed into 64 bits values, so 0x8 is our per-entry offset
  sub rax, 0x1
  mov rcx, 0x8
  mul rcx

  ; Find the appropriate entry
  add rbx, rax

  ; Get arguments from stack
  pop r9
  pop r8
  pop rcx
  pop rdx
  pop rsi
  pop rdi

  ; Preserve R14 and R15
  push r14
  push r15 

  mov r14, rsp ; Keep RSP in R14, in order to have a safe fork() return
  mov r15, rbp ; Same for RBP
  
  ; Call the appropriate kernel function
  call [rbx]
 
  ; At this point we returned from the C handler and RAX contains our syscall's result
ret_from_call:
  pop qword r15
  pop qword r14
  
  pop qword r11
  or r11, 0x200 ; Re-enable interrupts
  pop qword rcx
  mov bx, 0x23
  mov ds, bx

  pop qword r12
  pop qword r10
  mov rbp, r12
  mov rsp, r10

  o64 sysret

[global enter_userland_iret]
enter_userland_iret:
  mov rax, 0x23
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  push rax
  push rsi
  pushf
  or qword [rsp],0x200
  mov rax, 0x1b
  push rax
  push rdi
  iretq
