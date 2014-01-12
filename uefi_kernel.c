#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map_key(EFI_SYSTEM_TABLE *systab, UINTN *memory_map_key) {
  EFI_STATUS status;
  UINTN memory_map_size = 0;
  EFI_MEMORY_DESCRIPTOR *memory_map;
  UINTN memory_map_descriptor_size;
  UINT32 memory_map_descriptor_version;

  status = systab->BootServices->GetMemoryMap(&memory_map_size, memory_map, memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
  if (status != EFI_BUFFER_TOO_SMALL) {
    return EFI_LOAD_ERROR;
  }

  // Spec says we should give it some additional space, since the allocation of the new buffer can
  // potentially increase the memory map size.
  memory_map_size *= 1.25;

  status = systab->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (void**)&memory_map);
  if (status != EFI_SUCCESS) {
    systab->BootServices->FreePool(memory_map);
    return status;
  }

  return systab->BootServices->GetMemoryMap(&memory_map_size, memory_map, memory_map_key, &memory_map_descriptor_size, &memory_map_descriptor_version);
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
