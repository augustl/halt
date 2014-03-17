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

CHAR16* get_memory_map_type_string(UINT32 type) {
  switch (type) {
  case EfiReservedMemoryType: return L"EfiReservedMemoryType";
  case EfiLoaderCode: return L"EfiLoaderCode";
  case EfiLoaderData: return L"EfiLoaderData";
  case EfiBootServicesCode: return L"EfiBootServicesCode";
  case EfiBootServicesData: return L"EfiBootServicesData";
  case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
  case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
  case EfiConventionalMemory: return L"EfiConventionalMemory";
  case EfiUnusableMemory: return L"EfiUnusableMemory";
  case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
  case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
  case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
  case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
  case EfiPalCode: return L"EfiPalCode";
  case EfiMaxMemoryType: return L"EfiMaxMemoryType";
  default: return L"UNKNOWN";
  }
}

void print_memory_descriptor(EFI_MEMORY_DESCRIPTOR *md) {
  Print(L"memmap entry T:%s P:%ld V:%ld PGS:%ld AT:%ld", get_memory_map_type_string(md->Type), md->PhysicalStart, md->VirtualStart, md->NumberOfPages, md->Attribute);
}

void print_memory_map(EFI_MEMORY_DESCRIPTOR *memory_map, UINTN memmap_size, UINTN memmap_desc_size) {
  UINTN total_mem = 0;

  void *p = memory_map;
  void *end = p + memmap_size;
  EFI_MEMORY_DESCRIPTOR *md;
  for (; p < end; p += memmap_desc_size) {
    md = p;
    total_mem += md->NumberOfPages * 4096;
    print_memory_descriptor(md);
    Print(L"\r\n");
  }

  Print(L"Total map memory: %ld\r\n", total_mem);
}

void print_current_memory_map(EFI_SYSTEM_TABLE *systab) {
  EFI_STATUS status;
  EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
  UINTN memory_map_key;
  UINTN memmap_size;
  UINTN memmap_desc_size;
  UINT32 memmap_desc_version;

  status = get_memory_map(systab, &memmap_size, &memory_map, &memory_map_key, &memmap_desc_size, &memmap_desc_version);
  if (status == EFI_SUCCESS) {
    print_memory_map(memory_map, memmap_size, memmap_desc_size);
  }
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
  file_size = info->FileSize;
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
  *data = file_data;
  *size = file_size;
  return EFI_SUCCESS;
}

static void memcpy(void *dest, const void *src, UINTN count) {
  char* dst8 = (char*)dest;
  char* src8 = (char*)src;

  while (count--) {
    *dst8++ = *src8++;
  }
}

static int is_mergeable_type(UINT32 type) {
  return type == EfiBootServicesCode || type == EfiBootServicesData || type == EfiConventionalMemory;
}

static void merge_memory_map(EFI_MEMORY_DESCRIPTOR *memory_map, UINTN *memmap_size, UINTN memmap_desc_size) {
  void *p = memory_map;
  void *memmap_end = p + *memmap_size;
  EFI_MEMORY_DESCRIPTOR *md = NULL;
  EFI_MEMORY_DESCRIPTOR *prev_md = memory_map;
  EFI_MEMORY_DESCRIPTOR *curr_new_md = memory_map;
  p += memmap_desc_size;
  for (; p < memmap_end; p += memmap_desc_size) {
    md = p;

    if (is_mergeable_type(prev_md->Type) && is_mergeable_type(md->Type) &&
        prev_md->Attribute == prev_md->Attribute) {
      curr_new_md->Type = EfiConventionalMemory;
      curr_new_md->NumberOfPages += md->NumberOfPages;
      *memmap_size -= memmap_desc_size;
    } else {
      void *curr_p = curr_new_md;
      curr_p += memmap_desc_size;
      curr_new_md = curr_p;
      memcpy(curr_p, p, memmap_desc_size);
    }

    prev_md = md;
  }

}

// We might fail the first time, due to ExitBootServices triggering callbacks
// that alter the memory map. So we should only try twice.
#define MAX_EXIT_BOOT_ATTEMPTS 2

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  InitializeLib(image, systab);
  EFI_STATUS status;
  Print(L"This is HALT!\r\n");

  print_current_memory_map(systab);


  UINTN halt_image_size;
  CHAR16 *halt_image_location = L"\\halt\\halt_kernel.elf";
  CHAR8 *halt_image_data = NULL;
  status = file_read_from_loaded_image_root(image, systab, halt_image_location, &halt_image_data, &halt_image_size);
  if (status != EFI_SUCCESS) {
    Print(L"Unable to find HALT kernel image at ");
    Print(halt_image_location);
    Print(L"\r\n");
    return status;
  }

  Print(L"Successfully located HALT kernel image (%ld bytes)\r\n", halt_image_size);

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

  if (status != EFI_SUCCESS) {
    Print(L"Failed to exit boot services\r\n");
    return status;
  }

  merge_memory_map(memory_map, &memmap_size, memmap_desc_size);
  systab->RuntimeServices->SetVirtualAddressMap(memmap_size, memmap_desc_size, memmap_desc_version, memory_map);
  return EFI_SUCCESS;
}
