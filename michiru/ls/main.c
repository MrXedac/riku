#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char** argv)
{
    struct riku_fileinfo dir, file;

    int opendir_result;
    if(argc > 1)
        opendir_result = opendir(argv[1], &dir);
    else 
        opendir_result = opendir("B:/", &dir);

    if(opendir_result == 0)
    {
        int ret;
        printf("contents of directory %s\n", dir.name);

        printf("name\ttype\tsize (bytes)\n-------------------------\n");
        while(ret != -0x8) // ENMFIL
        {
            ret = readdir(&dir, 0, &file);
            if(ret == -0x8) break;

            if((file.type & 0x1) == 0x1) {
                printf("%s\t", file.name);
                printf("file\t");
            }
            else if ((file.type & 0x2) == 0x2){
                printf("\033[32;40m%s\033[0m\t", file.name);
                printf("dir\t");
            }
            else if ((file.type & 0x4) == 0x4){
                printf("\033[36;40m%s\033[0m\t", file.name);
                printf("device\t");
            }
            printf("%db\n", file.size);
        }

        printf("\n");
    }

    exit(0);
}
