#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <multiboot.h>

void halt_multiboot_main(unsigned long magic, unsigned long addr) {
  int *point = (int *)0x20;
  *point = 4;

  multiboot_info_t *mbi;

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    return;
  }

  mbi = (multiboot_info_t *)addr;
  if (mbi->flags & (1 << 6)) {
    return;
  }

  int *point2 = (int *)0x30;
  *point2 = 5;
}
