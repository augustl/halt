#include <stdint.h>

#if defined(__ELF__)
#define LIMINE_SECTION(name) __attribute__((used, section(name)))
#else
#define LIMINE_SECTION(name) __attribute__((used))
#endif

LIMINE_SECTION(".limine_requests")
static volatile uint64_t limine_base_revision[] = {
    0xf9562b2d5c95a6c8ULL,
    0x6a7b384944536bdcULL,
    3
};

#define LIMINE_BASE_REVISION_SUPPORTED (limine_base_revision[2] == 0)

LIMINE_SECTION(".limine_requests_start")
static volatile uint64_t limine_requests_start_marker[] = {
    0xf6b8f4b39de7d1aeULL,
    0xfab91a6940fcb9cfULL
};

LIMINE_SECTION(".limine_requests_end")
static volatile uint64_t limine_requests_end_marker[] = {
    0xadc0e0531bb10d03ULL,
    0x9572709f31764c62ULL
};

#if defined(__x86_64__)
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void serial_init(void) {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static int serial_ready(void) {
    uint8_t status;
    __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"((uint16_t)(0x3F8 + 5)));
    return (status & 0x20) != 0;
}
#else
static inline void outb(uint16_t port, uint8_t value) {
    (void)port;
    (void)value;
}

static int serial_ready(void) {
    return 1;
}

static void serial_init(void) {
}
#endif

static void serial_write_char(char c) {
    while (!serial_ready()) {
    }
    outb(0x3F8, (uint8_t)c);
}

static void serial_write(const char *msg) {
    for (const char *p = msg; *p != '\0'; p++) {
        if (*p == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*p);
    }
}

void kmain(void) {
    serial_init();

    if (!LIMINE_BASE_REVISION_SUPPORTED) {
        serial_write("HALT: unsupported Limine base revision\n");
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    serial_write("HALT C kernel: hello world from x86_64 long mode\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
