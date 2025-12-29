#include <efi.h>
#include <efilib.h>
#include <stdio.h>
#include <utils.h>
#include <efiprot.h>

#include "game.h"
#include "graphics.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
void drawBlock(int width, int height, int leftX, int upperY, color color) {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixelColor = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) {.Red = color.red, .Green = color.green, .Blue = color.blue};
    uefi_call_wrapper(gop->Blt, 10, gop, &pixelColor, EfiBltVideoFill, 0 ,0, leftX, upperY, width, height, 0);
}

uint16_t read_key_blocking_impl() {
    EFI_INPUT_KEY Key;
    while (uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key) == EFI_NOT_READY);
    return Key.UnicodeChar;
}

EFI_GRAPHICS_OUTPUT_PROTOCOL *init_gop(struct graphics_context *context)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if(EFI_ERROR(status))
        Print(L"Unable to locate GOP");
    
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;

    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
    // this is needed to get the current video mode
    if (status == EFI_NOT_STARTED)
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    if(EFI_ERROR(status)) {
        PrintLn(L"Unable to get native mode");
    } else {
        nativeMode = gop->Mode->Mode;
        numModes = gop->Mode->MaxMode;
    }

    for (int i = 0; i < numModes; i++) {
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
        if (i != nativeMode) {
            continue;
        }
        context->height = info->VerticalResolution;
        context->width = info->HorizontalResolution;
    }
    context->draw_block = drawBlock;
    return gop;
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
    return EFI_SUCCESS;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    debug_preamble(ImageHandle);
    EFI_STATUS Status = EFI_SUCCESS;

    /* Store the system table for future use in other functions */
    ST = SystemTable;
    /* Say hi */
    Print(L"Hello World\r\n");

    struct graphics_context context;
    init_gop(&context);
    start_game(context, &read_key_blocking_impl);

    return Status;
}