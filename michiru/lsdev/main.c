#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
    case HIDDevice:
      return 0x1;
      break;
    case StorageDevice:
      return 0x2;
      break;
    case NetworkDevice:
      return 0x3;
      break;
    case SpecialDevice:
      return 0x4;
      break;  
    case UnknownDevice:
    default:
      return 0x5;
      break;
      */

void main()
{
    struct riku_fileinfo dir, file;

    if(opendir("A:/", &dir) == 0)
    {
        int ret;
        printf("name\ttype\t\n-------------------------\n");
        while(ret != 0x8) // ENMFIL
        {
            ret = readdir(&dir, 0, &file);
            if(ret == 0x8) break;

            printf("\033[36;40m%s\033[0m\t", file.name);

            switch(file.type)
            {
                case 0x1:
                    printf("hid\n");
                    break;
                case 0x2:
                    printf("storage\n");
                    break;
                case 0x3:
                    printf("network\n");
                    break;
                case 0x4:
                    printf("special\n");
                    break;
                case 0x5:
                default:
                    printf("unknown\n");
                    break;
            }
        }

    }

    exit(0);
}
