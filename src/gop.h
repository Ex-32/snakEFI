#pragma once

#include <efi.h>

#include "utils.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL* getGop(VOID);

VOID plotPixel(point_u32_t pos, color_t color); 
