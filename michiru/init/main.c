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
	if(truc)
    {
        printf("I'm the parent process, my child has PID %d.\n", truc);
    } else {
        printf("I'm the child process!\n");
    }
	
    for(;;);
}
