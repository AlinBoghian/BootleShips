#include <efi.h>
#include <efilib.h>
#include <stdio.h>
#include <utils.h>
#include <efiprot.h>

void init_gop()
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if(EFI_ERROR(status))
        Print(L"Unable to locate GOP");
    else
        Print(L"Found GOP2");
    
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;

    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
    // this is needed to get the current video mode
    if (status == EFI_NOT_STARTED)
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    if(EFI_ERROR(status)) {
        PrintLn(L"Unable to get native mode");
    } else {
        PrintLn("got native mode");
        nativeMode = gop->Mode->Mode;
        numModes = gop->Mode->MaxMode;
    }

    for (int i = 0; i < numModes; i++) {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
        if (i != nativeMode) {
            continue;
        }
        PrintLn(L"mode %03d width %d height %d format %x%s",
        i,
        info->HorizontalResolution,
        info->VerticalResolution,
        info->PixelFormat,
        i == nativeMode ? L"(current)" : L""
        );
    }
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;
    pixel.Blue = 0;
    pixel.Green = 0;
    pixel.Red = 100;
    uefi_call_wrapper(gop->Blt, 10, gop, &pixel, EfiBltVideoFill, 0 ,0, 0, 0, 50, 50, 0);
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    /* Store the system table for future use in other functions */
    ST = SystemTable;
    /* Say hi */
    Print(L"Hello World\r\n");
    if (EFI_ERROR(Status))
        return Status;

    init_gop();

    Status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(Status))
        return Status;

    /* Now wait until a key becomes available.  This is a simple
       polling implementation.  You could try and use the WaitForKey
       event instead if you like */
    while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ;


    return Status;
}