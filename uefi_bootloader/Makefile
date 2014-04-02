# Thanks, http://mjg59.dreamwidth.org/18773.html.
ARCH = x86_64
LIB_PATH = /usr/lib64
EFI_PATH = /usr/lib
EFI_INCLUDE = /usr/include/efi
EFI_INCLUDES = -I$(EFI_INCLUDE) -I$(EFI_INCLUDE)/$(ARCH) -I$(EFI_INCLUDE)/protocol -nostdinc
EFI_LIBS := -lefi -lgnuefi $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

CFLAGS = -Wall -Werror -g \
	$(EFI_INCLUDES) -I../kernel/include/share \
	-fno-stack-protector -fpic -fshort-wchar -mno-red-zone -fno-builtin \
	-DGNU_EFI_USE_MS_ABI -fPIC -maccumulate-outgoing-args -D$(ARCH)

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_PATH)/elf_$(ARCH)_efi.lds -shared -Bsymbolic \
	-L$(EFI_PATH) -L$(LIB_PATH) $(EFI_PATH)/crt0-efi-$(ARCH).o -lefi -lgnuefi

TARGET = uefi_bootloader.efi

SECTIONS = .text .sdata .data .dynamic .dynsym .rel .rela .reloc .eh_frame
DEBUG_SECTIONS = .debug_info .debug_abbrev .debug_loc .debug_aranges \
.debug_line .debug_macinfo .debug_str 

all: $(TARGET)

debug: uefi_bootloader-debug.efi

%.so: %.o
	$(LD) -o $@ $(LDFLAGS) $^ $(EFI_LIBS)

%.efi: %.so
	objcopy $(foreach sec,$(SECTIONS),-j $(sec)) --target=efi-app-$(ARCH) $^ $@

%-debug.efi: %.so
	objcopy $(foreach sec,$(SECTIONS) $(DEBUG_SECTIONS),-j $(sec)) --target=efi-app-$(ARCH) $^ $@

clean:
	rm -rf *.efi
