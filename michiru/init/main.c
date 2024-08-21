#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHELL_COMMANDS 3

typedef struct {
    char name[16];
    char desc[48];
    void (*ptr)(void);
} shell_command_t;

void ish_help();
void ish_about();
void ish_coucou();

shell_command_t commands[SHELL_COMMANDS] =
{
    {"help", "Displays help.", ish_help},
    {"about", "Displays about info about Riku.", ish_about},
    {"coucou", "Displays the COUCOU.", ish_coucou}
};

void ish_help()
{
    int i;
    for(i=0;i<SHELL_COMMANDS; i++)
        printf("%s\t%s\n", commands[i].name, commands[i].desc);
}

void ish_coucou()
{
	printf("coucou le monde\n");
}

void ish_about()
{
    printf("Michiru Operating System\n");
    printf("Based on the Riku Kernel.\n");
    printf("\t(c)MrXedac, 2016-2019. All Rights Reserved.\n");
    printf("\nIn development stage.\n");
}


void shell()
{
    char buffer[32];
    int idx = 0;
    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        idx = 0;
        printf("michiru>");
        char c = 0;

        /* Read input */
        while(c != '\n')
        {
            read(0, &c, 1);
            if(c)
            {
                printf("%c", c);
                if(c != '\n') {
                    buffer[idx] = c;
                    idx++;
                }   
            }
        }
        int i;
        for(i = 0; i < SHELL_COMMANDS; i++)
        {
            if(!strcmp(buffer, commands[i].name)) {
                commands[i].ptr();
               break;
                
            }
        }
        if(i == SHELL_COMMANDS)
            printf("Unknown command '%s'.\n", buffer);
    }
}

void main()
{
    printf("Michiru Operating System\n");
    printf("Based on Riku kernel\n");
    printf("---\n");
    printf("Successfully entered userland.\n");

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

        int childPid = fork();
        if(childPid == 0)
        {
            printf("Processus enfant. Lancement du shell.\n");
            shell();
        } else {
            printf("Processus parent. yolo\n");
        }
    for(;;);
}
