#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map_key(EFI_SYSTEM_TABLE *systab, UINTN *key) {
  EFI_STATUS status;
  UINTN map_size = 0;
  EFI_MEMORY_DESCRIPTOR *map;
  UINTN descriptor_size;
  UINT32 descriptor_version;

  status = systab->BootServices->GetMemoryMap(&map_size, map, key, &descriptor_size, &descriptor_version);
  if (status != EFI_BUFFER_TOO_SMALL) {
    return EFI_LOAD_ERROR;
  }

  // Spec says we should give it some additional space, since the allocation of the new buffer can
  // potentially increase the memory map size.
  map_size *= 1.25;

  status = systab->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&map);
  if (status != EFI_SUCCESS) {
    systab->BootServices->FreePool(map);
    return status;
  }

  return systab->BootServices->GetMemoryMap(&map_size, memory_map, key, &descriptor_size, &descriptor_version);
}

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  SIMPLE_TEXT_OUTPUT_INTERFACE *con_out = systab->ConOut;
  con_out->OutputString(con_out, L"This is HALT!\r\n");

  UINTN memory_map_key;
  status = get_memory_map_key(systab, &memory_map_key);
  if (EFI_ERROR(status)) {
    con_out->OutputString(con_out, L"There was an error allocating memory map information.\r\n");
    return status;
  }

  status = systab->BootServices->ExitBootServices(image, memory_map_key);
  if (EFI_ERROR(status)) {
    con_out->OutputString(con_out, L"There was an error exiting UEFI boot mode.\r\n");
    return status;
  }

  return EFI_SUCCESS;
}
