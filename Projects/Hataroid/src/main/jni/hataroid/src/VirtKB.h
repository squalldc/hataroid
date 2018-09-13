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

enum
{
	InputLayer_None = 0,
	InputLayer_Zoom,
	InputLayer_QuickKeys,
	InputLayer_VirtKB,
	InputLayer_VirtController,
};

enum
{
	kLocale_EN = 0,
	kLocale_DE,
	kLocale_FR,

	kLocale_Numof
};

#define VKB_MaxTouches 5


extern void VirtKB_OnSurfaceCreated();
extern int VirtKB_OnSurfaceChanged(int width, int height, int prevW, int prevH);
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
extern void VirtKB_SetJoystickAlpha(float alpha);
extern void VirtKB_SetJoystickSize(float size);
extern void VirtKB_SetJoystickFireSize(float size);
extern void VirtKB_setScreenZoomMode(bool set);
extern void VirtKB_setDefaultScreenZoomPreset();
//extern void VirtKB_setExtraKeys(bool set);
extern void VirtKB_setObsessionKeys(bool set);
extern void VirtKB_setAutoHide(bool set);
extern void VirtKB_setHideAll(bool set);
extern void VirtKB_setJoystickOnly(bool set);
extern void VirtKB_setHideExtraJoyKeys(bool hide);
extern void VirtKB_setHideShortcutKeys(bool hide);
//extern void VirtKB_setHideTurboKeys(bool hide);
extern void VirtKB_ResetAllInputPresses();

extern float VirtKB_getVKBZoom();
extern float VirtKB_getVKBPanX();
extern float VirtKB_getVKBPanY();
extern void VirtKB_SetVKBPanZoom(float kbdZoom, float kbdPanX, float kbdPanY);
extern void VirtKB_setDefaultVKBZoomPreset();
extern void VirtKB_SetNonTouchKB(bool nonTouch);

extern bool VirtKB_getMouseActive();
extern void VirtKB_SetMouseActive(bool mouseActive);

extern void VirtKB_setShortcutKeysFromPrefs(const char *pref);

extern void VirtKB_setHideJoystick(bool set);

extern void VirtKB_SetKeySizeVX(float size);
extern void VirtKB_SetKeySizeVY(float size);
extern void VirtKB_SetKeySizeHX(float size);
extern void VirtKB_SetKeySizeHY(float size);

extern const SpriteDef* VirtKB_findSpriteDef(const char *name);

extern void VirtKB_VJStickEnable(bool useVJStick);
extern void VirtKB_VJStickSetFloating(bool vjStickFloating);
extern void VirtKB_VJStickSetDeadZone(float size);
extern void VirtKB_VJStickSetDiagSensitivity(float sensitivity);

extern int VirtKB_FindLocaleID(const char* localeStr);
extern int VirtKB_GetVKBRegion();
extern void VirtKB_SetVKBRegion(const char* localeStr);

#endif /* VIRTKB_H_ */
