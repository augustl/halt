#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void halt_multiboot_main() {
}
