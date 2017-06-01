#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol\SimpleTextOut.h>
#include <Protocol\Smbus.h>

#define Header_Version 1
#define SPD_PAGE_ADDRESS_0           0x6C
#define SPD_PAGE_ADDRESS_1           0x6E

void
TitleString (
    void
)
{
    CHAR16      s[100];

    Swprintf(s, L"Get SPD Dump Info Utility V0.%d(now only support DDR3/DDR4 module).(c)2017 Copyright Samsung BIOS Tang\n\r", Header_Version);
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

//----------------------------------------------------------------------------
// Name:    GetSpdDataByte 
//
// Description: Read Spd Data byte
//----------------------------------------------------------------------------
EFI_STATUS GetSpdDataByte(EFI_SMBUS_HC_PROTOCOL* SmbusController, UINT8 SlaveAddr, UINT8 SmbusOffset, UINT8 *pValue)
{
    EFI_STATUS  Status;
    UINTN       SmbusLength = 1;
    EFI_SMBUS_DEVICE_ADDRESS SmbusSlaveAddress;

    SmbusSlaveAddress.SmbusDeviceAddress = SlaveAddr >> 1;

    Status = SmbusController->Execute(
                                SmbusController,
                                SmbusSlaveAddress,
                                SmbusOffset,
                                EfiSmbusReadByte,
                                FALSE,
                                &SmbusLength,
                                pValue);

    return Status;
}

//----------------------------------------------------------------------------
// Name:    SetSpdDataByte 
//
// Description: Write SPD data byte
//----------------------------------------------------------------------------
EFI_STATUS SetSpdDataByte(EFI_SMBUS_HC_PROTOCOL* SmbusController, UINT8 SlaveAddr, UINT8 SmbusOffset, UINT8 *pValue)
{
    EFI_STATUS  Status;
    UINTN       SmbusLength = 1;
    EFI_SMBUS_DEVICE_ADDRESS SmbusSlaveAddress;

    SmbusSlaveAddress.SmbusDeviceAddress = SlaveAddr >> 1;

    Status = SmbusController->Execute(
                                SmbusController,
                                SmbusSlaveAddress,
                                SmbusOffset,
                                EfiSmbusWriteByte,
                                FALSE,
                                &SmbusLength,
                                pValue);

    return Status;
}

EFI_STATUS
ReadSpd (EFI_SMBUS_HC_PROTOCOL* SmbusController, UINT8 SlaveAddr)
{
    CHAR16      s[100];
    EFI_STATUS  Status;
    UINT8   MemoryType, bData, bData1, i, j;
 
    SetSpdDataByte(SmbusController, SPD_PAGE_ADDRESS_0, 0, &bData);
    Status = GetSpdDataByte(SmbusController, SlaveAddr, 0x02, &MemoryType);
    if(EFI_ERROR(Status)) 
    {
        Swprintf(s, L"\n\rSlaveAddr %02X SPD data read fail\n\r",SlaveAddr);
        gST->ConOut->OutputString(gST->ConOut, s);
        return  Status;
    }
    else
    {
        if((MemoryType == 0x0B) ||(MemoryType == 0xF1)) // DDR3/LPDDR3 case
        {
            gST->ConOut->OutputString(gST->ConOut, L"MemoryType is DDR3\n\r");

            //set 1st Page
            SetSpdDataByte(SmbusController, SPD_PAGE_ADDRESS_0, 0, &bData);
           
            for (i = 0; i <= 0x0f; i++) {
                Swprintf(s, L"\n\r%02X",(i * 0x10));
                gST->ConOut->OutputString(gST->ConOut, s);
                for(j = 0; j <= 0xf; j++ )
                {
                    GetSpdDataByte(SmbusController, SlaveAddr, ((i * 0x10) + j), &bData);
                    Swprintf(s, L" %02X ",bData);
                    gST->ConOut->OutputString(gST->ConOut, s);
                }
            }

        }
        else if(MemoryType == 0x0C)
        {
            gST->ConOut->OutputString(gST->ConOut, L"\rMemoryType is DDR4\n\r");
            
            Swprintf(s, L"Page 1:\n\r");
             gST->ConOut->OutputString(gST->ConOut, s);

             //set 1st Page
             SetSpdDataByte(SmbusController, SPD_PAGE_ADDRESS_0, 0, &bData);
            
             for (i = 0; i <= 0x0f; i++) {
                 Swprintf(s, L"\n\r%02X",(i * 0x10));
                 gST->ConOut->OutputString(gST->ConOut, s);
                 for(j = 0; j <= 0xf; j++ )
                 {
                     GetSpdDataByte(SmbusController, SlaveAddr, ((i * 0x10) + j), &bData);
                     Swprintf(s, L" %02X ",bData);
                     gST->ConOut->OutputString(gST->ConOut, s);
                 }
             }
             
             Swprintf(s, L"\n\rPage 2:");
             gST->ConOut->OutputString(gST->ConOut, s);

             //set 2nd Page
             SetSpdDataByte(SmbusController, SPD_PAGE_ADDRESS_1, 0, &bData1);

             for (i = 0; i <= 0xF; i++) {
                 Swprintf(s, L"\n\r%02X",(i * 0x10));
                 gST->ConOut->OutputString(gST->ConOut, s);
                 for(j = 0; j <= 0xf; j++ )
                 {
                     GetSpdDataByte(SmbusController, SlaveAddr, ((i * 0x10) + j), &bData1);
                     Swprintf(s, L" %02X ",bData1);
                     gST->ConOut->OutputString(gST->ConOut, s);
                 }
             }

        }
        else
        {
            gST->ConOut->OutputString(gST->ConOut, L"MemoryType is unknown\n\r");

        }
         
    }
    
 
    gST->ConOut->OutputString(gST->ConOut, L"\n\r");

    return EFI_SUCCESS;

}
//
// Module entry point
//
EFI_STATUS
EFIAPI
SpdDumpEntryPoint (
    IN EFI_HANDLE                     ImageHandle,
    IN EFI_SYSTEM_TABLE               *SystemTable )
{
    EFI_STATUS                Status;
    EFI_SMBUS_HC_PROTOCOL *SmbusController=NULL;
    UINT8 SlaveAddr;
    CHAR16      s[100];
    
    InitAmiLib(ImageHandle, SystemTable);

    Status = pBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, &SmbusController);
    if(EFI_ERROR(Status)) return Status;

    ClearScreen();
    
    TitleString();

    
    for (SlaveAddr = 0xA0; SlaveAddr <= 0xA6; SlaveAddr+=2)
    {
        Swprintf(s, L"\nSlave Address %02X dump:\n",SlaveAddr);
        gST->ConOut->OutputString(gST->ConOut, s);
        ReadSpd(SmbusController,SlaveAddr);
    }

    return EFI_SUCCESS;
}
