[BITS 32]
GDT64:                           ; Global Descriptor Table (64-bit).
	.Null: equ $ - GDT64         ; The null descriptor.
	dw 0                         ; Limit (low).
	dw 0                         ; Base (low).
	db 0                         ; Base (middle)
	db 0                         ; Access.
	db 0                         ; Granularity.
	db 0                         ; Base (high).
	.Code: equ $ - GDT64         ; The code descriptor.
	dw 0
	dw 0
	db 0
	db 10011010b                 ; Access (exec/read).
	db 00100000b
	db 0
	.Data: equ $ - GDT64         ; The data descriptor.
	dw 0
	dw 0
	db 0
	db 10010010b                 ; Access (read/write).
	db 00000000b
	db 0
	.UserCode: equ $ - GDT64         ; The code descriptor.
	dw 0
	dw 0
	db 0
	db 11111010b                 ; Access (exec/read).
	db 00100000b
	db 0
	.UserData: equ $ - GDT64         ; The data descriptor.
	dw 0
	dw 0
	db 0
	db 11110010b                 ; Access (read/write).
	db 00000000b
	db 0
	.TSS: equ $ - GDT64			; 16-bytes TSS entry
	dw 0
	dw 0
	db 0
	db 11101001b
	db 00000000b
	db 0
	dw 0
	dw 0
	dw 0
	dw 0
GDT64Pointer:                  ; The GDT-pointer.
dw $ - GDT64 - 1
dd GDT64

; Long-mode switch
[GLOBAL enter_long_mode]
[EXTERN kernel]
enter_long_mode:
	; Set CR3 to PML4T address
	mov edi, 0x1000 ; PML4T address
	mov cr3, edi ; Write address into CR3

	; Enable PAE
	mov eax, cr4		; Get CR4 value
	or eax, 1 << 5		; Set the PAE bit
	or eax, 0x00000010	; Enable large page support
	mov cr4, eax		; Write back CR4

	; Write long mode bit
	mov ecx, 0xC0000080 ; EFER MSR
	rdmsr				; Read MSR
	or eax, 1 << 8		; Set the long mode bit
	wrmsr				; Write back MSR

	; Enable paging
	mov eax, cr0		; Get CR0 value
	or eax, 1 << 31		; Set the paging bit
	mov cr0, eax		; Write back CR0

	; We're in x86 compatibility mode. This is exactly what we need : jump to the kernel's compatibility entry-point !
	lgdt [GDT64Pointer]		; Load x64 GDT
	jmp GDT64.Code:kernel	; Kernel's entrypoint
