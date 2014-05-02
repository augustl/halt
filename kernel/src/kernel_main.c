// These will work even on -ffreestanding
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void kernel_main() {
  char *addr = 0;
  *addr = 0x66;
}
