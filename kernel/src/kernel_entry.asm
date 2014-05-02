section .text
global _start
_start:
        ; Note: next step is to get data structure from eax and find
        ; a suitable location in memory for the kernel stack. Then call
        ; kernel.
        ; mov esp, the_stack_location
        extern kernel_main
        call kernel_main
        cli
.hang:
        hlt
        jmp .hang
