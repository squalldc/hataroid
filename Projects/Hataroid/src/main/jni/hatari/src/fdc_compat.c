#include "main.h"
#include "configuration.h"
#include "fdc.h"
#include "fdc_old.h"
#include "fdc_new.h"
#include "fdc_compat.h"

static bool _FDC_UseCompatMode = false;

void FDC_Compat_SetCompatMode(int compatMode)
{
    _FDC_UseCompatMode = (compatMode == FDC_CompatMode_Old) ? true : false;
}
int FDC_Compat_GetCompatMode()
{
    return _FDC_UseCompatMode ? FDC_CompatMode_Old : FDC_CompatMode_Default;
}



void FDC_MemorySnapShot_Capture ( bool bSave )
{
    if (_FDC_UseCompatMode) {
        FDC_MemorySnapShot_Capture_Old(bSave);
    } else {
        FDC_MemorySnapShot_Capture_New(bSave);
    }
    //(*( (void (*)(bool)) _FDC_FnTables[FDC_FN_INIT]))(bSave);
}

void FDC_Init ( void )
{
    // call both - note: these functions should use their own internal functions and not generic fdc_* functions
    FDC_Init_Old();
    FDC_Init_New();
}

void FDC_Reset ( bool bCold )
{
    // call both - note: these functions should use their own internal functions and not generic fdc_* functions
    FDC_Reset_Old(bCold);
    FDC_Reset_New(bCold);
}

void FDC_SetDMAStatus ( bool bError )
{
    if (_FDC_UseCompatMode) {
        FDC_SetDMAStatus_Old(bError);
    } else {
        FDC_SetDMAStatus_New(bError);
    }
    //(*( (void (*)(bool)) _FDC_FnTables[FDC_FN_SETDMASTATUS]))(bError);
}

void FDC_SetIRQ ( Uint8 IRQ_Source )
{
    if (_FDC_UseCompatMode) {
        FDC_SetIRQ_Old(IRQ_Source);
    } else {
        FDC_SetIRQ_New(IRQ_Source);
    }
    //(*( (void (*)(Uint8)) _FDC_FnTables[FDC_FN_SETIRQ]))(IRQ_Source);
}

void FDC_ClearIRQ ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_ClearIRQ_Old();
    } else {
        FDC_ClearIRQ_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_CLEARIRQ]))();
}

void FDC_ClearHdcIRQ(void)
{
    if (_FDC_UseCompatMode) {
        FDC_ClearHdcIRQ_Old();
    } else {
        FDC_ClearHdcIRQ_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_CLEARHDCIRQ]))();
}

void FDC_InterruptHandler_Update ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_InterruptHandler_Update_Old();
    } else {
        FDC_InterruptHandler_Update_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_INTERRUPTHANDLER_UPDATE]))();
}

void FDC_Drive_Set_BusyLed ( Uint8 SR )
{
    if (_FDC_UseCompatMode) {
        FDC_Drive_Set_BusyLed_Old(SR);
    } else {
        FDC_Drive_Set_BusyLed_New(SR);
    }
    //(*( (void (*)(Uint8)) _FDC_FnTables[FDC_FN_DRIVE_SET_BUSYLED]))(SR);
}

int	FDC_Get_Statusbar_Text ( char *text, size_t maxlen )
{
    if (_FDC_UseCompatMode) {
        return FDC_Get_Statusbar_Text_Old(text, maxlen);
    } else {
        return FDC_Get_Statusbar_Text_New(text, maxlen);
    }
    //return (*( (int (*)(char*, size_t)) _FDC_FnTables[FDC_FN_GET_STATUSBAR_TEXT]))(text, maxlen);
}

int	FDC_Get_CurTrack ()
{
    if (_FDC_UseCompatMode) {
        return FDC_Get_CurTrack_Old();
    } else {
        return FDC_Get_CurTrack_New();
    }
}

bool FDC_Get_HasActiveCommands ()
{
    if (_FDC_UseCompatMode) {
        return FDC_Get_HasActiveCommands_Old();
    } else {
        return FDC_Get_HasActiveCommands_New();
    }
}

void FDC_Drive_Set_Enable ( int Drive , bool value )
{
    if (_FDC_UseCompatMode) {
        FDC_Drive_Set_Enable_Old(Drive, value);
    } else {
        FDC_Drive_Set_Enable_New(Drive, value);
    }
    //(*( (void (*)(int, bool)) _FDC_FnTables[FDC_FN_DRIVE_SET_ENABLE]))(Drive, value);
}

void FDC_Drive_Set_NumberOfHeads ( int Drive , int NbrHeads )
{
    if (_FDC_UseCompatMode) {
        FDC_Drive_Set_NumberOfHeads_Old(Drive, NbrHeads);
    } else {
        FDC_Drive_Set_NumberOfHeads_New(Drive, NbrHeads);
    }
    //(*( (void (*)(int, int)) _FDC_FnTables[FDC_FN_DRIVE_SET_NUMBEROFHEADS]))(Drive, NbrHeads);
}

