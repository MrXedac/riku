#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void main()
{
	printf("Michiru Operating System\n");
	printf("Based on Riku kernel\n");
	printf("---\n");
	printf("Successfully entered userland.\n");
    printf("Testing stuff: %x\n", 0xDEADBEEFCAFEBABE);

    	int coucou = 4;
	printf("Var=%d\n", coucou);
	int truc = fork();
	printf("Forked! PID=%d\n", truc);
	int* ptr = &coucou;
	*ptr = 5;
	int* ptr2 = (int*)0x100000;
	*ptr2 = 0x32;
	printf("Var=%d\n", coucou);
	for(;;);
}
