#include <efi.h>

static CHAR16 *exampleText = L"This is HALT!";

EFI_STATUS efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab) {
  UINTN index;
  EFI_EVENT event = systab->ConIn->WaitForKey;
  SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = systab->ConOut;
  conOut->OutputString(conOut, exampleText);

  systab->BootServices->WaitForEvent(1, &event, &index);

  return EFI_SUCCESS;
}