void FDC_InsertFloppy ( int Drive )
{
    if (_FDC_UseCompatMode) {
        FDC_InsertFloppy_Old(Drive);
    } else {
        FDC_InsertFloppy_New(Drive);
    }
    //(*( (void (*)(int)) _FDC_FnTables[FDC_FN_INSERTFLOPPY]))(Drive);
}

void FDC_EjectFloppy ( int Drive )
{
    if (_FDC_UseCompatMode) {
        FDC_EjectFloppy_Old(Drive);
    } else {
        FDC_EjectFloppy_New(Drive);
    }
    //(*( (void (*)(int)) _FDC_FnTables[FDC_FN_EJECTFLOPPY]))(Drive);
}

void FDC_SetDriveSide ( Uint8 io_porta_old , Uint8 io_porta_new )
{
    if (_FDC_UseCompatMode) {
        FDC_SetDriveSide_Old(io_porta_old, io_porta_new);
    } else {
        FDC_SetDriveSide_New(io_porta_old, io_porta_new);
    }
    //(*( (void (*)(Uint8, Uint8)) _FDC_FnTables[FDC_FN_SETDRIVESIDE]))(io_porta_old, io_porta_new);
}

int	FDC_GetBytesPerTrack ( int Drive )
{
    if (_FDC_UseCompatMode) {
        return FDC_GetBytesPerTrack_Old(Drive);
    } else {
        return FDC_GetBytesPerTrack_New(Drive);
    }
    //return (*( (int (*)(int)) _FDC_FnTables[FDC_FN_GETBYTESPERTRACK]))(Drive);
}

int	FDC_IndexPulse_GetCurrentPos_FdcCycles ( Uint32 *pFdcCyclesPerRev )
{
    if (_FDC_UseCompatMode) {
        return FDC_IndexPulse_GetCurrentPos_FdcCycles_Old(pFdcCyclesPerRev);
    } else {
        return FDC_IndexPulse_GetCurrentPos_FdcCycles_New(pFdcCyclesPerRev);
    }
    //return (*( (int (*)(Uint32*)) _FDC_FnTables[FDC_FN_INDEXPULSE_GETCURRENTPOS_FDCCYCLES]))(pFdcCyclesPerRev);
}

int	FDC_IndexPulse_GetCurrentPos_NbBytes ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_IndexPulse_GetCurrentPos_NbBytes_Old();
    } else {
        return FDC_IndexPulse_GetCurrentPos_NbBytes_New();
    }
    //return (*( (int (*)()) _FDC_FnTables[FDC_FN_INDEXPULSE_GETCURRENTPOS_NBBYTES]))();
}

int	FDC_IndexPulse_GetState ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_IndexPulse_GetState_Old();
    } else {
        return FDC_IndexPulse_GetState_New();
    }
    //return (*( (int (*)()) _FDC_FnTables[FDC_FN_INDEXPULSE_GETSTATE]))();
}

int	FDC_NextIndexPulse_FdcCycles ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_NextIndexPulse_FdcCycles_Old();
    } else {
        return FDC_NextIndexPulse_FdcCycles_New();
    }
    //return (*( (int (*)()) _FDC_FnTables[FDC_FN_NEXTINDEXPULSE_FDCCYCLES]))();
}


Uint8 FDC_GetCmdType ( Uint8 CR )
{
    if (_FDC_UseCompatMode) {
        return FDC_GetCmdType_Old(CR);
    } else {
        return FDC_GetCmdType_New(CR);
    }
    //return (*( (Uint8 (*)(Uint8)) _FDC_FnTables[FDC_FN_GETCMDTYPE]))(CR);
}


void FDC_DiskController_WriteWord ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DiskController_WriteWord_Old();
    } else {
        FDC_DiskController_WriteWord_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DISKCONTROLLER_WRITEWORD]))();
}

void FDC_DiskControllerStatus_ReadWord ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DiskControllerStatus_ReadWord_Old();
    } else {
        FDC_DiskControllerStatus_ReadWord_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DISKCONTROLLERSTATUS_READWORD]))();
}

void FDC_DmaModeControl_WriteWord ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DmaModeControl_WriteWord_Old();
    } else {
        FDC_DmaModeControl_WriteWord_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DMAMODECONTROL_WRITEWORD]))();
}

void FDC_DmaStatus_ReadWord ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DmaStatus_ReadWord_Old();
    } else {
        FDC_DmaStatus_ReadWord_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DMASTATUS_READWORD]))();
}

int	FDC_DMA_GetModeControl_R_WR ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_DMA_GetModeControl_R_WR_Old();
    } else {
        return FDC_DMA_GetModeControl_R_WR_New();
    }
    //return (*( (int (*)()) _FDC_FnTables[FDC_FN_DMA_GETMODECONTROL_R_WR]))();
}

