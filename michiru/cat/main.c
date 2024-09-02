#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char** argv)
{
    char path[64];

    if(argc <= 1)
    {
        printf("cat: no file provided\n");
        exit(-1);
    }

    if(argv[1][1] == ':')
    {
        strcpy(path, argv[1]);
    } else {
        gwd(path);
        int idx = strlen(path);
        strcpy(&path[idx], argv[1]);
    }

    int f = open(path, 0x0);
    char content_buffer[512]; /* Big bad buffer lol */
    memset(content_buffer, 0x0, 512);
    read(f, content_buffer, 512);
    printf("%s", content_buffer);
    close(f);
    exit(0);
}
