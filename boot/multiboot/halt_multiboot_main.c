#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <multiboot.h>
#include <halt.h>

#define HALT_MMAP_MAX_ADDR_SIZE UINTPTR_MAX
#define HALT_MMAP_MAX_SIZE 32

// We assume that this funciton is ever called once and store the memmap statically so
// we don't have to deal with the complexity of allocating memory for it.
static halt_sys_t *static_halt_sys;
static halt_mmap_ent_t *static_halt_mmap[HALT_MMAP_MAX_SIZE];
static bool is_initialized = false;
static halt_err_t create_sys_struct(multiboot_info_t *mbi, halt_sys_t **halt_sys_ptr) {
  if (is_initialized) {
    return HALT_ERROR;
  }
  is_initialized = true;

  uint32_t i;
  uint32_t size;
  multiboot_memory_map_t *mb_mmap = NULL;
  static_halt_sys->mmap_length = 0;
  // length should mean length, but in multiboot it means size (number of bytes)
  uint32_t mbi_mmap_length = mbi->mmap_length / sizeof(multiboot_memory_map_t);
  for (i = 0; i < mbi_mmap_length; i++) {
    if (static_halt_sys->mmap_length > HALT_MMAP_MAX_SIZE) {
      return HALT_ERROR;
    }

    mb_mmap = ((void *)mbi->mmap_addr) + i * sizeof(multiboot_memory_map_t);

    if (mb_mmap->type != MULTIBOOT_MEMORY_AVAILABLE) {
      continue;
    }

    if (mb_mmap->addr >= UINTPTR_MAX) {
      continue;
    }

    if (mb_mmap->addr + mb_mmap->size > UINTPTR_MAX) {
      size = mb_mmap->size - (mb_mmap->addr + mb_mmap->size - UINTPTR_MAX);
    } else {
      size = mb_mmap->size;
    }

    halt_mmap_ent_t *ent = static_halt_mmap[static_halt_sys->mmap_length++];
    ent->size = size;
    ent->addr = mb_mmap->addr;
  }

  *halt_sys_ptr = static_halt_sys;
  return HALT_SUCCESS;
}

void halt_multiboot_main(unsigned long magic, unsigned long addr) {
  int *point = (int *)0x20;
  *point = 4;

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return;
  }

  multiboot_info_t *mbi = (multiboot_info_t *)addr;
  halt_sys_t *halt_sys = NULL;
  halt_err_t err = create_sys_struct(mbi, &halt_sys);
  if (err != HALT_SUCCESS) {
    return;
  }

  if (halt_sys->mmap_length == 0) {
    return;
  }
}
