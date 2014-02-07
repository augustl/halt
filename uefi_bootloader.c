#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map(EFI_SYSTEM_TABLE *systab, UINTN *size, EFI_MEMORY_DESCRIPTOR **map, UINTN *key, UINTN *descriptor_size, UINT32 *descriptor_version) {
  EFI_STATUS status = EFI_LOAD_ERROR;
  *size = 0;

  while (status != EFI_SUCCESS) {
    // Spec says we should give it some extra space.
    *size += sizeof(EFI_MEMORY_DESCRIPTOR) * 2;

    status = systab->BootServices->AllocatePool(EfiLoaderData, *size, (void **)map);
    if (status != EFI_SUCCESS) {
      // Allocation failed, assume that the world has ended.
      return status;
    }

    status = systab->BootServices->GetMemoryMap(size, *map, key, descriptor_size, descriptor_version);
    if (status != EFI_SUCCESS) {
      systab->BootServices->FreePool(*map);
    }
  }

  return status;
}

void print_memory_map(EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
  UINTN memory_map_key;
  UINTN memmap_size;
  UINTN memmap_desc_size;
  UINT32 memmap_desc_version;

  UINTN total_mem = 0;

  status = get_memory_map(systab, &memmap_size, &memory_map, &memory_map_key, &memmap_desc_size, &memmap_desc_version);
  if (status == EFI_SUCCESS) {
    void *p = memory_map;
    void *end = p + memmap_size;
    EFI_MEMORY_DESCRIPTOR *md;
    for (; p < end; p += memmap_desc_size) {
      md = p;
      total_mem += md->NumberOfPages * 4096;
      Print(L"memmap entry T:%d P:%ld V:%ld PGS:%ld AT:%ld \r\n", md->Type, md->PhysicalStart, md->VirtualStart, md->NumberOfPages, md->Attribute);
    }
  }

  Print(L"Total system memory: %ld\r\n", total_mem);
}

EFI_STATUS file_read_from_loaded_image_root(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab, CHAR16 *name, CHAR8 **data, UINTN *size) {
  EFI_STATUS status;

  EFI_LOADED_IMAGE *loaded_image;
  status = systab->BootServices->OpenProtocol(image, &LoadedImageProtocol, (void **)&loaded_image, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (status != EFI_SUCCESS) {
    return status;
  }

  EFI_FILE *root_dir;
  root_dir = LibOpenRoot(loaded_image->DeviceHandle);
  if (!root_dir) {
    return EFI_LOAD_ERROR;
  }

  EFI_FILE_HANDLE handle;
  status = root_dir->Open(root_dir, &handle, name, EFI_FILE_MODE_READ, 0);

  if (status != EFI_SUCCESS) {
    return status;
  }

  EFI_FILE_INFO *info;
  UINTN file_size;
  info = LibFileInfo(handle);
  file_size = info->FileSize+1;
  FreePool(info);

  CHAR8 *file_data;
  status = systab->BootServices->AllocatePool(EfiLoaderData, file_size, (void **)&file_data);
  if (status != EFI_SUCCESS) {
    return status;
  }

  status = handle->Read(handle, &file_size, file_data);
  if (status != EFI_SUCCESS) {
    FreePool(file_data);
    return status;
  }

  handle->Close(handle);
  file_data[file_size] = '\0';
  *data = file_data;
  *size = file_size;
  return EFI_SUCCESS;
}

// We might fail the first time, due to ExitBootServices triggering callbacks
// that alter the memory map. So we should only try twice.
#define MAX_EXIT_BOOT_ATTEMPTS 2

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  InitializeLib(image, systab);
  EFI_STATUS status;
  SIMPLE_TEXT_OUTPUT_INTERFACE *con_out = systab->ConOut;
  con_out->OutputString(con_out, L"This is HALT!\r\n");

  print_memory_map(systab);


  UINTN size;
  CHAR16 *halt_image_location = L"\\halt\\halt.img";
  CHAR8 *data = NULL;
  status = file_read_from_loaded_image_root(image, systab, halt_image_location, &data, &size);
  if (status != EFI_SUCCESS) {
    Print(L"Unable to find HALT kernel image at ");
    Print(halt_image_location);
    Print(L"\r\n");
    return status;
  }

  Print(L"Successfully located HALT kernel image (%ld bytes)\r\n", size);

  int num_exit_boot_attempts = 0;
  EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
  UINTN memory_map_key;
  UINTN memmap_size;
  UINTN memmap_desc_size;
  UINT32 memmap_desc_version;

  status = EFI_LOAD_ERROR;
  while (status != EFI_SUCCESS && num_exit_boot_attempts < MAX_EXIT_BOOT_ATTEMPTS) {
    ++num_exit_boot_attempts;

    status = get_memory_map(systab, &memmap_size, &memory_map, &memory_map_key, &memmap_desc_size, &memmap_desc_version);
    if (status == EFI_SUCCESS) {
      status = systab->BootServices->ExitBootServices(image, memory_map_key);
    } else {
      systab->BootServices->FreePool(memory_map);
    }
  }

  if (status == EFI_SUCCESS) {
    void *p = memory_map;
    EFI_MEMORY_DESCRIPTOR *md;
    for (; p < p + memmap_size; p += memmap_desc_size) {
      md = p;
      switch (md->Type) {
      case EfiLoaderCode:
      case EfiLoaderData:
        // The Loader and/or OS may use this memory as they see fit. Note: the OS loader that called ExitBootServices() is utilizing one or more EfiLoaderCode ranges.
        break;
      case EfiBootServicesCode:
      case EfiBootServicesData:
      case EfiConventionalMemory:
        // Memory available for general use.
        break;
      default:
        break;
      }
      // Print(L"--> memmap entry T:%d P:%ld V:%ld PGS:%ld AT:%ld ", md->Type, md->PhysicalStart, md->VirtualStart, md->NumberOfPages, md->Attribute);
    }
  }

  // Don't just use the current memmap, we need to annotate it first in the loop above.
  // systab->RuntimeServices->SetVirtualAddressMap(memmap_size, memmap_desc_size, memmap_desc_version, memory_map);

  return status;
}
