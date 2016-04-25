/*
  Hatari - fdc_compat.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_FDC_COMPAT_H
#define HATARI_FDC_COMPAT_H

enum
{
    FDC_CompatMode_Default   = 0,
    FDC_CompatMode_Old
};

extern void FDC_Compat_SetCompatMode(int compatMode);
extern int FDC_Compat_GetCompatMode();

#endif /* ifndef HATARI_FDC_COMPAT_H */
