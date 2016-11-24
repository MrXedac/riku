#include <stdint.h>
#include "elf64.h"
#include "serial.h"
#include "idt64.h"

void dump_elf64_info(Elf64_Ehdr* hdr)
{
  Elf64_Shdr* sTable = (Elf64_Shdr*)((uintptr_t)hdr + (uintptr_t)(hdr->e_shoff));
  Elf64_Shdr* sNameTable = (Elf64_Shdr*)((uintptr_t)sTable + ((uintptr_t)hdr->e_shstidx * (uintptr_t)hdr->e_shentsize));
  Elf64_Shdr* sSymbolTable = 0x0;
  Elf64_Shdr* sStringTable = 0x0;
  Elf64_Shdr* sRelaTable = 0x0;
  unsigned char* sNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)sNameTable->sh_offset);
  unsigned char* sSymNameTableBegin = 0x0;

  KTRACE("\tSection name table should be around %x, having a %x size\n", sNameTable, sNameTable->sh_size);
  KTRACE("\tWe have %d section entries, starting at %x, with %x length per entry\n", hdr->e_shnum, sTable, hdr->e_shentsize);
  uint32_t sIndex = 0;
  while(sIndex < hdr->e_shnum)
  {
    KTRACE("\tSection %d, address %x, offset %x, size %x\n", sIndex, sTable->sh_addr, sTable->sh_offset, sTable->sh_size);
    unsigned char* sName = (unsigned char*)sNameTableBegin + sTable->sh_name;
    KTRACE("\t\tName is %s\n", sName);

    if(sTable->sh_type == 2) { /* SHT_SYMTAB */
      sSymbolTable = sTable;
      KTRACE("Found symbol table entry at %x\n", sSymbolTable);
    }

    if(sTable->sh_type == 3) { /* SHT_STRTAB */
      sStringTable = sTable;
      sSymNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)(sStringTable->sh_offset));
      KTRACE("Found string table entry at %x\n", sSymbolTable);
    }

    if(sTable->sh_type == 4 && !sRelaTable) { /* SHT_RELA */
      sRelaTable = sTable;
      KTRACE("Found relocation table entry at %x\n", sRelaTable);
    }

    if(sIndex == 0 && sTable->sh_addr != 0x0)
      panic("NULL section is not null", 0);

    sTable = (Elf64_Shdr*)((uintptr_t)sTable + (uintptr_t)(hdr->e_shentsize));
    sIndex++;
  }

  uint32_t sSymbolTableSize = sSymbolTable->sh_size / sizeof(Elf64_Sym);
  KTRACE("Symbol table contains %d entries\n", sSymbolTableSize);
  KTRACE("We shall refer to string table with section entry at %x\n", sStringTable);

  uint32_t sSymbolIdx = 0;
  Elf64_Sym* sSym = (Elf64_Sym*)((uintptr_t)hdr + (uintptr_t)(sSymbolTable->sh_offset));
  while(sSymbolIdx < sSymbolTableSize)
  {
    KTRACE("\tSymbol %d size %x\n", sSymbolIdx, sSym->st_size);
    unsigned char* sSymName = (unsigned char*)sSymNameTableBegin + sSym->st_name;
    KTRACE("\t\tSymbol name is %s\n", sSymName);
    sSym = (Elf64_Sym*)((uintptr_t)sSym + sizeof(Elf64_Sym));
    sSymbolIdx++;
  }

  uint32_t sRelaIdx = 0;
  uint32_t sRelaTableSize = sRelaTable->sh_size / sizeof(Elf64_Rela);
  Elf64_Rela* sRela = (Elf64_Rela*)((uintptr_t)hdr + (uintptr_t)sRelaTable->sh_offset);
  while(sRelaIdx < sRelaTableSize)
  {
    KTRACE("\tRelocation entry %d refers to symbol %d\n", sRelaIdx, (sRela->r_info >> 32));
    sSym = (Elf64_Sym*)((uintptr_t)hdr + (uintptr_t)(sSymbolTable->sh_offset) + (sRela->r_info >> 32) * sizeof(Elf64_Sym));
    unsigned char* sSymName = (unsigned char*)sSymNameTableBegin + sSym->st_name;
    KTRACE("\t\t'%s' has to be linked with kernel\n", sSymName);
    sRela = (Elf64_Rela*)((uintptr_t)sRela + sizeof(Elf64_Rela));
    sRelaIdx++;
  }
}
