# Thanks, http://mjg59.dreamwidth.org/18773.html.
ARCH = x86_64
LIB_PATH = /usr/lib64
EFI_PATH = /usr/lib
EFI_INCLUDE = /usr/include/efi
EFI_INCLUDES = -I$(EFI_INCLUDE) -I$(EFI_INCLUDE)/$(ARCH) -I$(EFI_INCLUDE)/protocol -nostdinc

CFLAGS = -Wall -g \
	$(EFI_INCLUDES) \
	-fno-stack-protector -fpic -fshort-wchar -mno-red-zone \
	-DEFI_FUNCTION_WRAPPER

LDFLAGS = -nostdlib -znocombreloc -T $(EFI_PATH)/elf_$(ARCH)_efi.lds -shared -Bsymbolic \
	-L$(EFI_PATH) -L$(LIB_PATH) $(EFI_PATH)/crt0-efi-$(ARCH).o -lefi -lgnuefi

TARGET = uefi_kernel.efi

all: $(TARGET)

%.so: %.o
	$(LD) -o $@ $(LDFLAGS) $^ $(EFI_LIBS)

%.efi: %.so
	objcopy -j .text -j .sdata -j .data \
		-j .dynamic -j .dynsym -j .rel \
		-j .rela -j .reloc -j .eh_frame \
		--target=efi-app-$(ARCH) $^ $@

clean:
	rm -rf $(TARGET)
