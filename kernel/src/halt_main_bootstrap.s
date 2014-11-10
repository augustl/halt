.global _start
.type _start, @function
_start:
        call halt_main
        cli
        hlt
