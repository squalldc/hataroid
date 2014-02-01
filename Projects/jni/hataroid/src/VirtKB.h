#ifndef VIRTKB_H_
#define VIRTKB_H_

#include <GLES2/gl2.h>
#include "VirtKBDefs.h"

extern int VirtKB_OnSurfaceChanged(int width, int height);
extern void VirtKB_CleanUp();
extern void VirtKB_EnableInput(bool enable);
extern void VirtKB_RefreshKB();
extern void VirtKB_SetJoystickPort(int port);
extern int VirtKB_GetJoystickPort();
extern void VirtKB_MapJoysticksToArrowKeys(bool map);
extern void VirtKB_SetMouseEmuDirect();
extern void VirtKB_SetMouseEmuButtons();
extern void VirtKB_SetMouseEmuSpeed(float speed);
extern void VirtKB_SetControlAlpha(float alpha);
extern void VirtKB_SetJoystickSize(float size);
extern void VirtKB_setScreenZoomMode(bool set);
extern void VirtKB_setExtraKeys(bool set);
extern void VirtKB_setObsessionKeys(bool set);

#endif /* VIRTKB_H_ */
