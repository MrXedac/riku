#include <stdint.h>

void puts(char* c)
{
	__asm volatile("MOV $0x1, %%RAX; \
			MOV %0, %%RBX; \
			SYSCALL;"
			:: "r"(c)
			: "rax", "rbx");

	return;
}

void main()
{
	puts("Michiru Operating System\n");
	puts("Based on Riku kernel\n");
	puts("---\n");
	puts("Successfully entered userland.\n");

	for(;;);
}
