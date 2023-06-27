#pragma once

#include <efi.h>

EFI_FILE_HANDLE getRootDir(VOID);

UINTN fileSize(EFI_FILE_HANDLE file);
