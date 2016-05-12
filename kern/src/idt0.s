; IDT stubs

[GLOBAL irq_handle]
[GLOBAL misr_handle]

irq_handle:
	iretq
	
isr_handle:
	iretq
