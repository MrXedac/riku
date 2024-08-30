#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char** argv)
{
    if(argc <= 1)
    {
        printf("cat: no file provided\n");
        exit(-1);
    }

    int f = open(argv[1], 0x0);
    char content_buffer[512]; /* Big bad buffer lol */
    memset(content_buffer, 0x0, 512);
    read(f, content_buffer, 512);
    printf("%s", content_buffer);
    close(f);
    exit(0);
}
