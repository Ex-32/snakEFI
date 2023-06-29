#pragma once

#include <efi.h>

#include "utils.h"
#include "x86_64/efibind.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL* getGop(VOID);

VOID drawRect(
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop,
  UINTN x, UINTN y,
  UINTN w, UINTN h,
  UINT32 color
);
