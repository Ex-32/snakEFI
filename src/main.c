#include "gop.h"
#include "rng.h"
#include "snake.h"
#include "utils.h"
#include "version.h"
#include <efi.h>

// entry point for EFI application
EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
    // in gnu-efi this would be handled by `InitializeLib(ih, st);`
    ST = st;
    BS = ST->BootServices;
    imgHandle = ih;

    okOrPanic(ST->ConOut->ClearScreen(ST->ConOut));
    puts(L"SnakEFI v" VERSION "\r\n~ press any key to load ~\r\n");
    waitForUser();

    puts(L"locating Input Ex Protocol...  ");
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx;
    {
        EFI_GUID inputExGuid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
        okOrPanic(BS->LocateProtocol(&inputExGuid, NULL, (void**)&inputEx));
    }
    puts(L"done!\r\n");

    puts(L"locating Graphics Output Protocol...  ");
    okOrPanic(gopInit());
    puts(L"done!\r\n");
    okOrPanic(setResolution());
    puts(L"~ press any key to start game ~\r\n");
    waitForUser();

    EFI_EVENT timer;
    UINTN index;
    snakeInit(inputEx);
    while (TRUE) {
        BS->CreateEvent(EVT_TIMER, TPL_APPLICATION, NULL, NULL, &timer);
        BS->SetTimer(timer, TimerRelative, 2000000 /*unit = 100ns*/);
        if (!snakeDoTick()) break;
        BS->WaitForEvent(1, &timer, &index);
        BS->CloseEvent(timer);
    }
    snakeDeinit(inputEx);
    drawRect(
        0,
        0,
        gop->Mode->Info->HorizontalResolution,
        gop->Mode->Info->VerticalResolution,
        0x000000
    );

    if (snakeWon) { puts(L" !! you won !!\r\n"); }
    print(L"Final score: %u\r\n", snakeLen);
    puts(L"~ press any key to exit ~\r\n");
    BS->Stall(500000);
    waitForUser();

    return EFI_SUCCESS;
}