void FDC_DMA_FIFO_Push ( Uint8 Byte )
{
    if (_FDC_UseCompatMode) {
        FDC_DMA_FIFO_Push_Old(Byte);
    } else {
        FDC_DMA_FIFO_Push_New(Byte);
    }
    //(*( (void (*)(Uint8)) _FDC_FnTables[FDC_FN_DMA_FIFO_PUSH]))(Byte);
}

Uint8 FDC_DMA_FIFO_Pull ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_DMA_FIFO_Pull_Old();
    } else {
        return FDC_DMA_FIFO_Pull_New();
    }
    //return (*( (Uint8 (*)()) _FDC_FnTables[FDC_FN_DMA_FIFO_PULL]))();
}


void FDC_Buffer_Reset ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_Buffer_Reset_Old();
    } else {
        FDC_Buffer_Reset_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_BUFFER_RESET]))();
}

void FDC_Buffer_Add_Timing ( Uint8 Byte , Uint16 Timing )
{
    if (_FDC_UseCompatMode) {
        FDC_Buffer_Add_Timing_Old(Byte, Timing);
    } else {
        FDC_Buffer_Add_Timing_New(Byte, Timing);
    }
    //(*( (void (*)(Uint8, Uint16)) _FDC_FnTables[FDC_FN_BUFFER_ADD_TIMING]))(Byte, Timing);
}

void FDC_Buffer_Add ( Uint8 Byte )
{
    if (_FDC_UseCompatMode) {
        FDC_Buffer_Add_Old(Byte);
    } else {
        FDC_Buffer_Add_New(Byte);
    }
    //(*( (void (*)(Uint8)) _FDC_FnTables[FDC_FN_BUFFER_ADD]))(Byte);
}

Uint16 FDC_Buffer_Read_Timing ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_Buffer_Read_Timing_Old();
    } else {
        return FDC_Buffer_Read_Timing_New();
    }
    //return (*( (Uint16 (*)()) _FDC_FnTables[FDC_FN_BUFFER_READ_TIMING]))();
}

Uint8 FDC_Buffer_Read_Byte ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_Buffer_Read_Byte_Old();
    } else {
        return FDC_Buffer_Read_Byte_New();
    }
    //return (*( (Uint8 (*)()) _FDC_FnTables[FDC_FN_BUFFER_READ_BYTE]))();
}

Uint8 FDC_Buffer_Read_Byte_pos ( int pos )
{
    if (_FDC_UseCompatMode) {
        return FDC_Buffer_Read_Byte_pos_Old(pos);
    } else {
        return FDC_Buffer_Read_Byte_pos_New(pos);
    }
    //return (*( (Uint8 (*)(int)) _FDC_FnTables[FDC_FN_BUFFER_READ_BYTE_POS]))(pos);
}

int	FDC_Buffer_Get_Size ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_Buffer_Get_Size_Old();
    } else {
        return FDC_Buffer_Get_Size_New();
    }
    //return (*( (int (*)()) _FDC_FnTables[FDC_FN_BUFFER_GET_SIZE]))();
}


void FDC_DmaAddress_ReadByte ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DmaAddress_ReadByte_Old();
    } else {
        FDC_DmaAddress_ReadByte_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DMAADDRESS_READBYTE]))();
}

void FDC_DmaAddress_WriteByte ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_DmaAddress_WriteByte_Old();
    } else {
        FDC_DmaAddress_WriteByte_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_DMAADDRESS_WRITEBYTE]))();
}

Uint32 FDC_GetDMAAddress ( void )
{
    if (_FDC_UseCompatMode) {
        return FDC_GetDMAAddress_Old();
    } else {
        return FDC_GetDMAAddress_New();
    }
    //return (*( (Uint32 (*)()) _FDC_FnTables[FDC_FN_GETDMAADDRESS]))();
}

void FDC_WriteDMAAddress ( Uint32 Address )
{
    if (_FDC_UseCompatMode) {
        FDC_WriteDMAAddress_Old(Address);
    } else {
        FDC_WriteDMAAddress_New(Address);
    }
    //(*( (void (*)(Uint32)) _FDC_FnTables[FDC_FN_WRITEDMAADDRESS]))(Address);
}


void FDC_FloppyMode_ReadByte ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_FloppyMode_ReadByte_Old();
    } else {
        FDC_FloppyMode_ReadByte_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_FLOPPYMODE_READBYTE]))();
}

void FDC_FloppyMode_WriteByte ( void )
{
    if (_FDC_UseCompatMode) {
        FDC_FloppyMode_WriteByte_Old();
    } else {
        FDC_FloppyMode_WriteByte_New();
    }
    //(*( (void (*)()) _FDC_FnTables[FDC_FN_FLOPPYMODE_WRITEBYTE]))();
}

