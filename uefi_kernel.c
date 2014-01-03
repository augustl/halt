#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map_key(EFI_SYSTEM_TABLE *systab, UINTN *memoryMapKey) {
  EFI_STATUS status;
  UINTN memoryMapSize = 0;
  EFI_MEMORY_DESCRIPTOR *memoryMap;
  UINTN memoryMapDescriptorSize;
  UINT32 memoryMapDescriptorVersion;

  status = systab->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
  if (status != EFI_BUFFER_TOO_SMALL) {
    return EFI_LOAD_ERROR;
  }

  // Spec says we should give it some additional space, since the allocation of the new buffer can
  // potentially increase the memory map size.
  memoryMapSize *= 1.25;

  status = systab->BootServices->AllocatePool(EfiLoaderData, memoryMapSize, (void**)&memoryMap);
  if (status != EFI_SUCCESS) {
    systab->BootServices->FreePool(memoryMap);
    return status;
  }

  return systab->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
}

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = systab->ConOut;
  conOut->OutputString(conOut, L"This is HALT!\r\n");

  UINTN memoryMapKey;
  status = get_memory_map_key(systab, &memoryMapKey);
  if (EFI_ERROR(status)) {
    conOut->OutputString(conOut, L"There was an error allocating memory map information.\r\n");
    return status;
  }

  systab->BootServices->ExitBootServices(image, memoryMapKey);

  return EFI_SUCCESS;
}
