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

; Replaces the default VGA font with the builtin font - classy Riku!
[EXTERN __font_bitmap__]
[GLOBAL fixvga]
fixvga:
        mov edi,    0A0000h
		;clear even/odd mode
		mov			dx, 03ceh
		mov			ax, 5
		out			dx, ax
		;map VGA memory to 0A0000h
		mov			ax, 0406h
		out			dx, ax
		;set bitplane 2
		mov			dx, 03c4h
		mov			ax, 0402h
		out			dx, ax
		;clear even/odd mode
		mov			ax, 0604h
		out			dx, ax
		;copy charmap
		mov			esi, __font_bitmap__
		mov			ecx, 256
		;copy 16 bytes to bitmap
vgaloop:		
        movsd
		movsd
		movsd
		movsd
		;skip another 16 bytes
		add			edi, 16
		loop	    vgaloop
		;restore VGA state to normal operation
		mov			ax, 0302h
		out			dx, ax
		mov			ax, 0204h
		out			dx, ax
		mov			dx, 03ceh
		mov			ax, 1005h
		out			dx, ax
		mov			ax, 0E06h
		out			dx, ax
        ret

SECTION .stack
	times 8192 db 0		; Allocate a 8kb early-boot stack. This should be more than enough.
	_earlystack:
