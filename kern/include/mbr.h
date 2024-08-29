#ifndef __MBR__
#define __MBR__

#include <stdint.h>


struct mbr_partition_entry {
    uint8_t attributes;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t sector_count;
}__attribute__((packed));

struct mbr_header {
    uint8_t bootstrap[440];
    uint32_t signature;
    uint16_t reserved;
    struct mbr_partition_entry first_entry;
    struct mbr_partition_entry second_entry;
    struct mbr_partition_entry third_entry;
    struct mbr_partition_entry fourth_entry;
    uint16_t bootsector_signature;
}__attribute__((packed));

#endif