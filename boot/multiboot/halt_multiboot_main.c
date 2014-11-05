#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <multiboot.h>
#include <halt.h>

#define HALT_MMAP_MAX_ADDR_SIZE UINTPTR_MAX
#define HALT_MMAP_MAX_SIZE 32

static halt_sys_t *static_halt_sys;
static halt_mmap_ent_t static_halt_mmap[HALT_MMAP_MAX_SIZE];
static bool is_initialized = false;

static void sys_struct_set_mmap(multiboot_memory_map_t *mb_mmap_ary, uint32_t mb_mmap_size) {
  static_halt_sys->mmap_length = 0;
  static_halt_sys->mmap = static_halt_mmap;
  halt_mmap_ent_t *ent = static_halt_mmap;

  while (mb_mmap_size-- > 0) {
    multiboot_memory_map_t *mb_mmap = mb_mmap_ary++;

    if (mb_mmap->type != MULTIBOOT_MEMORY_AVAILABLE) {
      continue;
    }

    if (mb_mmap->addr >= UINTPTR_MAX) {
      continue;
    }

    ent->addr = mb_mmap->addr;
    if (mb_mmap->addr + mb_mmap->len > UINTPTR_MAX) {
      ent->size = mb_mmap->len - (mb_mmap->addr + mb_mmap->len - UINTPTR_MAX);
    } else {
      ent->size = mb_mmap->len;
    }

    ++ent;
    ++static_halt_sys->mmap_length;
  }
}

static halt_err_t sys_struct_create(multiboot_info_t *mbi, halt_sys_t **halt_sys_ptr) {
  // Should only be called once - don't want to deal with correctly allocating
  // memory at this early stage, so we use static variables to store the map.
  if (is_initialized) {
    return HALT_ERROR;
  }
  is_initialized = true;

  uint32_t mbi_mmap_length = mbi->mmap_length / sizeof(multiboot_memory_map_t);
  sys_struct_set_mmap((void *)mbi->mmap_addr, mbi_mmap_length);

  *halt_sys_ptr = static_halt_sys;
  return HALT_SUCCESS;
}

void halt_multiboot_main(unsigned long magic, unsigned long addr) {
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return;
  }

  multiboot_info_t *mbi = (multiboot_info_t *)addr;
  halt_sys_t *halt_sys = NULL;
  halt_err_t err = sys_struct_create(mbi, &halt_sys);
  if (err != HALT_SUCCESS) {
    return;
  }

  halt_main(halt_sys);
}
