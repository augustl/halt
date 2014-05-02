#include <efi.h>
#include <efilib.h>
#include <halt_elf.h>

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

EFI_STATUS file_read_from_loaded_image_root(EFI_LOADED_IMAGE *loaded_image, EFI_SYSTEM_TABLE *systab, CHAR16 *name, CHAR8 **data, UINTN *size) {
  EFI_STATUS status;

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

static void memset(void *s, int c, UINTN n) {
  char *p = s;
  while (n--) {
    *p++ = c;
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

static EFI_STATUS elf_validate_header(halt_elf_header *elf_header) {
  if (elf_header->e_ident[halt_elf_ident_magic_0] != 0x7f
      || elf_header->e_ident[halt_elf_ident_magic_1] != 'E'
      || elf_header->e_ident[halt_elf_ident_magic_2] != 'L'
      || elf_header->e_ident[halt_elf_ident_magic_3] != 'F') {
    return EFI_LOAD_ERROR;
  }

  if (elf_header->e_ident[halt_elf_ident_class] != halt_elf_class_64) {
    return EFI_LOAD_ERROR;
  }

  if (elf_header->e_ident[halt_elf_ident_endianness] != halt_elf_endianness_little) {
    return EFI_LOAD_ERROR;
  }

  if (elf_header->e_ident[halt_elf_ident_os_abi] != halt_elf_os_abi_system_v) {
    return EFI_LOAD_ERROR;
  }

  if (elf_header->e_ident[halt_elf_ident_version] != 1) {
    return EFI_LOAD_ERROR;
  }

  if (elf_header->e_type != halt_elf_type_executable) {
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

static EFI_STATUS elf_perform_load(halt_elf_header *elf_header) {
  int i;
  halt_elf_program_header *program_headers = (halt_elf_program_header *)((void *)elf_header + elf_header->e_phoff);
  for (i = 0; i < elf_header->e_phnum; i++) {
    halt_elf_program_header *program_header = &program_headers[i];

    memcpy((void *)program_header->p_vaddr, ((void *)elf_header + program_header->p_offset), program_header->p_filesz);
    uint64_t remainder = program_header->p_memsz - program_header->p_filesz;
    if (remainder > 0) {
      memset((void *)program_header->p_vaddr + program_header->p_filesz, 0, remainder);
    }
  }

  return EFI_SUCCESS;
}

static EFI_STATUS elf_validate_max_addr(halt_elf_header *elf_header, UINTN max_addr) {
  int i;
  halt_elf_program_header *program_headers = (halt_elf_program_header *)((void *)elf_header + elf_header->e_phoff);
  for (i = 0; i < elf_header->e_phnum; i++) {
    halt_elf_program_header *program_header = &program_headers[i];
    if (program_header->p_vaddr + program_header->p_memsz > max_addr) {
      return EFI_LOAD_ERROR;
    }
  }

  return EFI_SUCCESS;
}

static EFI_STATUS elf_get_entry_point(halt_elf_header *elf_header, uint64_t *entry_point) {
  int i;
  halt_elf_program_header *program_headers = (halt_elf_program_header *)((void *)elf_header + elf_header->e_phoff);
  for (i = 0; i < elf_header->e_phnum; i++) {
    halt_elf_program_header *program_header = &program_headers[i];
    if (program_header->p_type == halt_elf_program_header_type_load) {
      memcpy(entry_point, &program_header->p_vaddr, sizeof(uint64_t));
      return EFI_SUCCESS;
    }
  }

  return EFI_LOAD_ERROR;
}

static EFI_STATUS load_elf(void *data, uint64_t size, uint64_t max_addr, uint64_t *entry_point) {
  EFI_STATUS status;
  halt_elf_header *elf_header = (halt_elf_header*)data;

  status = elf_validate_header(elf_header);
  if (status != EFI_SUCCESS) {
    return status;
  }

  status = elf_validate_max_addr(elf_header, max_addr);
  if (status != EFI_SUCCESS) {
    return status;
  }

  status = elf_get_entry_point(elf_header, entry_point);
  if (status != EFI_SUCCESS) {
    return status;
  }

  return elf_perform_load(elf_header);
}

// We might fail the first time, due to ExitBootServices triggering callbacks
// that alter the memory map. So we should only try twice.
#define MAX_EXIT_BOOT_ATTEMPTS 2

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  InitializeLib(image, systab);
  EFI_STATUS status;
  Print(L"This is HALT!\r\n");

  EFI_LOADED_IMAGE *loaded_image;
  status = systab->BootServices->OpenProtocol(image, &LoadedImageProtocol, (void **)&loaded_image, image, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (status != EFI_SUCCESS) {
    return status;
  }

  Print(L"Boot loader base: 0x%lx\r\n", loaded_image->ImageBase);

  UINTN halt_image_size;
  CHAR16 *halt_image_location = L"\\halt\\halt_kernel.elf";
  CHAR8 *halt_image_data = NULL;
  status = file_read_from_loaded_image_root(loaded_image, systab, halt_image_location, &halt_image_data, &halt_image_size);
  if (status != EFI_SUCCESS) {
    Print(L"Unable to find HALT kernel image at ");
    Print(halt_image_location);
    Print(L"\r\n");
    return status;
  }

  Print(L"Successfully located HALT kernel image (%ld bytes, addr 0x%lx)\r\n", halt_image_size, halt_image_location);

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
  // Figure out if/why we actually need to do this.
  // systab->RuntimeServices->SetVirtualAddressMap(memmap_size, memmap_desc_size, memmap_desc_version, memory_map);

  EFI_MEMORY_DESCRIPTOR *first_memmap_item = memory_map;
  if (first_memmap_item->PhysicalStart != 0) {
    return EFI_LOAD_ERROR;
  }

  UINTN first_segment_num_bytes = first_memmap_item->NumberOfPages * 4096;
  UINTN halt_init_struct_size = 1024; // We don't yet know the actual size.
  uint64_t kernel_entry_point = 0;
  status = load_elf(halt_image_data, halt_image_size, first_segment_num_bytes + halt_init_struct_size, &kernel_entry_point);
  if (status != EFI_SUCCESS) {
    return status;
  }

  int wait = 1;
  while (wait) {
    __asm__ __volatile__("pause");
  }

  return EFI_SUCCESS;
}
