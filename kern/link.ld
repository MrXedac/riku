OUTPUT_FORMAT("elf64-x86-64")
ENTRY(start)
KERNEL_VMA = 0xFFFF800000300000;
SECTIONS
{
	. = KERNEL_VMA;
	.text : AT(ADDR(.text) - KERNEL_VMA)
	{
		code = .; _code = .; __code = .;
		*(.text)
		*(.rodata*)
		. = ALIGN(4096);
	}
	.data : AT(ADDR(.data) - KERNEL_VMA)
	{
		data = .; _data = .; __data = .;
		*(.data)
		. = ALIGN(4096);
	}
	.eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA)
	{
		_ehframe = .;
		*(.eh_frame)
		. = ALIGN(4096);
	}
	.bss : AT(ADDR(.bss) - KERNEL_VMA)
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4096);
	}
	.stack :
	{
		stack = .; _stack = .; __stack = .;
		*(.stack)
		. = ALIGN(4096);
	}
	end = .; _end = .; __end = .;
	/DISCARD/ :
	{
		*(.comment)
	}
}
