ARCH := x86_64
TARGET := x86_64-unknown-elf

LLVM_PREFIX ?= /opt/homebrew/opt/llvm/bin
LLD_PREFIX ?= /opt/homebrew/opt/lld/bin

CC = clang
LD = $(if $(wildcard $(LLVM_PREFIX)/ld.lld),$(LLVM_PREFIX)/ld.lld,$(if $(wildcard $(LLD_PREFIX)/ld.lld),$(LLD_PREFIX)/ld.lld,ld.lld))
READELF = $(if $(wildcard $(LLVM_PREFIX)/llvm-readelf),$(LLVM_PREFIX)/llvm-readelf,llvm-readelf)
OBJDUMP = $(if $(wildcard $(LLVM_PREFIX)/llvm-objdump),$(LLVM_PREFIX)/llvm-objdump,llvm-objdump)
XORRISO = xorriso
QEMU = qemu-system-x86_64
GIT = git

BUILD_DIR := build
ISO_ROOT := $(BUILD_DIR)/iso_root

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
ISO_IMAGE := $(BUILD_DIR)/halt.iso

LIMINE_DIR := tools/limine
LIMINE_REPO := https://github.com/limine-bootloader/limine.git
LIMINE_BRANCH := v9.x-binary

CFLAGS := \
	--target=$(TARGET) \
	-std=c11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pic \
	-mcmodel=kernel \
	-mno-red-zone \
	-Wall -Wextra -Werror \
	-O2 -g

ASFLAGS := --target=$(TARGET) -ffreestanding -fno-pic -mcmodel=kernel -mno-red-zone -g
LDFLAGS := -T config/linker-x86_64.ld -nostdlib

OBJS := $(BUILD_DIR)/start.o $(BUILD_DIR)/kernel.o

.PHONY: all limine clean run debug inspect doctor

all: $(ISO_IMAGE)

$(LIMINE_DIR)/limine:
	./scripts/fetch-limine.sh "$(LIMINE_REPO)" "$(LIMINE_BRANCH)" "$(LIMINE_DIR)"
	if [ ! -x "$(LIMINE_DIR)/limine" ]; then $(MAKE) -C "$(LIMINE_DIR)"; fi

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/start.o: kernel/start.S | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(ISO_IMAGE): $(KERNEL_ELF) $(LIMINE_DIR)/limine
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/limine
	mkdir -p $(ISO_ROOT)/EFI/BOOT
	cp $(KERNEL_ELF) $(ISO_ROOT)/boot/kernel.elf
	cp config/limine.conf $(ISO_ROOT)/boot/limine/limine.conf
	cp $(LIMINE_DIR)/limine-bios.sys $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/
	$(XORRISO) -as mkisofs \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part \
		--efi-boot-image \
		--protective-msdos-label \
		$(ISO_ROOT) -o $(ISO_IMAGE)
	$(LIMINE_DIR)/limine bios-install $(ISO_IMAGE)

run: $(ISO_IMAGE)
	$(QEMU) -M q35 -cdrom $(ISO_IMAGE) -serial stdio -no-reboot -no-shutdown

debug: $(ISO_IMAGE)
	$(QEMU) -M q35 -cdrom $(ISO_IMAGE) -serial stdio -no-reboot -no-shutdown -s -S

inspect: $(KERNEL_ELF)
	$(READELF) -h -S $(KERNEL_ELF)
	$(OBJDUMP) -d $(KERNEL_ELF)

doctor:
	@missing=0; \
	check_tool() { \
		label="$$1"; \
		tool="$$2"; \
		if [ -x "$$tool" ]; then \
			echo "[ok] $$label: $$tool"; \
		elif command -v "$$tool" >/dev/null 2>&1; then \
			echo "[ok] $$label: $$(command -v "$$tool")"; \
		else \
			echo "[missing] $$label ($$tool)"; \
			missing=1; \
		fi; \
	}; \
	check_tool clang "$(CC)"; \
	check_tool ld.lld "$(LD)"; \
	check_tool llvm-readelf "$(READELF)"; \
	check_tool llvm-objdump "$(OBJDUMP)"; \
	check_tool xorriso "$(XORRISO)"; \
	check_tool qemu-system-x86_64 "$(QEMU)"; \
	check_tool git "$(GIT)"; \
	if [ $$missing -ne 0 ]; then \
		echo ""; \
		echo "One or more required tools are missing."; \
		case "$$(uname -s)" in \
			Darwin) \
				echo "macOS hint: brew install llvm lld xorriso qemu git"; \
				echo "If brew LLVM is not on PATH, this Makefile auto-detects /opt/homebrew/opt/llvm/bin."; \
				echo "If ld.lld is still missing, this Makefile also auto-detects /opt/homebrew/opt/lld/bin."; \
				echo "You can also export PATH manually, e.g.:"; \
				echo '  export PATH="/opt/homebrew/opt/llvm/bin:$$PATH"'; \
				;; \
			Linux) \
				echo "Linux hint: install llvm, lld, xorriso, qemu-system-x86, and git via your package manager."; \
				;; \
			*) \
				echo "Install clang, lld, llvm tools, xorriso, qemu-system-x86_64, and git."; \
				;; \
		esac; \
		exit 1; \
	fi; \
	echo ""; \
	echo "All required tools are installed."

limine:
	$(MAKE) $(LIMINE_DIR)/limine

clean:
	rm -rf $(BUILD_DIR)
