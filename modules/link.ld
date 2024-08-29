ENTRY(module_init)
SECTIONS
{
	.text :
	{
		*(.text*)
		*(.rodata)
	}
	.data :
	{
		*(.data*)
	}
	.bss :
	{
		*(.bss*)
	}
}
