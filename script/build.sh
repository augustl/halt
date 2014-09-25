# TODO: Make gcc location configurable
BIN=$HOME/local/gcc-cross/bin
GCCFLAGS="-Wall -Wextra -ggdb -Iinclude"
rm -rf target
mkdir -p target
$BIN/i686-elf-as boot/multiboot/halt_multiboot_init.s -o target/halt_multiboot_init.o
$BIN/i686-elf-gcc -c boot/multiboot/halt_multiboot_main.c -o target/halt_multiboot_main.o -std=gnu99 -nostdlib -ffreestanding -O2 -Iboot/multiboot/include $GCCFLAGS
$BIN/i686-elf-gcc -c kernel/src/halt_main.c -o target/halt_main.o -std=gnu99 -nostdlib -ffreestanding -O2 $GCCFLAGS
$BIN/i686-elf-gcc -T script/linker.ld -o target/halt_multiboot.bin -ffreestanding -O2 -nostdlib target/halt_multiboot_init.o target/halt_multiboot_main.o target/halt_main.o -lgcc $GCCFLAGS
