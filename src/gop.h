#pragma once
#include "utils.h"
#include <efi.h>

extern EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
VOID gopInit(VOID);

VOID drawRect(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color);
