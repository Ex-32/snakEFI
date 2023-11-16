#include "gop.h"
#include "utils.h"
#include "version.h"
#include <efi/efi.h>

static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;

EFI_STATUS gopInit(void) {
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    return BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
}

EFI_STATUS setResolution(void) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    UINTN sizeOfInfo;
    UINTN numModes;
    UINTN nativeMode;

    EFI_STATUS status;
    status = gop->QueryMode(
        gop, gop->Mode ? gop->Mode->Mode : 0, &sizeOfInfo, &info
    );

    if (status == (EFI_STATUS)EFI_NOT_STARTED) gop->SetMode(gop, 0);

    if (EFI_ERROR(status)) {
        puts(L"\r\nunable to query native graphics mode :(\r\n");
        waitForUser();
        return status;
    }

    nativeMode = gop->Mode->Mode;
    numModes = gop->Mode->MaxMode;

    if (numModes > 1) {
        puts(L"Available Resolutions:\r\n");
        for (UINTN i = 0; i < numModes; ++i) {
            okOrPanic(gop->QueryMode(gop, i, &sizeOfInfo, &info));
            print(
                L" %s%u: %s%ux%u%s%s",
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
        if (numModes % 2 == 1) puts(L"\r\n");

    TRY_AGAIN:;
        puts(L"Select resolution (*current): ");
        (void) ST->ConOut->EnableCursor(ST->ConOut, TRUE);
        okOrPanic(ST->ConIn->Reset(ST->ConIn, FALSE));

        UINTN selection;

        static const UINTN BUF_SIZE = 100;
        CHAR16* buf = bmalloc(BUF_SIZE * sizeof(CHAR16));
        selection = strToUint(readline(buf, BUF_SIZE), 10);
        bfree(buf);

        if (selection > numModes) {
            print(L"display mode \"%u\" invalid!\r\n", selection);
            goto TRY_AGAIN;
        }
        okOrPanic(gop->SetMode(gop, selection));
        info = gop->Mode->Info;
        ST->ConOut->ClearScreen(ST->ConOut);
        puts(L"SnakEFI v" VERSION "\r\n");
    }
    return EFI_SUCCESS;
}

void drawRect(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color) {
    const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* const info = gop->Mode->Info;
    const UINTN pixelsPerLine = info->PixelsPerScanLine;

    UINT32* base = (void*)gop->Mode->FrameBufferBase;
    base += (pixelsPerLine * y) + x;

    for (UINTN i = 0; i < h; ++i) {
        UINT32* offset = base + (pixelsPerLine * i);
        for (UINTN j = 0; j < w; ++j) { *(offset + j) = color; }
    }
}
