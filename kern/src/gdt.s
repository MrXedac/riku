; gdt.s : assembly routines for GDT management
global gdt_flush
extern gp
gdt_flush:
	lgdt [edi]
	ret