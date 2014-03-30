# TODO: make this file easy to work with. Currently any changes you make, such as break points, will show up in "git status".

file uefi_bootloader/uefi_bootloader.efi

# Use python to parse and set symbols properly for the uefi boot loader. This is probably built-in since I'm using gdb to extract the information..
python
# Set to the output of "Boot loader base" when running the boot loader under qemu.
boot_loader_addr = 0x3EB97000

import subprocess
out = subprocess.check_output(["gdb", "-nx", "--batch", "-ex", "info files", "uefi_bootloader/uefi_bootloader.efi"])
for line in out.split("\n"):
    if ".text" in line:
        text_offset = hex(boot_loader_addr + int(line.strip().split(' ')[0], 0))
    if ".data" in line:
        data_offset = hex(boot_loader_addr + int(line.strip().split(' ')[0], 0))
gdb.execute("add-symbol-file uefi_bootloader/uefi_bootloader-debug.efi " + text_offset + " -s .data " + data_offset)
end

target remote :1234
set architecture i386:x86-64:intel

break uefi_bootloader.c:265
