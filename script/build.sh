# TODO: Make gcc location configurable
BIN=$HOME/local/gcc-cross/bin
GCCFLAGS_DEBUG="-Og -ggdb"
GCCFLAGS="-nostdlib -ffreestanding -Wall -Wextra -Iinclude $GCCFLAGS_DEBUG"
GCCFLAGS64="-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow $GCCFLAGS"
rm -rf target/*
mkdir -p target/boot
mkdir -p target/kernel
$BIN/i686-elf-as boot/multiboot/halt_multiboot_bootstrap.s -o target/boot/halt_multiboot_bootstrap.o
$BIN/i686-elf-gcc -c boot/multiboot/halt_multiboot_main.c -o target/boot/halt_multiboot_main.o -Iboot/multiboot/include $GCCFLAGS
$BIN/i686-elf-gcc -T script/linker.ld -o target/halt_multiboot.bin target/boot/halt_multiboot_bootstrap.o target/boot/halt_multiboot_main.o -nostdlib -lgcc
$BIN/x86_64-elf-as kernel/src/halt_main_bootstrap.s -o target/kernel/halt_main_bootstrap.o
$BIN/x86_64-elf-gcc -c kernel/src/halt_main.c -o target/kernel/halt_main.o $GCCFLAGS64
$BIN/x86_64-elf-gcc -T script/halt_linker.ld -o target/halt.bin target/kernel/halt_main_bootstrap.o target/kernel/halt_main.o -nostdlib -lgcc
