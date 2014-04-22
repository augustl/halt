#!/bin/sh

node etc/gdb/qemu-debug.js --efi-app-debug uefi_bootloader/uefi_bootloader-debug.efi --efi-app uefi_bootloader/uefi_bootloader.efi --boot-loader-addr 0x3EB96000
