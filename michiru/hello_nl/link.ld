OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)
phys = 0x00100000;
SECTIONS
{
	.text phys : AT(phys)
	{
		code = .; _code = .; __code = .;
		*(.text)
		. = ALIGN(4096);
	}
	.data :
	{
		data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}
	.bss :
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}
	end = .; _end = .; __end = .;
}
