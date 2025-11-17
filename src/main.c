#include <efi.h>
#include <efilib.h>
#include <stdio.h>
#include <utils.h>
#include <efiprot.h>
#define MAT_SIZE 10

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

EFI_STATUS debug_preamble(EFI_HANDLE ImageHandle) {
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

    // Define the GUID variable (cannot pass macro directly)
    EFI_GUID LoadedImageProtocolGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    // Retrieve the Loaded Image Protocol using HandleProtocol
    EFI_STATUS status = uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &LoadedImageProtocolGUID, (void **)&loaded_image);
    if (EFI_ERROR(status)) {
        PrintLn("HandleProtocol failed: 0x%lx\n", status);
        return status;
    }

    // Print the actual base address of the loaded image
    PrintLn("Image loaded at: 0x%lx\n", (uint64_t)loaded_image->ImageBase);

    // Write image base and marker for GDB
    volatile uint64_t *marker_ptr = (uint64_t *)0x10000;
    volatile uint64_t *image_base_ptr = (uint64_t *)0x10008;
    *image_base_ptr = (uint64_t)loaded_image->ImageBase;  // Store ImageBase
    *marker_ptr = 0xDEADBEEF;   // Set marker

    PrintLn("Wrote deadbeef");

    return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    debug_preamble(ImageHandle);
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