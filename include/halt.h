#include <stdint.h>

typedef struct {
  uint32_t size;
  uintptr_t addr;
} halt_mmap_ent_t;

typedef struct {
  uint32_t mmap_length;
  halt_mmap_ent_t *mmap;
} halt_sys_t;


typedef uint32_t halt_err_t;
#define HALT_SUCCESS 0
#define HALT_ERROR 1


void halt_main(halt_sys_t *halt_sys);
