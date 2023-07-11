#include "utils.h"
#include <efi.h>
#include <stdarg.h>

EFI_SYSTEM_TABLE* ST;
EFI_BOOT_SERVICES* BS;
EFI_HANDLE imgHandle;

VOID* bmalloc(UINTN size) {
    void* buf;
    okOrPanic(BS->AllocatePool(EfiBootServicesData, size, &buf));
    return buf;
}

VOID bfree(VOID* buf) { okOrPanic(BS->FreePool(buf)); }

VOID waitForUser(VOID) {
    UINTN index;
    okOrPanic(ST->ConIn->Reset(ST->ConIn, FALSE));
    okOrPanic(BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &index));
}

void print(CHAR16* fmt, ...) {
    if (fmt == NULL) return;

    va_list args;
    va_start(args, fmt);

    CHAR16* str = fmt;
    BOOLEAN fmt_char = FALSE;
    while (*str) {
        if (fmt_char) {
            if (*str == L'x') {
                UINTN x = va_arg(args, UINTN);
                CHAR16 buf[32];
                uintToStr(x, buf, 16);
                puts(buf);
            } else if (*str == L'u') {
                UINTN x = va_arg(args, UINTN);
                CHAR16 buf[43];
                uintToStr(x, buf, 10);
                puts(buf);
            } else if (*str == L's') {
                CHAR16* x = va_arg(args, CHAR16*);
                puts(x);
            } else if (*str == L'%') {
                puts(L"%");
            }
            fmt_char = FALSE;
        } else if (*str == L'%') {
            fmt_char = TRUE;
        } else {
            puts((CHAR16[2]){*str, 0});
        }
        ++str;
    }

    va_end(args);
}

// caller is responsible for ensuring that buffer is sufficiently large, meaning
// that there is one char for each digit in the number for the base used (+1 for
// the null terminator); for example, a 43 byte buffer should always be
// sufficient for base 10.
CHAR16* uintToStr(UINTN num, CHAR16* buf, UINT8 base) {
    if (base > 16 || buf == NULL) goto END;

    // there's no good reason these values can't be computed instead of using
    // a lookup table, but this is more readable and faster, besides, it's only
    // 32 bytes.
    static const CHAR16 LOOKUP_TAB[16] = {
        L'0',
        L'1',
        L'2',
        L'3',
        L'4',
        L'5',
        L'6',
        L'7',
        L'8',
        L'9',
        L'A',
        L'B',
        L'C',
        L'D',
        L'E',
        L'F'};

    // special case for 0.
    if (num == 0) {
        buf[0] = L'0';
        buf[1] = L'\0';
        goto END;
    }

    // create string from number by modulo-ing by base to get least significant
    // digit, then dividing by base to shift the number one digit down.
    UINTN i = 0;
    while (num != 0) {
        buf[i] = LOOKUP_TAB[num % base];
        ++i;
        num /= base;
    }

    // since numbers are processed least significant to most significant the
    // string is created backwards and needs to be reversed.
    for (UINTN j = 0; j < (i / 2); ++j) {
        CHAR16 temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }

    // don't forget your null terminator!
    buf[i] = L'\0';

END:
    return buf;
}

UINTN strToUint(CHAR16* buf, UINT8 base) {
    if (base > 16 || buf == NULL) return -1;

    CHAR16* end = buf;
    while (*end != 0)
        ++end;

    UINTN x = 0;
    UINTN digit = 1;

    for (; end >= buf; --end) {
        CHAR16 ch = *end;
        if (ch >= L'0' && ch <= L'9') {
            x += (ch - L'0') * digit;
            digit *= base;
        } else if (ch >= L'a' && ch <= L'f') {
            x += (ch - L'a') * digit;
            digit *= base;
        } else if (ch >= L'A' && ch <= L'F') {
            x += (ch - L'A') * digit;
            digit *= base;
        }
    }

    return x;
}

CHAR16* readline(CHAR16* buf, UINTN bufSize) {
    ST->ConIn->Reset(ST->ConIn, FALSE);
    UINTN i = 0;
    while (TRUE) {
        EFI_INPUT_KEY key;
        EFI_STATUS status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
        if (status == (EFI_STATUS)EFI_NOT_READY) continue;

        if (key.UnicodeChar == CHAR_BACKSPACE) {
            if (i == 0) continue;
            --i;
        } else if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            ST->ConOut->EnableCursor(ST->ConOut, FALSE);
            puts(L"\r\n");
            buf[i] = 0; // null terminator
            break;
        } else if (i < bufSize - 1) {
            buf[i] = key.UnicodeChar;
            ++i;
        }

        CHAR16 str[2] = {key.UnicodeChar, 0};
        ST->ConOut->OutputString(ST->ConOut, str);
    }
    return buf;
}
