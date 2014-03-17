section .bootstrap_stack
align 4
stack_bottom:
times 16384 db 0
stack_top:

section .text
global _start
_start:
        mov esp, stack_top
        cli
.hang:
        hlt
        jmp .hang
