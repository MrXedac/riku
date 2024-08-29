#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SHELL_COMMANDS 5

typedef struct {
    char name[16];
    char desc[48];
    void (*ptr)(void);
} shell_command_t;

void ish_help();
void ish_about();
void ish_ansi();
void ish_read();
void ish_exit();

shell_command_t commands[SHELL_COMMANDS] =
{
    {"help", "Displays help.", ish_help},
    {"about", "Displays about info about Riku.", ish_about},
    {"ansi", "Performs an ANSI test.", ish_ansi},
    {"read", "Reads a file.", ish_read},
    {"exit", "Exits shell.", ish_exit}
};

void ish_exit()
{
    exit(0);
}

void ish_ansi()
{
    printf("ANSI test : \n");
    printf("\tBlack: \033[30;40mBLACK\033[0m\n");
    printf("\tRed: \033[31;40mRED\033[0m\n");
    printf("\tGreen: \033[32;40mGREEN\033[0m\n");
    printf("\tYellow: \033[33;40mYELLOW\033[0m\n");
    printf("\tBlue: \033[34;40mBLUE\033[0m\n");
    printf("\tMagenta: \033[35;40mMAGENTA\033[0m\n");
    printf("\tCyan: \033[36;40mCYAN\033[0m\n");
    printf("\tWhite: \033[37;40mWHITE\033[0m\n");
    printf("Done. If we're alive until now, then everything is fine.\n");
}

void ish_read()
{
    char buffer[32];
    int idx = 0;
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, "B:/", 3);
    idx = 3;
    printf("filename>");
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
    buffer[idx]='\0';

    printf("opening file...\n");
    int f = open(buffer, 0x0);
    printf("f=%d\n", f);
    char content_buffer[512]; /* Big bad buffer lol */
    memset(content_buffer, 0x0, 512);
    read(f, content_buffer, 512);
    printf("%s", content_buffer);
}

void ish_help()
{
    int i;
    for(i=0;i<SHELL_COMMANDS; i++)
        printf("%s\t%s\n", commands[i].name, commands[i].desc);
}

void ish_about()
{
    printf("Riku Operating System\n");
    printf("Based on the Riku Kernel.\n");
    printf("\t(c)MrXedac, 2016-2024. All Rights Reserved.\n");
    printf("\nIn development stage.\n");
}

void spawn(char* name)
{
    int childpid = fork();
    if(childpid == 0)
    {
        /* Child : execve into child process */
        int ret = execve(name, (char**)0, (char**)0);
        if(ret != 0)
        {
            if(ret == -10) printf("file is not an executable binary\n");
            exit(-1);
        }
            
    } else {
        int returncode = wait(childpid);
        printf("process exited with return code %d\n", returncode);
    };
}

void shell()
{
    char buffer[32];
    int idx = 0;
    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        idx = 0;
        printf("rsh>");
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
            spawn(buffer);
    }
}

void main()
{
    printf("rsh/riku shell\n");
    printf("ver0.1\n");
    printf("---\n");

    shell();
    exit(0);
}
