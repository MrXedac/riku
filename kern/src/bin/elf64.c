#include <stdint.h>
#include "elf64.h"
#include "serial.h"
#include "idt64.h"
#include "vga.h"
#include "string.h"

Elf64_Shdr* elf64_get_section(Elf64_Ehdr* hdr, Elf64_Section type)
{
	uint32_t sIndex = 0;

	/* Section header */
	Elf64_Shdr* sTable = (Elf64_Shdr*)((uintptr_t)hdr + (uintptr_t)(hdr->e_shoff));
	
	/* Section name table */
	Elf64_Shdr* sNameTable = (Elf64_Shdr*)((uintptr_t)sTable + ((uintptr_t)hdr->e_shstidx * (uintptr_t)hdr->e_shentsize));
	unsigned char* sNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)sNameTable->sh_offset);	
	
	while(sIndex < hdr->e_shnum)
	{
		unsigned char* sName = (unsigned char*)sNameTableBegin + sTable->sh_name;
		if(strcmp((const char*)sName, Elf64_SectionNames[type]) == 0)
		{
			KTRACE("Yay! Found %s on section %d, addr %x\n", Elf64_SectionNames[type], sIndex, sTable);
			return sTable;
		}
		sTable = (Elf64_Shdr*)((uintptr_t)sTable + (uintptr_t)(hdr->e_shentsize));
		sIndex++;
	}
	KTRACE("Didn't found section %s...\n", Elf64_SectionNames[type]);
	return 0x0;
}

Elf64_Sym* elf64_get_symbol(Elf64_Ehdr* hdr, Elf64_Shdr* symtable, uint32_t idx)
{
	//KTRACE("elf64: get_symbol, %x, %x\n", symtable, idx);
	return (Elf64_Sym*)((uintptr_t)hdr + (uintptr_t)(symtable->sh_offset) + idx * sizeof(Elf64_Sym));
}

unsigned char* elf64_get_symbol_name(Elf64_Ehdr* hdr, Elf64_Shdr* strtable, Elf64_Sym* symbol)
{
	//KTRACE("elf64: get_symbol_name, %x, %x\n", strtable, symbol);
	unsigned char* sSymNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)(strtable->sh_offset));
	unsigned char* sSymName = (unsigned char*)sSymNameTableBegin + symbol->st_name;
	return sSymName;
}

uintptr_t elf64_find_symbol(Elf64_Ehdr* hdr, const char* symname)
{
	Elf64_Shdr* sTable = (Elf64_Shdr*)((uintptr_t)hdr + (uintptr_t)(hdr->e_shoff));
	Elf64_Shdr* sNameTable = (Elf64_Shdr*)((uintptr_t)sTable + ((uintptr_t)hdr->e_shstidx * (uintptr_t)hdr->e_shentsize));
	
	/* Interesting sections */
	Elf64_Shdr* sSymbolTable = elf64_get_section(hdr, SECTION_SYM);
	Elf64_Shdr* sStringTable = elf64_get_section(hdr, SECTION_STR);
	
	/* Amount of symbols in kernel (ie. A LOT) */
	uint32_t sSymbolTableSize = sSymbolTable->sh_size / sizeof(Elf64_Sym);
	KTRACE("elf64_find_symbol: worst case, going through %d entries\n", sSymbolTableSize);
	uint32_t sSymbolIdx = 0;
	while(sSymbolIdx < sSymbolTableSize)
	{
		Elf64_Sym* sym = elf64_get_symbol(hdr, sSymbolTable, sSymbolIdx);
		unsigned char* ksymname = elf64_get_symbol_name(hdr, sStringTable, sym);
		if(strcmp((const char*)ksymname, symname) == 0)
		{
			KTRACE("elf64_kernel_symbol_addr: found symbol %s at %x\n", symname, sym->st_value);
			return (uintptr_t)sym->st_value;
		} 
		sSymbolIdx++;
	}
	KTRACE("elf64_find_symbol: did not found symbol address :( this is sad\n");

	return 0x0;
}

