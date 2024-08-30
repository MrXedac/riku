; Fake crt0.s for init process
; Do nothing right now.

[global start]
[extern main]
start:
	; Perform system call
;	mov rbx, hw
;	mov rax, 0x1
;	syscall
	mov rdi, r12
	mov rsi, r13
	call main
	jmp $

;hw:	db	'Hello world\n\0'
