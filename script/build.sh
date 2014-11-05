# TODO: Make gcc location configurable
BIN=$HOME/local/gcc-cross/bin
GCCFLAGS_DEBUG="-Og -ggdb"
GCCFLAGS="-std=gnu99 -nostdlib -ffreestanding -O2 -Wall -Wextra -Iinclude $GCCFLAGS_DEBUG"
rm -rf target
mkdir -p target
$BIN/i686-elf-as boot/multiboot/halt_multiboot_init.s -o target/halt_multiboot_init.o
$BIN/i686-elf-gcc -c boot/multiboot/halt_multiboot_main.c -o target/halt_multiboot_main.o -Iboot/multiboot/include $GCCFLAGS
$BIN/i686-elf-gcc -c kernel/src/halt_main.c -o target/halt_main.o $GCCFLAGS
$BIN/i686-elf-gcc -T script/linker.ld -o target/halt_multiboot.bin target/halt_multiboot_init.o target/halt_multiboot_main.o target/halt_main.o -lgcc $GCCFLAGS
