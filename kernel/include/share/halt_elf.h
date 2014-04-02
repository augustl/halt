#define ELF_NIDENT 16

typedef struct {
  uint8_t e_ident[ELF_NIDENT];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
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

/* enum Elf_Ident { */
/* 	EI_MAG0		= 0, // 0x7F */
/* 	EI_MAG1		= 1, // 'E' */
/* 	EI_MAG2		= 2, // 'L' */
/* 	EI_MAG3		= 3, // 'F' */
/* 	EI_CLASS	= 4, // Architecture (32/64) */
/* 	EI_DATA		= 5, // Byte Order */
/* 	EI_VERSION	= 6, // ELF Version */
/* 	EI_OSABI	= 7, // OS Specific */
/* 	EI_ABIVERSION	= 8, // OS Specific */
/* 	EI_PAD		= 9  // Padding */
/* }; */
