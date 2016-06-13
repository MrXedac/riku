; Generic fault handler, without any error code pushed
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

; Generic fault handler, with an error code pushed
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; Generic IRQ handler
%macro IRQ_STUB 2
	global irq%1
	irq%1:
		push byte 0
		push byte %2
		jmp irq_common_stub
%endmacro

; PUSHA/POPA disappeared on x64. Simulate them, our own way (rsp first)
%macro PUSHA 0
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rax
	push rcx
	push rdx
	push rbx
	push rsp
	push rbp
	push rsi
	push rdi
%endmacro

%macro POPA 0
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro

[EXTERN irq_handler]
[EXTERN isr_handler]
[GLOBAL isr_common_stub]
[GLOBAL irq_common_stub]

irq_common_stub:
	cli
    PUSHA

    mov ax, ds
    push rax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	mov rdi, rsp ; Our regs structure is on RSP
    call irq_handler

    pop rbx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    POPA
    add rsp, 16
	sti
    iretq

isr_common_stub:
	cli
	PUSHA

    mov ax, ds
    push rax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

	mov rdi, rsp ; Our regs structure is on RSP
    call isr_handler

    pop rbx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    POPA
    add rsp, 16
	sti
    iretq

[GLOBAL idt_flush]
idt_flush:
    mov rax, rdi; Get the IDT from RDI (parameter)
    lidt [rax] ; Load the IDT !
    ret

; Definition of each interrupt handler for x86/x64 (0-31 : faults, 32-47 : IRQ)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31
IRQ_STUB 0 , 32
IRQ_STUB 1 , 33
IRQ_STUB 2 , 34
IRQ_STUB 3 , 35
IRQ_STUB 4 , 36
IRQ_STUB 5 , 37
IRQ_STUB 6 , 38
IRQ_STUB 7 , 39
IRQ_STUB 8 , 40
IRQ_STUB 9 , 41
IRQ_STUB 10 , 42
IRQ_STUB 11 , 43
IRQ_STUB 12 , 44
IRQ_STUB 13 , 45
IRQ_STUB 14 , 46
IRQ_STUB 15 , 47
ISR_NOERRCODE 254