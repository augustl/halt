#include <efi.h>
#include <efilib.h>

EFI_STATUS load_runtime_image_handler(EFI_HANDLE parentImage, EFI_SYSTEM_TABLE *systab, EFI_HANDLE *handle) {
  EFI_LOADED_IMAGE *li;
  EFI_GUID protocol = LOADED_IMAGE_PROTOCOL;
  if (EFI_ERROR(systab->BootServices->HandleProtocol(parentImage, &protocol, (void**)&li))) {
    return EFI_LOAD_ERROR;
  }
  EFI_DEVICE_PATH *runtime_image_path = FileDevicePath(li->DeviceHandle,  L"\\wat\\wut.efi");
  return systab->BootServices->LoadImage(FALSE, parentImage, runtime_image_path, NULL, 0, handle);
}

EFI_STATUS get_memory_map_key(EFI_SYSTEM_TABLE *systab, UINTN *memoryMapKey) {
  UINTN memoryMapSize;
  EFI_MEMORY_DESCRIPTOR memoryMap;
  UINTN memoryMapDescriptorSize;
  UINT32 memoryMapDescriptorVersion;
  return systab->BootServices->GetMemoryMap(&memoryMapSize, &memoryMap, memoryMapKey, &memoryMapDescriptorSize, &memoryMapDescriptorVersion);
}

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = systab->ConOut;
  conOut->OutputString(conOut, L"This is HALT!");

  EFI_HANDLE runtimeImageHandler;
  if (EFI_ERROR(load_runtime_image_handler(image, systab, &runtimeImageHandler))) {
    return EFI_LOAD_ERROR;
  }

  UINTN memoryMapKey;
  if (EFI_ERROR(get_memory_map_key(systab, &memoryMapKey))) {
    return EFI_LOAD_ERROR;
  }

  systab->BootServices->ExitBootServices(runtimeImageHandler, memoryMapKey);

  return EFI_SUCCESS;
}
