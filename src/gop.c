#include <efi.h>
#include "gop.h"
#include "utils.h"

static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;

VOID gopInit(VOID) {
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    okOrPanic(BS->LocateProtocol(&gopGuid, NULL, (void**)&gop));
}

VOID drawRect(
  UINT16 x, UINT16 y,
  UINT16 w, UINT16 h,
  UINT32 color
) {
  const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* const info = gop->Mode->Info;
  if ((x + w) > info->HorizontalResolution ||
      (y + h) > info->VerticalResolution ) return;

  const UINTN pixelsPerLine = info->PixelsPerScanLine;

  UINT32* base = (VOID*)gop->Mode->FrameBufferBase;
  base += (pixelsPerLine * y) + x;

  for (UINTN i = 0; i < h; ++i) {
    UINT32* offset = base + (pixelsPerLine * i);
    for (UINTN j = 0; j < w; ++j) {
      *(offset + j) = color;
    }
  }
}

