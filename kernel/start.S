.section .text
.global _start
.type _start, @function

_start:
    lea stack_top(%rip), %rsp
    call kmain

1:
    hlt
    jmp 1b

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:
