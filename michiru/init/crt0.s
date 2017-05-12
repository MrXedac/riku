; Fake crt0.s for init process
; Do nothing right now.

[global start]
[extern main]
start:
	; Perform system call
;	mov rbx, hw
;	mov rax, 0x1
;	syscall
	call main
	jmp $

;hw:	db	'Hello world\n\0'
