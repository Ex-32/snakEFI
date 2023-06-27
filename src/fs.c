
#include <efi.h>

#include "fs.h"
#include "utils.h"

EFI_FILE_HANDLE getRootDir(VOID) {
    static EFI_FILE_HANDLE volume = NULL;
    if (volume != NULL) return volume;
   
    {
        EFI_LOADED_IMAGE* loadedImg = NULL;
        EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
        EFI_FILE_IO_INTERFACE* ioVolume;
        EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

        BS->HandleProtocol(imgHandle, &lipGuid, (VOID**)&loadedImg);
        BS->HandleProtocol(loadedImg->DeviceHandle, &fsGuid, (VOID**)&ioVolume);
        ioVolume->OpenVolume(ioVolume, &volume);
    }
    return volume;
}

UINT64 fileSize(EFI_FILE_HANDLE file) {
    static EFI_GUID fileInfoGuid = EFI_FILE_INFO_ID;
    UINTN size = SIZE_OF_EFI_FILE_INFO + 64;
    EFI_STATUS status;
    EFI_FILE_INFO* info; 

    BUF_TOO_SMALL: 
    info = bmalloc(size);

    status = file->GetInfo(file, &fileInfoGuid, &size, info);
    if (status == (EFI_STATUS)EFI_BUFFER_TOO_SMALL) {
        free(info);
        goto BUF_TOO_SMALL;
    }
    okOrPanic(status);

    UINT64 ret = info->FileSize;
    free(info); 
    return ret;
}