void elf64_load_module(Elf64_Ehdr* hdr)
{
	/* Retrieve interesting sections in ELF */
  Elf64_Shdr* sTable = (Elf64_Shdr*)((uintptr_t)hdr + (uintptr_t)(hdr->e_shoff));
  Elf64_Shdr* sNameTable = (Elf64_Shdr*)((uintptr_t)sTable + ((uintptr_t)hdr->e_shstidx * (uintptr_t)hdr->e_shentsize));
  Elf64_Shdr* sSymbolTable = elf64_get_section(hdr, SECTION_DYNSYM);
  Elf64_Shdr* sStringTable = elf64_get_section(hdr, SECTION_DYNSTR);
  unsigned char* sSymNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)(sStringTable->sh_offset));
  Elf64_Shdr* sRelaTable = elf64_get_section(hdr, SECTION_RELA_PLT);
  Elf64_Shdr* sText = elf64_get_section(hdr, SECTION_TEXT);
  unsigned char* sNameTableBegin = (unsigned char*)((uintptr_t)hdr + (uintptr_t)sNameTable->sh_offset);

  KTRACE("\tSection name table should be around %x, having a %x size\n", sNameTable, sNameTable->sh_size);
  KTRACE("\tWe have %d section entries, starting at %x, with %x length per entry\n", hdr->e_shnum, sTable, hdr->e_shentsize);
  uint32_t sIndex = 0;
  Elf64_Sym* sSym = (Elf64_Sym*)((uintptr_t)hdr + (uintptr_t)(sSymbolTable->sh_offset));
  
  uint32_t sSymbolTableSize = sSymbolTable->sh_size / sizeof(Elf64_Sym);
  KTRACE("Symbol table contains %d entries\n", sSymbolTableSize);
  KTRACE("We shall refer to string table with section entry at %x\n", sStringTable);

  uint32_t sSymbolIdx = 0;
  
  uint32_t sRelaIdx = 0;
  uint32_t sRelaTableSize = sRelaTable->sh_size / sizeof(Elf64_Rela);
  Elf64_Rela* sRela = (Elf64_Rela*)((uintptr_t)hdr + (uintptr_t)sRelaTable->sh_offset);
  
  /* Time to do the dynamic linking thing */
  while(sRelaIdx < sRelaTableSize)
  {
	/* Find symbol and name */
    KTRACE("\tRelocation entry %d refers to symbol %d\n", sRelaIdx, (sRela->r_info >> 32));
	sSym = elf64_get_symbol(hdr, sSymbolTable, (sRela->r_info) >> 32);
	unsigned char* symname = elf64_get_symbol_name(hdr, sStringTable, sSym);
	if(sSym->st_value == 0x0) {
		/* Find linked symbol in kernel */
		KTRACE("Searching symbol %s in kernel\n", symname);
		uintptr_t kernsym = elf64_kernel_symbol_addr((const char*)symname);
		if(!kernsym)
		{
			KTRACE("Whoops! Kernel module linked against invalid symbol : '%s'\n", symname);
			return;
		}
		
		/* Now that we found the symbol, link it into our module */
		KTRACE("Symbol in GOT should be around %x\n", sRela->r_offset);
		uintptr_t dest = (uintptr_t)hdr + (uintptr_t)sRela->r_offset;
		*(uintptr_t*)dest = kernsym;
		
		/* Some user output */
		KTRACE("Relocated symbol %s to %x\n", symname, *(uintptr_t*)dest);	
	} else {
		KTRACE("\tSymbol didn't need relocation, locating into binary\n");
		uintptr_t dest = (uintptr_t)hdr + (uintptr_t)sRela->r_offset;
		*(uintptr_t*)dest = (uintptr_t)hdr + (uintptr_t)sSym->st_value;
	}
		
	sRela = (Elf64_Rela*)((uintptr_t)sRela + sizeof(Elf64_Rela));
	sRelaIdx++;
  }

  /* Now that everything has been linked, run the module */
  KTRACE("Preparing to run module\n");
  
  /* First we have to find a specific symbol in our module : module_init */
  uintptr_t sModEP = (uintptr_t)hdr + elf64_find_symbol(hdr, "module_init");
  void (*modEP)(void)  = (void*)sModEP;
  KTRACE("Module EP=%x\n", sModEP);
  
  /* Find module information in binary */
  /* TODO : This -0x1000 thing is due to some misplacement in the module binary.
   * Once we have proper ELF-loading with appropriate tasks, placements in virtual
   * memory should solve the dirty side of this... */
  char* (*modname)(void) = (void*)((uintptr_t)hdr + elf64_find_symbol(hdr, "module_name"));
  char* (*moddesc)(void) = (void*)((uintptr_t)hdr + elf64_find_symbol(hdr, "module_description"));
  puts("-> Initializing module [");
  puts(modname());
  puts("]\n");
  puts("\t(");
  puts(moddesc());
  puts(")\n");
  KTRACE("Loading module %s\n", modname());
  __asm volatile("MOV %0, %%RAX; \
                  CALL %%RAX;"
                  :: "r" (sModEP));
  KTRACE("Back from module\n");

}

uintptr_t elf64_kernel_symbol_addr(const char* symname)
{
	return elf64_find_symbol((Elf64_Ehdr*)0x2FF000, symname);
}