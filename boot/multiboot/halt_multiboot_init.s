.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
        /* Initialize stack */
	movl $stack_top, %esp

        /* Reset EFLAGS */
        pushl $0
        popf

        /* Args for main function */
        pushl %ebx
        pushl %eax

	call halt_multiboot_main

	cli
	hlt
.Lhang:
	jmp .Lhang

.size _start, . - _start
