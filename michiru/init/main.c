#include <stdint.h>

uint64_t strlen(const char * str)
{
	const char *s;
	for (s = str; *s; ++s) {}
	return(s - str);
}

void puts(char* c)
{
	uint64_t len = strlen(c);
	__asm volatile("MOV $0x5, %%RAX; \
			MOV $0x1, %%RBX; \
			MOV %0,	%%RDX; \
			MOV %1, %%RSI; \
			SYSCALL;"
			:: "r"(c), "r"(len)
			: "rax", "rbx", "rdx", "rsi");

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
