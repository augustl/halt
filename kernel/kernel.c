#include <stdint.h>

#if defined(__ELF__)
#define LIMINE_SECTION(name) __attribute__((used, section(name)))
#else
#define LIMINE_SECTION(name) __attribute__((used))
#endif

// Limine discovers requests by scanning a dedicated linker section.
// These three 64-bit words are part of the Limine base revision handshake:
//  - word 0/1: fixed Limine request ID for "base revision"
//  - word 2: requested base revision (3 here)
// Limine overwrites word 2 with 0 when the revision is supported.
LIMINE_SECTION(".limine_requests")
static volatile uint64_t limine_base_revision[] = {
    0xf9562b2d5c95a6c8ULL,
    0x6a7b384944536bdcULL,
    3
};

// Convenience check: Limine writes 0 on success for the requested base revision.
#define LIMINE_BASE_REVISION_SUPPORTED (limine_base_revision[2] == 0)

// Start marker for Limine's request block. The loader expects these exact IDs.
LIMINE_SECTION(".limine_requests_start")
static volatile uint64_t limine_requests_start_marker[] = {
    0xf6b8f4b39de7d1aeULL,
    0xfab91a6940fcb9cfULL
};

// End marker for Limine's request block. Must bracket all request entries.
LIMINE_SECTION(".limine_requests_end")
static volatile uint64_t limine_requests_end_marker[] = {
    0xadc0e0531bb10d03ULL,
    0x9572709f31764c62ULL
};

#if defined(__x86_64__)
// x86 port-mapped I/O write. Used to talk to legacy UART registers.
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void serial_init(void) {
    // We use COM1 (I/O base 0x3F8), a de-facto standard debug UART.
    // Register offsets below are relative to this base.

    // +1 = IER (Interrupt Enable Register): disable UART interrupts.
    outb(0x3F8 + 1, 0x00);

    // +3 = LCR (Line Control Register): set DLAB=1 to access divisor latches.
    outb(0x3F8 + 3, 0x80);

    // +0/+1 = DLL/DLM when DLAB=1.
    // Divisor 3 => 115200 / 3 = 38400 baud.
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);

    // +3 = LCR: clear DLAB, configure 8 data bits, no parity, 1 stop bit (8N1).
    outb(0x3F8 + 3, 0x03);

    // +2 = FCR (FIFO Control Register): enable FIFO, clear RX/TX queues,
    // and use 14-byte RX threshold (0xC7 = 1100_0111b).
    outb(0x3F8 + 2, 0xC7);

    // +4 = MCR (Modem Control Register): assert DTR/RTS and OUT2 so the UART
    // behaves as expected on common emulators/hardware (0x0B = 0000_1011b).
    outb(0x3F8 + 4, 0x0B);
}

static int serial_ready(void) {
    uint8_t status;
    // +5 = LSR (Line Status Register). Bit 5 (0x20) is THR empty, meaning
    // the transmitter can accept another byte.
    __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"((uint16_t)(0x3F8 + 5)));
    return (status & 0x20) != 0;
}
#else
// Non-x86_64 fallback: keep the same API but make operations no-ops.
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
    // Busy-wait until COM1 can accept the next byte.
    while (!serial_ready()) {
    }
    // COM1 data register is at base offset +0.
    outb(0x3F8, (uint8_t)c);
}

static void serial_write(const char *msg) {
    for (const char *p = msg; *p != '\0'; p++) {
        // Many serial consoles expect CRLF line endings. Translate \n to \r\n.
        if (*p == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*p);
    }
}

void kmain(void) {
    serial_init();

    // Halt immediately if bootloader/kernel handshake failed. Continuing with
    // an unknown base revision could make all later Limine interactions invalid.
    if (!LIMINE_BASE_REVISION_SUPPORTED) {
        serial_write("HALT: unsupported Limine base revision\n");
        // hlt in a tight loop keeps CPU usage low while stopping progress.
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    serial_write("HALT C kernel: hello world from x86_64 long mode\n");

    // Park the bootstrap CPU forever. At this stage this kernel has no scheduler
    // or idle loop, so an explicit halt loop is the simplest safe terminal state.
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
