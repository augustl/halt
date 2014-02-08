# Thanks, http://mjg59.dreamwidth.org/18773.html.
ARCH = x86_64
LIB_PATH = /usr/lib64
EFI_PATH = /usr/lib
EFI_INCLUDE = /usr/include/efi
EFI_INCLUDES = -I$(EFI_INCLUDE) -I$(EFI_INCLUDE)/$(ARCH) -I$(EFI_INCLUDE)/protocol -nostdinc
EFI_LIBS := -lefi -lgnuefi $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

CFLAGS = -Wall -Werror -g \
	$(EFI_INCLUDES) \
	-fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
	-DGNU_EFI_USE_MS_ABI -fPIC -maccumulate-outgoing-args -D$(ARCH)

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_PATH)/elf_$(ARCH)_efi.lds -shared -Bsymbolic \
	-L$(EFI_PATH) -L$(LIB_PATH) $(EFI_PATH)/crt0-efi-$(ARCH).o -lefi -lgnuefi

TARGET = uefi_bootloader.efi

SECTIONS = .text .sdata .data .dynamic .dynsym .rel .rela .reloc .eh_frame

all: $(TARGET)

%.so: %.o
	$(LD) -o $@ $(LDFLAGS) $^ $(EFI_LIBS)

%.efi: %.so
	objcopy $(foreach sec,$(SECTIONS),-j $(sec)) --target=efi-app-$(ARCH) $^ $@

clean:
	rm -rf $(TARGET) target

.PHONY vboximage: target/disk.vdi

target/disk.vdi: target/disk.img
	if [ -f $@ ]; then rm $@; fi;
	VBoxManage convertdd -format VDI --uuid "19fede09-2621-4fd8-8267-bb85e00936a6" $^ $@

target/disk.img:
	mkdir -p target
	dd if=/dev/zero of=$@ bs=1M count=50
	parted $@ mklabel gpt
	parted -- $@ mkpart primary 1 -1
	mkdosfs -F 12 $@

MOUNT_DIR = mnt
DEPLOY_DIR = $(MOUNT_DIR)/EFI/BOOT

mount: target/disk.img
	mkdir -p $(MOUNT_DIR)
	sudo mount -o loop,flush,uid=1000 target/disk.img $(MOUNT_DIR)

.PHONY qemu:
	qemu-system-x86_64 -L qemu-bios -cdrom target/disk.img -m 1024 -vga none -monitor stdio

umount:
	sudo umount $(MOUNT_DIR)

deploy: $(TARGET)
	mkdir -p $(DEPLOY_DIR)
	cp $(TARGET) $(DEPLOY_DIR)/BOOTx64.EFI

build: all target/disk.img mount deploy umount vboximage
