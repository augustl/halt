typedef struct {
  uint32_t mmap_length;
  uint32_t mmap_addr;
} halt_sys_t;

typedef struct {
  uint32_t size;
  uint64_t addr;
} halt_mmap_ent_t;


typedef uint32_t halt_err_t;
#define HALT_SUCCESS 0
#define HALT_ERROR 1
