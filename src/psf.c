
#include <efi.h>

#include "gop.h"
#include "psf.h"
#include "utils.h"
#include "fs.h"

static PSF_font* font = NULL;
static UINTN fontBufSize;

#define PIXEL UINT32

VOID loadPsfFont(EFI_FILE_HANDLE fontFile) {
    fontBufSize = fileSize(fontFile);
    VOID* fileBuf = bmalloc(fontBufSize);
    okOrPanic(fontFile->Read(fontFile, &fontBufSize, fileBuf));
    
    font = fileBuf;

    if (font->magic != PSF_FONT_MAGIC) {
        puts(L"invalid magic bytes in font file!\r\n");
    }
}

// TODO add proper error handing if no font file
VOID putchar(CHAR16 ch, UINT32 cx, UINT32 cy, UINT32 fg, UINT32 bg) {
    if (font == NULL) return;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = getGop();

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    UINTN sizeOfInfo;

    okOrPanic(gop->QueryMode(gop, gop->Mode->Mode, &sizeOfInfo, &info));

    UINTN scanline = info->PixelsPerScanLine * sizeof(PIXEL);

    UINTN bytesPerLine = (font->width + 7) / 8;

    UINT8* glyph = (VOID*)(((UINTN)font) + font->headersize +
        (((ch > 0 && ch < font->numglyph) ? ch : 0) * (font->bytesperglyph)));

    UINTN offset = 
        (cy * font->height * scanline) +
        (cx * (font->width + 1) * sizeof(PIXEL));

    UINTN line;
    UINTN mask;
    for (UINTN y = 0; y < font->height; ++y) {
        line = offset;
        mask = 1 << (font->width - 1);

        for (UINTN x = 0; x < font->width; ++x) {
            *((PIXEL*)(gop->Mode->FrameBufferBase + line)) = *(UINT32*)glyph & mask ? fg : bg; 
            mask >>= 1;
            line += sizeof(PIXEL);
        }
        glyph += bytesPerLine;
        offset += scanline;
    }
}

