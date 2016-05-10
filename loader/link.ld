OUTPUT_FORMAT("elf32-i386")
ENTRY(start)
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
	.stack :
	{
		stack = .; _stack = .; __stack = .;
		*(.stack)
		. = ALIGN(4096);
	}
	.kernel 0x200000 :
	{
		kernel = .; _kernel = .; __kernel = .;
		*(.kernel)
		. = ALIGN(4096);
	}
	end = .; _end = .; __end = .;
}
