
#include <efi.h>

#include "gop.h"
#include "utils.h"

static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;

EFI_GRAPHICS_OUTPUT_PROTOCOL* getGop(VOID) {
    if (gop != NULL) return gop;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    okOrPanic(BS->LocateProtocol(&gopGuid, NULL, (void**)&gop));
    return gop;
}

VOID plotPixel(point_u32_t pos, color_t color) {
    if (gop == NULL) getGop();
    *((UINT32*)(
        gop->Mode->FrameBufferBase + 
        4 * gop->Mode->Info->PixelsPerScanLine * pos.y + 
        4 * pos.x
    )) = color.hex;
}
