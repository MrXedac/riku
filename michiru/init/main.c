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
        
        printf("I'm the child process! Recompilation okay.\n");
    	printf("ANSI test : \n");
	printf("\tBlack: \033[30;40mBLACK\033[0m\n");
	printf("\tRed: \033[31;40mRED\033[0m\n");
	printf("\tGreen: \033[32;40mGREEN\033[0m\n");
	printf("\tYellow: \033[33;40mYELLOW\033[0m\n");
	printf("\tBlue: \033[34;40mBLUE\033[0m\n");
	printf("\tMagenta: \033[35;40mMAGENTA\033[0m\n");
	printf("\tCyan: \033[36;40mCYAN\033[0m\n");
	printf("\tWhite: \033[37;40mWHITE\033[0m\n");
	printf("Done. If we're alive until now, then everything is fine ~\n");
    printf("opening file...\n");
    int f = open("B:/motd", 0x0);
    printf("f=%d\n", f);
    char buffer[512]; /* Big bad buffer lol */
    read(f, buffer, 512);
    printf("%s", buffer);
    printf("\nBoot banner:\n");
    int f2 = open("B:/banner", 0x0);
    read(f2, buffer, 512);
    printf("%s", buffer);
    }
    for(;;);
}
