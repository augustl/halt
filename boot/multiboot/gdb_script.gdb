file target/halt_multiboot.bin
target remote :1234
break halt_multiboot_main.c:9
continue