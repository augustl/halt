#define ELF_NIDENT 16

typedef uint64_t elf64_addr_t;
typedef uint64_t elf64_off_t;
typedef uint16_t elf64_half_t;
typedef uint32_t elf64_word_t;
typedef uint32_t elf64_sword_t;
typedef uint64_t elf64_xword_t;
typedef uint64_t elf64_sxword_t;

typedef struct {
  uint8_t e_ident[ELF_NIDENT];
  elf64_half_t e_type;
  elf64_half_t e_machine;
  elf64_word_t e_version;
  elf64_addr_t e_entry;
  elf64_off_t e_phoff;
  elf64_off_t e_shoff;
  elf64_word_t e_flags;
  elf64_half_t e_ehsize;
  elf64_half_t e_phentsize;
  elf64_half_t e_phnum;
  elf64_half_t e_shentsize;
  elf64_half_t e_shnum;
  elf64_half_t e_shstrndx;
} halt_elf_header;

enum halt_elf_ident {
  halt_elf_ident_magic_0 = 0,
  halt_elf_ident_magic_1 = 1,
  halt_elf_ident_magic_2 = 2,
  halt_elf_ident_magic_3 = 3,
  halt_elf_ident_class = 4,
  halt_elf_ident_endianness = 5,
  halt_elf_ident_version = 6,
  halt_elf_ident_os_abi = 7,
  halt_elf_ident_abi_version = 8,
  halt_elf_ident_pad = 9
};

typedef struct {
  elf64_word_t sh_name;
  elf64_word_t sh_type;
  elf64_xword_t sh_flags;
  elf64_addr_t sh_addr;
  elf64_off_t sh_offset;
  elf64_xword_t sh_size;
  elf64_word_t sh_link;
  elf64_word_t sh_info;
  elf64_xword_t sh_addralign;
  elf64_xword_t sh_entsize;
} halt_elf_section_header;

enum halt_elf_section_header_type {
  halt_elf_section_header_type_null = 0,
  halt_elf_section_header_type_progbits = 1,
  halt_elf_section_header_type_symtab = 2,
  halt_elf_section_header_type_strtab = 3,
  halt_elf_section_header_type_rela = 4,
  halt_elf_section_header_type_nobits = 8,
  halt_elf_section_header_type_rel = 9
};

typedef struct {
  elf64_word_t p_type;
  elf64_word_t p_flags;
  elf64_off_t p_offset;
  elf64_addr_t p_vaddr;
  elf64_addr_t p_paddr;
  elf64_xword_t p_filesz;
  elf64_xword_t p_memsz;
  elf64_xword_t p_align;
} halt_elf_program_header;

enum halt_elf_program_header_type {
  halt_elf_program_header_type_null = 0,
  halt_elf_program_header_type_load = 1,
  halt_elf_program_header_type_dynamic = 2,
  halt_elf_program_header_type_interp = 3,
  halt_elf_program_header_type_note = 4,
  halt_elf_program_header_type_shlib = 5,
  halt_elf_program_header_type_phdr = 6
};

#define halt_elf_class_32 (1)
#define halt_elf_class_64 (2)

#define halt_elf_endianness_little (1)
#define halt_elf_endianness_big (2)

#define halt_elf_os_abi_system_v (0x00)
#define halt_elf_os_abi_hp_ux (0x01)
#define halt_elf_os_abi_netbsd (0x02)
#define halt_elf_os_abi_linux (0x03)
#define halt_elf_os_abi_solaris (0x06)
#define halt_elf_os_abi_aix (0x07)
#define halt_elf_os_abi_irx (0x08)
#define halt_elf_os_abi_freebsd (0x09)
#define halt_elf_os_abi_openbsd (0x0C)

enum halt_elf_type {
  halt_elf_type_none = 0,
  halt_elf_type_relocatable = 1,
  halt_elf_type_executable = 2
};
