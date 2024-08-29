#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main()
{
    struct riku_fileinfo dir, file;

    if(opendir("B:/", &dir) == 0)
    {
        int ret;
        printf("name\ttype\tsize (bytes)\n-------------------------\n");
        while(ret != 0x8) // ENMFIL
        {
            ret = readdir(&dir, 0, &file);
            if(ret == 0x8) break;

            if(file.type == 0x2) {
                printf("\033[32;40m%s\033[0m\t", file.name);
                printf("exec\t");
            }
            else {
                printf("%s\t", file.name);
                printf("plain\t");
            }
            printf("%do\n", file.size);
        }

        printf("\n");
    }

    exit(0);
}
