[BITS 32]

[GLOBAL start]
[GLOBAL mboot]
[EXTERN main]
ALIGN 4

SECTION .text
; Multiboot2 header
mboot:
	MBOOT_HEADER_MAGIC	equ	0xe85250d6
	MBOOT_ARCH_I386		equ	0x0
	MBOOT_SIZE			equ	multiboot_header_end - mboot
	MBOOT_CHECKSUM		equ	-(MBOOT_HEADER_MAGIC + MBOOT_ARCH_I386 + multiboot_header_end - mboot)

	EXTERN code, bss, end

	dd  MBOOT_HEADER_MAGIC
	dd  MBOOT_ARCH_I386
	dd  MBOOT_SIZE
	dd  MBOOT_CHECKSUM
	dd  mboot
	dd  code
	dd  bss
	dd  end
	dd  start
multiboot_header_end:

; x86 bootstrap code
start:
	cli			; Clear interrupts
	mov esp, _earlystack	; Setup early-boot stack
	push ebx		; Push multiboot2 info to main()
	call main		; Call main()
	jmp $			; We shouldn't return from main. If so, loop forever.

SECTION .stack
	times 8192 db 0		; Allocate a 8kb early-boot stack. This should be more than enough.
	_earlystack:
