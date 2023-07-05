#pragma once
#include <efi.h>

extern EFI_SYSTEM_TABLE* ST;
extern EFI_BOOT_SERVICES* BS;

extern EFI_HANDLE imgHandle;

// exits with `status` if it's an error; quick and dirty error handling
static inline VOID okOrPanic(EFI_STATUS status) {
    if (EFI_ERROR(status)) BS->Exit(imgHandle, status, 0, NULL);
}

VOID* bmalloc(UINTN size);
VOID  bfree(VOID* buf);

// prints a L-string to console out using boot services
static inline VOID puts(CHAR16* str) {
    okOrPanic(ST->ConOut->OutputString(ST->ConOut, str));
}

VOID waitForUser(VOID);

VOID print(CHAR16* fmt, ...);

CHAR16* uintToStr(UINTN num, CHAR16* buf, UINT8 base);
UINTN strToUint(CHAR16* buf, UINT8 base);


CHAR16* readline(CHAR16* buf, UINTN bufSize);

