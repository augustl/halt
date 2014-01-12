#include <efi.h>

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *systab, EFI_MEMORY_DESCRIPTOR *map, UINTN *key) {
  UINTN size = sizeof(EFI_MEMORY_DESCRIPTOR);

  EFI_STATUS status = EFI_LOAD_ERROR;
  while (status != EFI_SUCCESS) {
    // Spec says we should give it some extra space.
    size += sizeof(EFI_MEMORY_DESCRIPTOR) * 2;

    status = systab->BootServices->AllocatePool(EfiLoaderData, size, (void **)&map);
    if (status != EFI_SUCCESS) {
      // Allocation failed, assume that the world has ended.
      return status;
    }

    status = systab->BootServices->GetMemoryMap(&size, map, key, NULL, NULL);
    if (status != EFI_SUCCESS) {
      systab->BootServices->FreePool(map);
    }
  }

  return status;
}

// Because 4 is a good random number! http://xkcd.com/221/
#define MAX_EXIT_BOOT_RETRIES 4

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  SIMPLE_TEXT_OUTPUT_INTERFACE *con_out = systab->ConOut;
  con_out->OutputString(con_out, L"This is HALT!\r\n");

  int num_exit_boot_retries = 0;
  status = EFI_LOAD_ERROR;
  while (status != EFI_SUCCESS && num_exit_boot_retries < MAX_EXIT_BOOT_RETRIES) {
    ++num_exit_boot_retries;

    EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
    UINTN memory_map_key;
    status = get_memory_map(systab, memory_map, &memory_map_key);
    if (status == EFI_SUCCESS) {
      status = systab->BootServices->ExitBootServices(image, memory_map_key);
    } else {
      systab->BootServices->FreePool(memory_map);
    }
  }

  return status;
}
