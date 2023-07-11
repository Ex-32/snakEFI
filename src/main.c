#include "gop.h"
#include "rng.h"
#include "snake.h"
#include "utils.h"
#include "version.h"
#include <efi.h>

// entry point for EFI application
EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
    // in gnu-efi this would be handled by `InitalizeLib(ih, st);`
    ST = st;
    BS = ST->BootServices;
    imgHandle = ih;

    okOrPanic(ST->ConOut->ClearScreen(ST->ConOut));
    puts(L"SnakEFI v" VERSION "\r\n~ press any key to load ~\r\n");
    waitForUser();

    puts(L"locating Input Ex Protocol...  ");
    EFI_GUID inputExGuid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx;
    okOrPanic(BS->LocateProtocol(&inputExGuid, NULL, (VOID**)&inputEx));
    puts(L"done!\r\n");

    puts(L"locating Graphics Output Protocol...  ");
    gopInit();
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    UINTN sizeOfInfo;
    UINTN numModes;
    UINTN nativeMode;
    {
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
    }
    nativeMode = gop->Mode->Mode;
    numModes = gop->Mode->MaxMode;
    puts(L"done!\r\n");

    if (numModes > 1) {
        puts(L"Avalible Resolutions:\r\n");
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

    TRY_AGAIN:;

        puts(L"Select resolution (*current): ");
        ST->ConOut->EnableCursor(ST->ConOut, TRUE);
        ST->ConIn->Reset(ST->ConIn, FALSE);

        UINTN selection;
        {
            static const UINTN BUF_SIZE = 100;
            CHAR16* buf = bmalloc(BUF_SIZE * sizeof(CHAR16));
            selection = strToUint(readline(buf, BUF_SIZE), 10);
            bfree(buf);
        }

        if (selection > numModes) {
            print(L"display mode \"%u\" invalid!\r\n", selection);
            goto TRY_AGAIN;
        }
        okOrPanic(gop->SetMode(gop, selection));
        info = gop->Mode->Info;
        ST->ConOut->ClearScreen(ST->ConOut);
        puts(L"SnakEFI v" VERSION "\r\n");
    }
    puts(L"~ press any key to start game ~\r\n");
    waitForUser();

    EFI_EVENT timer;
    UINTN index;
    snakeInit(inputEx);
    while (TRUE) {
        BS->CreateEvent(EVT_TIMER, TPL_APPLICATION, NULL, NULL, &timer);
        BS->SetTimer(timer, TimerRelative, 2000000 /*unit = 100ns*/);
        snakeDoTick();
        if (!snakeRunning) break;
        BS->WaitForEvent(1, &timer, &index);
        BS->CloseEvent(timer);
    }
    snakeDeinit(inputEx);
    drawRect(
        0, 0, info->HorizontalResolution, info->VerticalResolution, 0x000000
    );

    if (snakeWon) { puts(L" !! you won !!\r\n"); }
    print(L"Final score: %u\r\n", snakeLen);
    puts(L"~ press any key to exit ~\r\n");
    BS->Stall(500000);
    waitForUser();

    return EFI_SUCCESS;
}
