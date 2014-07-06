#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void halt_multiboot_main() {
  int *point = (int *)0x20;
  *point = 4;
  int *point2 = (int *)0x30;
  *point2 = 5;
}
