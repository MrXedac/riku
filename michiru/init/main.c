#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SHELL_COMMANDS 7

typedef struct {
    char name[16];
    char desc[48];
    void (*ptr)(char* args);
} shell_command_t;

void ish_help(char*);
void ish_about(char*);
void ish_ansi(char*);
void ish_read(char*);
void ish_exit(char*);
void ish_crash(char*);
void ish_cd(char*);

/* Shell prompt */
int pid = -1;
char current_dir[64] = "B:/";
char* binpath;
int cfglen = 0;

shell_command_t commands[SHELL_COMMANDS] =
{
    {"help", "Displays help.", ish_help},
    {"about", "Displays about info about Riku.", ish_about},
    {"ansi", "Performs an ANSI test.", ish_ansi},
    {"read", "Reads a file.", ish_read},
    {"cd", "Switchs to another folder.", ish_cd},
    {"exit", "Exits shell.", ish_exit},
    {"crash", "Crashes shell by dereferencing a bad pointer.", ish_crash }
};

void ish_cd(char* args)
{
    if(args[1] == ':') {
        /* Absolute path */
        memset(current_dir, 0, 64);
        strcpy(current_dir, args);
    } else {
        int index = strlen(current_dir);
        int newpath = strlen(args);

        if(index + newpath >= 64) {
            /* Abort */
            printf("path too long\n");
            return;
        } else {
            strcpy(&current_dir[index], args);
        }
    }
    int len = strlen(current_dir);
    if(current_dir[len-1] != '/' )
    {
        current_dir[len] = '/';
        current_dir[len+1]= '\0';
    }
    
    cwd(current_dir);
}

void ish_crash(char* args)
{
    uint32_t* bad_pointer = (uint32_t*)0xDEADBEEF;
    *bad_pointer = 0xCAFEBABE;
}

void ish_exit(char* args)
{
    exit(0);
}

void ish_ansi(char* args)
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

void ish_read(char* args)
{
    char path[64];

    if(args[1] == ':') {
        /* Absolute path */
        memset(path, 0, 64);
        strcpy(path, args);
    } else {
        int index = strlen(current_dir);
        int newpath = strlen(args);

        if(index + newpath >= 64) {
            /* Abort */
            printf("path too long\n");
            return;
        } else {
            strcpy(path, current_dir);
            strcpy(&path[index], args);
        }
    }
    int f = open(path, 0x0);
    char content_buffer[512]; /* Big bad buffer lol */
    memset(content_buffer, 0x0, 512);
    read(f, content_buffer, 512);
    printf("%s", content_buffer);
}

void ish_help(char* args)
{
    int i;
    for(i=0;i<SHELL_COMMANDS; i++)
        printf("%s\t%s\n", commands[i].name, commands[i].desc);
}

void ish_about(char* args)
{
    printf("Riku Operating System\n");
    printf("Based on the Riku Kernel.\n");
    printf("\t(c)MrXedac, 2016-2024. All Rights Reserved.\n");
    printf("\nIn development stage.\n");
}

void spawn(char* name)
{
    char fullpath[128];
    int idx = 0;
    strcpy(fullpath, current_dir);
    idx += strlen(fullpath);

    int childpid = fork();
    if(childpid == 0)
    {
        /* Child : execve into child process */
        char* params[6];
        char* line = name;
        int count = 0;
        char c;
        int i = 0;
        do {
            c = name[i];

            if(c == ' '){
                name[i] = '\0';
                params[count] = line;

                line = name + i + 1;
                count++;
            }
            i++;
        } while(c != '\0');

        params[count] = line;
        params[count+1] = 0x0;

        strcpy(&fullpath[idx], params[0]);
        
        int ret = execve(fullpath, params, params);
        if(ret != 0)
        {
            if(ret == -10) 
            {
                /* Try binpath */
                char pathbuf[64];
                strcpy(pathbuf, binpath);
                int idx = strlen(pathbuf);
                strcpy(&pathbuf[idx], params[0]);
                ret = execve(pathbuf, params, params);
                if(ret != 0)
                {
                    printf("file not found or not an elf64 binary\n");
                }
            }
            exit(-1);
        }
            
    } else {
        int returncode = wait(childpid);
        if(returncode == -0x10) /* EBADPTR */
            printf("process killed by kernel (-EBADPTR)\n");
        else if(returncode > 0)
            printf("process exited with return code %d\n", returncode);
    };
}

