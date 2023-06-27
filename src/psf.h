#pragma once

#include <efi.h>

#include "utils.h"

#define PSF1_FONT_MAGIC 0x0436
 
typedef struct {
    UINT16 magic;        // Magic bytes for idnetiifcation.
    UINT8 fontMode;      // PSF font mode
    UINT8 characterSize; // PSF character size.
} PSF1_Header;
 
 
#define PSF_FONT_MAGIC 0x864ab572
 
typedef struct {
    UINT32 magic;         /* magic bytes to identify PSF */
    UINT32 version;       /* zero */
    UINT32 headersize;    /* offset of bitmaps in file, 32 */
    UINT32 flags;         /* 0 if there's no unicode table */
    UINT32 numglyph;      /* number of glyphs */
    UINT32 bytesperglyph; /* size of each glyph */
    UINT32 height;        /* height in pixels */
    UINT32 width;         /* width in pixels */
} PSF_font;

VOID loadPsfFont(EFI_FILE_HANDLE fontFile);
VOID putchar(CHAR16 ch, UINT32 cx, UINT32 cy, UINT32 fg, UINT32 bg);

