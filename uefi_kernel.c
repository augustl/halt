#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map_key(EFI_SYSTEM_TABLE *systab, UINTN *memoryMapKey) {
  UINTN memoryMapSize;
  EFI_MEMORY_DESCRIPTOR memoryMap;
  UINTN memoryMapDescriptorSize;
  UINT32 memoryMapDescriptorVersion;
  return systab->BootServices->GetMemoryMap(&memoryMapSize, &memoryMap, memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
}

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = systab->ConOut;
  conOut->OutputString(conOut, L"This is HALT!\r\n");

  UINTN memoryMapKey;
  if (EFI_ERROR(get_memory_map_key(systab, &memoryMapKey))) {
    return EFI_LOAD_ERROR;
  }

  systab->BootServices->ExitBootServices(image, memoryMapKey);

  return EFI_SUCCESS;
}
