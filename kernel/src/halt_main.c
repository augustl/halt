#include <halt.h>

void halt_main(halt_sys_t *halt_sys) {
  int *point = (int *)0x20;
  *point = 0x66;

  if (halt_sys->mmap_length == 0) {
    return;
  }
}
