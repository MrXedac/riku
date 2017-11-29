#include "fs_vfat.h"
#include "printk.h"
#include "heap.h"
#include <stdint.h>
#include "mem.h"

/* Reads FAT header from Ramdrive */
void vfat_readHeader(uintptr_t* rd)
{
    printk("Parsing FAT header at %x.\n", rd);
    /* Generic Fat header */
    fat_BS_t* fat_boot = (fat_BS_t*)rd; 

    /* Fat32 extended boot structure */
    fat_extBS_32_t* fat_boot_ext_32 = (fat_extBS_32_t*)(fat_boot->extended_section);
    printk("Extended FAT structure at %x.\n", fat_boot_ext_32);

    /* Fat size */
    uint32_t fat_size = (fat_boot->table_size_16 == 0)? fat_boot_ext_32->table_size_32 : fat_boot->table_size_16;
    printk("FAT size: %x\n", fat_size);
    
    /* Sum stuff */
    uint32_t root_dir_sectors = ((fat_boot->root_entry_count * 32) + (fat_boot->bytes_per_sector - 1)) / fat_boot->bytes_per_sector;
    uint32_t first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors;
    printk("Root directory sectors: %x\n", root_dir_sectors);
    uint32_t data_sectors = fat_boot->total_sectors_16 - (fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors);
    uint32_t total_clusters = data_sectors / fat_boot->sectors_per_cluster;

    /* Compute Fat filesystem variant */
    if(total_clusters < 4085)
    {
        printk("Ramdrive is FAT12 filesystem\n");
        uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
        unsigned char *entry = (unsigned char*)((uintptr_t)rd + first_root_dir_sector * fat_boot->bytes_per_sector);
//        printk("First entry probably around %x.\n", entry);
        /* Parse each entry */
        while(*entry)
        {
/*             printk("Found entry, byte: %x\n", *entry); */
            if(*entry == 0xE5)
            {
                /* Unused entry */
            } else {
                if(*(entry + 11) == 0x0F)
                {
                    printk("Long file name entry\n");
                } else {
                   fat_dirent_t* dirent = (fat_dirent_t*)entry;
                   char nm[12];
                   memcpy(nm, dirent->name, 11*sizeof(char));
                   nm[11] = '\0';
                   printk("Found file %s\n",
                           nm);
                   if(*(entry + 11) & 0x10)
                   {
                        printk("-> Directory entry, parsing again\n");
                        /* TODO : refactor all of this */
                   }
                }
            }  
            entry += 32;
        }
    }
    else if(total_clusters < 65525)
    {
        printk("Ramdrive is FAT16 filesystem\n");
    }
    else if (total_clusters < 268435445)
    {
        printk("Ramdrive is FAT32 filesystem\n");
    }
    else
    {
        printk("Ramdrive is exFAT filesystem\n");
    }
}
