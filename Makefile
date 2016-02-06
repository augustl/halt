ARCH=x86_64

BUILD=build/$(ARCH)-halt/debug
LDARGS=-n

AS=nasm
CP=cp
LD=ld
MKDIR=mkdir
RM=rm

.PHONY: clean all iso run

all: $(BUILD)/os.iso

clean:
	$(RM) -rf build

run: $(BUILD)/os.iso
	qemu-system-x86_64 -cdrom $(BUILD)/os.iso

$(BUILD)/boot.o: boot/multiboot/boot.asm
	$(MKDIR) -p $(BUILD)
	$(AS) -f elf64 -o $@ $<

$(BUILD)/multiboot_header.o: boot/multiboot/multiboot_header.asm
	$(MKDIR) -p $(BUILD)
	$(AS) -f elf64 -o $@ $<

$(BUILD)/long_mode_init.o: boot/multiboot/long_mode_init.asm
	$(MKDIR) -p $(BUILD)
	$(AS) -f elf64 -o $@ $<

$(BUILD)/kernel.bin: $(BUILD)/multiboot_header.o $(BUILD)/boot.o $(BUILD)/long_mode_init.o
	$(LD) $(LDARGS) -o $@ -T boot/multiboot/linker.ld $(BUILD)/multiboot_header.o $(BUILD)/boot.o $(BUILD)/long_mode_init.o

$(BUILD)/os.iso: $(BUILD)/kernel.bin
	$(MKDIR) -p $(BUILD)/isofiles/boot/grub
	$(CP) $(BUILD)/kernel.bin $(BUILD)/isofiles/boot/kernel.bin
	$(CP) boot/grub.cfg $(BUILD)/isofiles/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD)/os.iso $(BUILD)/isofiles 2> /dev/null
	$(RM) -r $(BUILD)/isofiles
