#include <efi.h>
#include <efilib.h>
#include <stdio.h>

#define NL "\r\n"


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    /* Store the system table for future use in other functions */
    ST = SystemTable;
    /* Say hi */
    print(L"Hello World\r\n");
    if (EFI_ERROR(Status))
        return Status;

    Status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(Status))
        return Status;

    /* Now wait until a key becomes available.  This is a simple
       polling implementation.  You could try and use the WaitForKey
       event instead if you like */
    while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ;


    return Status;
}