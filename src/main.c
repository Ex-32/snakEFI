
#include <stdarg.h>

#include <efi.h>

#include "version.h"
#include "utils.h"
#include "gop.h"

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
    // in gnu-efi this would be handled by InitalizeLib(ih, st);
    ST = st;  
    BS = ST->BootServices;
    imgHandle = ih;

    okOrPanic(ST->ConOut->ClearScreen(ST->ConOut));
    puts(L"SnakEFI v" VERSION "\r\n~ press any key to continue ~\r\n");

    waitForUser();
    
    puts(L"locating Graphics Output Protocol...  ");
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = getGop();
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    EFI_STATUS status;
    UINTN sizeOfInfo;
    UINTN numModes;
    UINTN nativeMode;

    status = gop->QueryMode(gop, gop->Mode? gop->Mode->Mode : 0, &sizeOfInfo, &info);

    if (status == (EFI_STATUS)EFI_NOT_STARTED) {
        gop->SetMode(gop, 0);
    }

    if (EFI_ERROR(status)) {
        puts(L"\r\nunable to query native graphics mode :(\r\n");
        waitForUser();
        return status;
    }
    nativeMode = gop->Mode->Mode;
    numModes = gop->Mode->MaxMode;
    puts(L"done!\r\n");

    puts(L"Avalible Resolutions:\r\n");
    for (UINTN i = 0; i < numModes; ++i) {
        okOrPanic(gop->QueryMode(gop, i, &sizeOfInfo, &info));
        print(L" %s%u: %s%ux%u%s%s",
            i == nativeMode ? L"*" : L" ",
            i,
            i < 10 ? L" " : L"",
            info->HorizontalResolution,
            info->VerticalResolution,
            info->HorizontalResolution < 1000 ? L" " : L"",
            info->VerticalResolution < 1000 ? L" " : L""
        );
        if (i % 2 == 1) puts(L"\r\n");
    }

    TRY_AGAIN:;

    puts(L"Select resolution (*current): ");
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);
    ST->ConIn->Reset(ST->ConIn, FALSE);

    static const UINTN BUF_SIZE = 100;
    CHAR16* buf = bmalloc(BUF_SIZE * sizeof(CHAR16));
    UINTN i = 0;

    while (TRUE) {
        EFI_INPUT_KEY key;
        EFI_STATUS status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
        if (status == (EFI_STATUS)EFI_NOT_READY) continue;

        if (key.UnicodeChar == CHAR_BACKSPACE) {
            if (i <= 0) continue;
            --i;
        } else if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            ST->ConOut->EnableCursor(ST->ConOut, FALSE);
            puts(L"\r\n");
            buf[i] = 0; // null terminator
            break;
        } else if (i < BUF_SIZE - 1) {
            buf[i] = key.UnicodeChar;
            ++i;
        }

        CHAR16 str[2] = { key.UnicodeChar, 0 };
        ST->ConOut->OutputString(ST->ConOut, str); 
    }

    UINTN selection = strToUint(buf, 10); 
    bfree(buf);
    buf = NULL;
    
    if (selection > numModes) {
        print(L"display mode \"%u\" invalid!\r\n", selection);
        goto TRY_AGAIN;
    }

    okOrPanic(gop->SetMode(gop, selection));
    ST->ConOut->ClearScreen(ST->ConOut);



   //  puts(L"getting filesystem handle...  ");
   //  EFI_FILE_HANDLE rootDir = getRootDir();
   //  puts(L"done!\r\n");
   //
   //  print(L"loading font file \"%s\"...  ", FONT16);
   //  EFI_FILE_HANDLE fontFile;
   //
   //  rootDir->Open(
   //      rootDir, 
   //      &fontFile, 
   //      FONT16, 
   //      EFI_FILE_MODE_READ, 
   //      EFI_FILE_READ_ONLY // <- this only applys with EFI_FILE_MODE_CREATE
   //  );
   //
   //  loadPsfFont(fontFile); 
   //  puts(L"done!\r\n");
   //  waitForUser();
   // 
   //  CHAR16* hello = L"Hello, World!";
   //  for (UINTN i = 0; hello[i] > 0; ++i) {
   //      putchar(hello[i], i, 0, 0xFF0000u, 0u);
   //  }

    puts(L"\r\n~ reached end of efi_main() ~\r\n");
    waitForUser();
    return EFI_SUCCESS;
}

