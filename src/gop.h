#pragma once
#include <efi.h>
#include "utils.h"

extern EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
VOID gopInit(VOID);

VOID drawRect(
  UINT16 x, UINT16 y,
  UINT16 w, UINT16 h,
  UINT32 color
);
