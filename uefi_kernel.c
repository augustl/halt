#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *systab, EFI_MEMORY_DESCRIPTOR **map, UINTN *key) {
  EFI_STATUS status;
  EFI_MEMORY_DESCRIPTOR *m = NULL;
  UINTN map_size;
  UINTN descriptor_size;
  UINT32 descriptor_version;

  map_size = sizeof(*m);

 again:
  map_size += sizeof(*m) * 2;
  status = systab->BootServices->AllocatePool(EfiLoaderData, map_size, (void **)&m);
  if (status != EFI_SUCCESS) {
    goto fail;
  }
  status = systab->BootServices->GetMemoryMap(&map_size, m, key, &descriptor_size, &descriptor_version);
  if (status == EFI_BUFFER_TOO_SMALL) {
    systab->BootServices->FreePool(map);
    goto again;
  }

  if (status != EFI_SUCCESS) {
    systab->BootServices->FreePool(map);
    goto fail;
  }

 fail:
  *map = m;
  return status;
}

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  SIMPLE_TEXT_OUTPUT_INTERFACE *con_out = systab->ConOut;
  con_out->OutputString(con_out, L"This is HALT!\r\n");

  UINTN memory_map_key;
  EFI_MEMORY_DESCRIPTOR *memory_map;
  int called_exit = 0;
 get_map:
  status = get_memory_map(systab, &memory_map, &memory_map_key);
  if (EFI_ERROR(status)) {
    con_out->OutputString(con_out, L"There was an error allocating memory map information.\r\n");
    goto free_mem_map;
    return status;
  }

  status = systab->BootServices->ExitBootServices(image, memory_map_key);
  if (status != EFI_SUCCESS) {
    if (called_exit) {
      goto free_mem_map;
    } else {
      // Only retrying once. The EFI spec says that upon first exit call, event handlers
      // might cause the memory map to change, and that this will only happen with the
      // first exit call.
      called_exit = 1;
      systab->BootServices->FreePool(memory_map);
      goto get_map;
    }
  }

  con_out->OutputString(con_out, L"Successfully exited UEFI bootloader!\r\n");

 free_mem_map:
  systab->BootServices->FreePool(memory_map);
  UINTN index;
  systab->BootServices->WaitForEvent(1, &systab->ConIn->WaitForKey, &index);
  return status;
}
