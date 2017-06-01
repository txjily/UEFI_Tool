#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "SecBasicLib.h"
#include <Protocol\SimpleTextOut.h>

#define Header_Version 1

void
TitleString (
    void
)
{
    CHAR16      s[100];

    Swprintf(s, L"Get PCI Dump Info Utility V0.%d\n\r", Header_Version);
    gST->ConOut->OutputString(gST->ConOut, s);
    Swprintf(s, L"(c)2017 Copyright Samsung BIOS Tang\n\r\n\r");
    gST->ConOut->OutputString(gST->ConOut, s);
}

void
ClearScreen (
    void
)
{
    gST->ConOut->SetAttribute( gST->ConOut, (EFI_BACKGROUND_BLACK | EFI_WHITE));;
    gST->ConOut->ClearScreen(gST->ConOut);

}

void
ReadPci (
    void
)
{
    CHAR16      s[100];
    UINT8       Bus,    Dev,    Fun;
    UINT16  VendorID,   DeviceID;

    Swprintf(s, L"Bus#    Dev#      Fun#    VendorID    DeviceID\n\r");
    gST->ConOut->OutputString(gST->ConOut, s);
    
    for(Bus = 0;Bus < 0x14; Bus++)
    {
        for(Dev = 0;Dev < 0x1f; Dev++)
        {
            for(Fun = 0;Fun < 0x08; Fun++)
                if(SecReadPciRegDword(Bus, Dev, Fun, 0x00) != 0xFFFFFFFF)
                {
                    VendorID = SecReadPciRegWord(Bus, Dev, Fun, 0x00);
                    DeviceID = SecReadPciRegWord(Bus, Dev, Fun, 0x02);
                    Swprintf(s, L"%02d      %02d        %02d        %04x        %04x \n\r",Bus,Dev,Fun,VendorID,DeviceID);
                    gST->ConOut->OutputString(gST->ConOut, s);
                }
                    
        }
    }
}
//
// Module entry point
//
EFI_STATUS
EFIAPI
PciDumpEntryPoint (
    IN EFI_HANDLE                     ImageHandle,
    IN EFI_SYSTEM_TABLE               *SystemTable )
{
    ClearScreen();
    
    TitleString();
    
    ReadPci();
    
    return EFI_SUCCESS;
}
