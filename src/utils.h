#pragma once
#include <efi.h>

extern EFI_SYSTEM_TABLE* ST;
extern EFI_BOOT_SERVICES* BS;
extern EFI_HANDLE imgHandle;

// exits with `status` if it's an error; quick and dirty error handling
static inline void okOrPanic(EFI_STATUS status) {
    if (EFI_ERROR(status)) BS->Exit(imgHandle, status, 0, NULL);
}

// stdlib malloc-like allocator based on the boot service allocator; not
// guaranteed to be performant
void* bmalloc(UINTN size);

// stdlib free-like deallocator used to free memory allocated with `bmalloc()`
void bfree(void* buf);

// prints a L-string to console out using boot services
static inline void puts(CHAR16* str) {
    okOrPanic(ST->ConOut->OutputString(ST->ConOut, str));
}

// pauses execution until the user presses a key
void waitForUser(void);

// printf-like function; supported formatters:
// - %%: literal '%'
// - %u: UINTN (decimal)
// - %x: UINTN (hex)
// - %s: CHAR16* (string)
void print(CHAR16* fmt, ...);

// converts UINTN to a CHAR16* string in the specified base; base must be <= 16.
// caller is responsible for ensuring that buffer is sufficiently large, meaning
// that there is one char for each digit in the number for the base used (+1 for
// the null terminator); for example, a 43 byte buffer should always be
// sufficient for base 10.
CHAR16* uintToStr(UINTN num, CHAR16* buf, UINT8 base);

// parses a CHAR16* string containing a number into a UINTN in the specified
// base; base must be <= 16.
UINTN strToUint(CHAR16* buf, UINT8 base);

// prompts the user for a string.
CHAR16* readline(CHAR16* buf, UINTN bufSize);
