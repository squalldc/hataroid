/*
  Hatari - fdc_new.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_FDC_NEW_H
#define HATARI_FDC_NEW_H


extern void	FDC_MemorySnapShot_Capture_New ( bool bSave );
extern void	FDC_Init_New ( void );
extern void	FDC_Reset_New ( bool bCold );
extern void	FDC_SetDMAStatus_New ( bool bError );

extern void	FDC_SetIRQ_New ( Uint8 IRQ_Source );
extern void	FDC_ClearIRQ_New ( void );
extern void	FDC_ClearHdcIRQ_New (void);
extern void	FDC_InterruptHandler_Update_New ( void );

extern void	FDC_Drive_Set_BusyLed_New ( Uint8 SR );
extern int	FDC_Get_Statusbar_Text_New ( char *text, size_t maxlen );
extern void	FDC_Drive_Set_Enable_New ( int Drive , bool value );
extern void	FDC_Drive_Set_NumberOfHeads_New ( int Drive , int NbrHeads );
extern void	FDC_InsertFloppy_New ( int Drive );
extern void	FDC_EjectFloppy_New ( int Drive );
extern void	FDC_SetDriveSide_New ( Uint8 io_porta_old , Uint8 io_porta_new );
extern int	FDC_GetBytesPerTrack_New ( int Drive );

extern int	FDC_IndexPulse_GetCurrentPos_FdcCycles_New ( Uint32 *pFdcCyclesPerRev );
extern int	FDC_IndexPulse_GetCurrentPos_NbBytes_New ( void );
extern int	FDC_IndexPulse_GetState_New ( void );
extern int	FDC_NextIndexPulse_FdcCycles_New ( void );

extern Uint8	FDC_GetCmdType_New ( Uint8 CR );

extern void	FDC_DiskController_WriteWord_New ( void );
extern void	FDC_DiskControllerStatus_ReadWord_New ( void );
extern void	FDC_DmaModeControl_WriteWord_New ( void );
extern void	FDC_DmaStatus_ReadWord_New ( void );
extern int	FDC_DMA_GetModeControl_R_WR_New ( void );
extern void	FDC_DMA_FIFO_Push_New ( Uint8 Byte );
extern Uint8	FDC_DMA_FIFO_Pull_New ( void );

extern void	FDC_Buffer_Reset_New ( void );
extern void	FDC_Buffer_Add_Timing_New ( Uint8 Byte , Uint16 Timing );
extern void	FDC_Buffer_Add_New ( Uint8 Byte );
extern Uint16	FDC_Buffer_Read_Timing_New ( void );
extern Uint8	FDC_Buffer_Read_Byte_New ( void );
extern Uint8	FDC_Buffer_Read_Byte_pos_New ( int pos );
extern int	FDC_Buffer_Get_Size_New ( void );

extern void	FDC_DmaAddress_ReadByte_New ( void );
extern void	FDC_DmaAddress_WriteByte_New ( void );
extern Uint32	FDC_GetDMAAddress_New ( void );
extern void	FDC_WriteDMAAddress_New ( Uint32 Address );

extern void	FDC_FloppyMode_ReadByte_New ( void );
extern void	FDC_FloppyMode_WriteByte_New ( void );

#endif /* ifndef HATARI_FDC_NEW_H */
