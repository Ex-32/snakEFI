#pragma once
#include "utils.h"
#include <efi/efi.h>

// this value is initialized to NULL, and must be initalized by calling
// `gopInit()` and receiving `EFI_SUCCESS`
extern EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

EFI_STATUS gopInit(void);
EFI_STATUS setResolution(void);
void drawRect(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color);
