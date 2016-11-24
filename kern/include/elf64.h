/**
 * ELF64 header file for modules and binaries
 */

#ifndef __ELF64__
#define __ELF64__

#include <stdint.h>

/* ELF64 types */
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_SWord;
typedef uint64_t Elf64_XWord;
typedef int64_t Elf64_SXWord;
typedef uint8_t Elf64_UChar;

/* ELF64 header structure */
struct Elf64_FileHeader {
  Elf64_UChar e_ident[16];
  Elf64_Half  e_type;
  Elf64_Half  e_machine;
  Elf64_Word  e_version;
  Elf64_Addr  e_entry;
  Elf64_Off   e_phoff;
  Elf64_Off   e_shoff;
  Elf64_Word  e_flags;
  Elf64_Half  e_ehsize;
  Elf64_Half  e_phentsize;
  Elf64_Half  e_phnum;
  Elf64_Half  e_shentsize;
  Elf64_Half  e_shnum;
  Elf64_Half  e_shstidx;
} __attribute__((packed));

/* ELF64 section header structure */
struct Elf64_SectionHeader {
  Elf64_Word  sh_name; /* Offset in bytes from the section name table */
  Elf64_Word  sh_type;
  Elf64_XWord sh_flags;
  Elf64_Addr  sh_addr;
  Elf64_Off   sh_offset;
  Elf64_XWord sh_size;
  Elf64_Word  sh_link;
  Elf64_Word  sh_info;
  Elf64_XWord sh_addralign;
  Elf64_XWord sh_entsize;
} __attribute__((packed));

/* ELF64 symbol header structure */
struct Elf64_SymbolHeader {
  Elf64_Word  st_name;
  Elf64_UChar st_info;
  Elf64_UChar st_other;
  Elf64_Half  st_shndx;
  Elf64_Addr  st_value;
  Elf64_XWord st_size;
};

struct Elf64_RelocationHeader {
  Elf64_Addr    r_offset;
  Elf64_XWord   r_info;
  Elf64_SXWord  r_addend;
};

typedef struct Elf64_FileHeader Elf64_Ehdr;
typedef struct Elf64_SectionHeader Elf64_Shdr;
typedef struct Elf64_SymbolHeader Elf64_Sym;
typedef struct Elf64_RelocationHeader Elf64_Rela;
void dump_elf64_info(Elf64_Ehdr* hdr);

#endif
