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
  sys_struct_set_mmap((multiboot_memory_map_t *)(intptr_t)mbi->mmap_addr, mbi_mmap_length);

  *halt_sys_ptr = static_halt_sys;
  return HALT_SUCCESS;
}

static char *halt_multiboot_module_name = "halt.bin";
static int halt_multiboot_module_name_length = sizeof(*halt_multiboot_module_name);
static bool my_strcmp(char *a, char *b, int i) {
  while (i--) {
    if (a[i] != b[i]) {
      return false;
    }
  }

  return true;
}

static halt_err_t kernel_main_mod_get(uint32_t mods_count, uint32_t mods_addr, multiboot_module_t **result) {
  multiboot_module_t *mod;
  char *cmdline;

  if (mods_count == 0) {
    return HALT_ERROR;
  }

  mod = (multiboot_module_t *)(intptr_t)mods_addr;

  while (mods_count-- > 0) {
    cmdline = (char *)mod->cmdline;
    if (my_strcmp(cmdline, halt_multiboot_module_name, halt_multiboot_module_name_length)) {
      *result = (multiboot_module_t *)(intptr_t)mods_addr;
      return HALT_SUCCESS;
    }
    ++mod;
  }

  return HALT_ERROR;
}

void halt_multiboot_main(unsigned long magic, unsigned long addr) {
  halt_err_t err;
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return;
  }

  multiboot_info_t *mbi = (multiboot_info_t *)addr;

  multiboot_module_t *kernel_main_mod;
  err = kernel_main_mod_get(mbi->mods_count, mbi->mods_addr, &kernel_main_mod);
  if (err != HALT_SUCCESS) {
    return;
  }

  halt_sys_t *halt_sys = NULL;
  err = sys_struct_create(mbi, &halt_sys);
  if (err != HALT_SUCCESS) {
    return;
  }

  asm volatile("call _enable_long_mode");
  // TODO: Set static_halt_sys in %eax, which will be the 1st argument to halt_main
  asm volatile("call _jump_to_halt_main");

  // halt_main(halt_sys);
}
