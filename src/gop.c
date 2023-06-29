
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

VOID drawRect(
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop,
  UINTN x, UINTN y,
  UINTN w, UINTN h,
  UINT32 color
) {
  if ((x + w) > gop->Mode->Info->HorizontalResolution || 
      (y + h) > gop->Mode->Info->VerticalResolution ) return;

  const UINTN pixelsPerLine = gop->Mode->Info->PixelsPerScanLine;

  UINT32* base = (VOID*)gop->Mode->FrameBufferBase;
  base += (pixelsPerLine * y) + x;

  for (UINTN i = 0; i < h; ++i) {
    UINT32* offset = base + (pixelsPerLine * i);
    for (UINTN j = 0; j < w; ++j) {
      *(offset + j) = color;
    }
  }
}

