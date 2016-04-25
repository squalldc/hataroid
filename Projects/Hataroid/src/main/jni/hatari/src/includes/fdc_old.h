/*
  Hatari - fdc_old.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_FDC_OLD_H
#define HATARI_FDC_OLD_H

extern void	FDC_MemorySnapShot_Capture_Old ( bool bSave );
extern void	FDC_Init_Old ( void );
extern void	FDC_Reset_Old ( bool bCold );
extern void	FDC_SetDMAStatus_Old ( bool bError );

extern void	FDC_SetIRQ_Old ( Uint8 IRQ_Source );
extern void	FDC_ClearIRQ_Old ( void );
extern void	FDC_ClearHdcIRQ_Old (void);
extern void	FDC_InterruptHandler_Update_Old ( void );

extern void	FDC_Drive_Set_BusyLed_Old ( Uint8 SR );
extern int	FDC_Get_Statusbar_Text_Old ( char *text, size_t maxlen );
extern void	FDC_Drive_Set_Enable_Old ( int Drive , bool value );
extern void	FDC_Drive_Set_NumberOfHeads_Old ( int Drive , int NbrHeads );
extern void	FDC_InsertFloppy_Old ( int Drive );
extern void	FDC_EjectFloppy_Old ( int Drive );
extern void	FDC_SetDriveSide_Old ( Uint8 io_porta_old , Uint8 io_porta_Old );
extern int	FDC_GetBytesPerTrack_Old ( int Drive );

extern int	FDC_IndexPulse_GetCurrentPos_FdcCycles_Old ( Uint32 *pFdcCyclesPerRev );
extern int	FDC_IndexPulse_GetCurrentPos_NbBytes_Old ( void );
extern int	FDC_IndexPulse_GetState_Old ( void );
extern int	FDC_NextIndexPulse_FdcCycles_Old ( void );

extern Uint8	FDC_GetCmdType_Old ( Uint8 CR );

extern void	FDC_DiskController_WriteWord_Old ( void );
extern void	FDC_DiskControllerStatus_ReadWord_Old ( void );
extern void	FDC_DmaModeControl_WriteWord_Old ( void );
extern void	FDC_DmaStatus_ReadWord_Old ( void );
extern int	FDC_DMA_GetModeControl_R_WR_Old ( void );
extern void	FDC_DMA_FIFO_Push_Old ( Uint8 Byte );
extern Uint8	FDC_DMA_FIFO_Pull_Old ( void );

extern void	FDC_Buffer_Reset_Old ( void );
extern void	FDC_Buffer_Add_Timing_Old ( Uint8 Byte , Uint16 Timing );
extern void	FDC_Buffer_Add_Old ( Uint8 Byte );
extern Uint16	FDC_Buffer_Read_Timing_Old ( void );
extern Uint8	FDC_Buffer_Read_Byte_Old ( void );
extern Uint8	FDC_Buffer_Read_Byte_pos_Old ( int pos );
extern int	FDC_Buffer_Get_Size_Old ( void );

extern void	FDC_DmaAddress_ReadByte_Old ( void );
extern void	FDC_DmaAddress_WriteByte_Old ( void );
extern Uint32	FDC_GetDMAAddress_Old ( void );
extern void	FDC_WriteDMAAddress_Old ( Uint32 Address );

extern void	FDC_FloppyMode_ReadByte_Old ( void );
extern void	FDC_FloppyMode_WriteByte_Old ( void );

#endif /* ifndef HATARI_FDC_OLD_H */
