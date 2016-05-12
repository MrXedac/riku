; gdt.s : assembly routines for GDT management
global gdt_flush
extern gp
gdt_flush:
	lgdt [edi]
	ret

[GLOBAL tss_flush]
tss_flush:
	mov ax, 0x2B ; 0x28 = GDT TSS entry + 3 = Ring-level 3
	ltr ax
	ret