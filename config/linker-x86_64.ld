OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_VMA = 0xffffffff80000000;
KERNEL_LMA = 0x100000;

PHDRS {
    text PT_LOAD FLAGS(5);
    rodata PT_LOAD FLAGS(4);
    data PT_LOAD FLAGS(6);
}

SECTIONS {
    . = KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA) ALIGN(4K) {
        *(.text .text.*)
    } :text

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA + KERNEL_LMA) ALIGN(4K) {
        *(.rodata .rodata.*)
    } :rodata

    .limine_requests : AT(ADDR(.limine_requests) - KERNEL_VMA + KERNEL_LMA) ALIGN(8) {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :rodata

    .data : AT(ADDR(.data) - KERNEL_VMA + KERNEL_LMA) ALIGN(4K) {
        *(.data .data.*)
    } :data

    .bss : AT(ADDR(.bss) - KERNEL_VMA + KERNEL_LMA) ALIGN(4K) {
        *(COMMON)
        *(.bss .bss.*)
    } :data
}