char* get_config_value(char config[512], char* key)
{
    uint32_t index = 0;
    char c;
    /* Split config string into multiple strings, I guess */
    while(config[index] != '\0')
    {
        if(config[index] == '=' || config[index] == '\n') config[index] = '\0';
        index++;
    }

    index = 0;
    c = 'a';
    while(strcmp(&config[index], key) && index < cfglen){
        c = config[index];
        while(c != '\0')
        {
            c = config[index];
            index++;
        }
    }

    /* Found prompt block, go to next block */
    c = config[index];
    while(c != '\0')
    {
        c = config[index];
        index++;
    }

    if(index == cfglen) return (char*)0;

    return &config[index];
}

void make_prompt(char* prompt)
{
    uint32_t index_src = 0;
    uint32_t tagbufidx = 0;
    char tagbuffer[8];
    char cur = prompt[index_src];
    while(cur != '\0')
    {
        if(cur == '#') // Tag
        {
            memset(tagbuffer, 0, 8);
            index_src++;
            cur = prompt[index_src];
            tagbufidx = 0;
            while(cur != '#')
            {
                tagbuffer[tagbufidx] = cur;
                index_src++;
                tagbufidx++;
                cur = prompt[index_src];
            }
            tagbuffer[tagbufidx] = '\0';

            if(!strcmp(tagbuffer, "PID"))
            {
                printf("%d", pid);
            } 
            else if(!strcmp(tagbuffer, "PATH"))
            {
                printf("%s", current_dir);
            }
            index_src++;
        } else {
            printf("%c", cur);
            index_src++;
        }

        cur = prompt[index_src];
    }
}

void shell()
{
    /* Get prompt */
    int fcfg = open("B:/rshrc", 0x0);
    char prompt[64];
    pid = getpid();

    if(fcfg > 0)
    {
        char content_buffer[512]; /* Big bad buffer lol */
        memset(content_buffer, 0x0, 512);
        read(fcfg, content_buffer, 512);
        strcpy(prompt, content_buffer);
        close(fcfg);
        cfglen = strlen(content_buffer);
        char* promptPtr = get_config_value(content_buffer, "PROMPT");
        binpath = get_config_value(content_buffer, "BINPATH");
        if(promptPtr > 0)
        {
            strcpy(prompt, promptPtr);
        } else {
            strcpy(prompt, "rsh>");
        }
    } else {
        strcpy(prompt, "rsh_bare>" );
        strcpy(binpath, "B:/");
    }

    
    char buffer[128];
    char cmdbuf[128];
    int idx = 0;
    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        idx = 0;
        make_prompt(prompt);

        char c = 0;

        /* Read input */
        while(c != '\n')
        {
            read(0, &c, 1);
            if(c)
            {
                if(c == '\b')
                {
                    if(idx > 0) {
                        /* backspace */
                        printf("\b \b");
                        idx--;
                        buffer[idx] = (char)0; 
                    }
                } else {
                    printf("%c", c);
                    if(c != '\n') {
                        buffer[idx] = c;
                        idx++;
                    }   
                }
            }
        }
        int i;
        memcpy(cmdbuf, buffer, 128);
        for(i = 0; i < SHELL_COMMANDS; i++)
        {
            int j = 0;
            char c = cmdbuf[j];
            while(c != '\0')
            {
                if(c == ' ') {
                    cmdbuf[j] = '\0';
                    break;
                }
                j++;
                c = cmdbuf[j];
            }
            
            if(!strcmp(cmdbuf, commands[i].name)) {
                commands[i].ptr(&cmdbuf[j+1]);
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

    cwd(current_dir);
    shell();
    exit(0);
}
