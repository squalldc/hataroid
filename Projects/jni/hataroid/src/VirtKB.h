#ifndef VIRTKB_H_
#define VIRTKB_H_

#include <GLES2/gl2.h>
#include "VirtKBDefs.h"

enum
{
	ScreenZoom_Fit = 0,
	ScreenZoom_1,
	ScreenZoom_2,
	ScreenZoom_3,
	ScreenZoom_4,
	ScreenZoom_Stretch,
	ScreenZoom_NumOf
};

enum
{
	VkbZoom_Fit = 0,
	VkbZoom_1,
	VkbZoom_2,
	VkbZoom_NumOf
};

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
extern void VirtKB_setDefaultScreenZoomPreset();
extern void VirtKB_setExtraKeys(bool set);
extern void VirtKB_setObsessionKeys(bool set);
extern void VirtKB_setHideAll(bool set);
extern void VirtKB_setJoystickOnly(bool set);
extern void VirtKB_setHideExtraJoyKeys(bool hide);
extern void VirtKB_ResetAllInputPresses();

#endif /* VIRTKB_H_ */
