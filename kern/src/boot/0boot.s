; x86 compatibility mode right now, but not for long
[BITS 64]

[GLOBAL start]
[EXTERN main]
ALIGN 4

SECTION .text
; x64 bootstrap code
start:
	cli						; Clear interrupts
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov rax, main				; Prepare to jump to higher half !
	mov rdx, 0xFFFF800000000000
	or rax, rdx	; OR with the higher half offset
	call rax					; Absolute call to main
	jmp $
	
SECTION .stack
	times 8192 db 0		; Allocate a 8kb early-boot stack. This should be more than enough.
	_earlystack:
