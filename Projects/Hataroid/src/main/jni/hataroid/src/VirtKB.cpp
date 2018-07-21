#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <SDL.h>

#include "VirtKBDefs.h"
#include "VirtKBTex.h"
#include "VirtJoy.h"
#include "uncompressGZ.h"
#include "nativeRenderer_ogles2.h"
#include "ShortcutMap.h"
#include "VirtKB.h"
#include "RTShader.h"
#include "BitFlags.h"
#include "hataroid.h"

extern "C"
{
	#include <ikbd.h>
	#include <joy.h>
	#include <screen.h>
	#include <video.h>
	#include <configuration.h>

	extern void hatari_onMouseMoved(int dx, int dy);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_updateInput(JNIEnv * env, jobject obj,
																						jboolean t0, jfloat tx0, jfloat ty0,
																						jboolean t1, jfloat tx1, jfloat ty1,
																						jboolean t2, jfloat tx2, jfloat ty2,
																						jfloat mouseX, jfloat mouseY, jint mouseBtns,
																						jintArray keyPresses, jfloatArray curAxis);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleMouseActive(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetMouseActive(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetScreenScalePreset(JNIEnv * env, jobject obj, jint preset);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleVirtKeyboard(JNIEnv * env, jobject obj, jboolean nonTouch);

	extern volatile int g_doubleBuffer;
};

#define MAKE_DWORD(lo16, hi16)				((((hi16)&0xffff)<<16)|((lo16)&0xffff))
#define GET_LOWORD(dw32)					((dw32)&0xffff)
#define GET_HIWORD(dw32)					(((dw32)>>16)&0xffff)

#define MAKE_BUTTON_DOWN_SET(vkID, qkID)	MAKE_DWORD(vkID, qkID)
#define GET_BUTTON_DOWN_VKID(id)			GET_LOWORD(id)
#define GET_BUTTON_DOWN_QKID(id)			GET_HIWORD(id)

#define INVALID_QUICK_KEY_ID				(0xffff)

#define ST_SCANCODE_UPARROW		0x48
#define ST_SCANCODE_DOWNARROW	0x50
#define ST_SCANCODE_RIGHTARROW	0x4D
#define ST_SCANCODE_LEFTARROW	0x4B

#define AUTOHIDE_DELAY_SECS     1.0f

static const int kAxis_X1			= 0;
static const int kAxis_Y1			= 1;
static const int kAxis_X2			= 2;
static const int kAxis_Y2			= 3;
static const int kNumAxis			= 4;

struct QuickKey;
typedef void (*OnKeyEvent)(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);

struct KeyCallback
{
	uint32_t uParam1;
	OnKeyEvent onKeyEvent;
};

struct QuickKey
{
	enum {
		PANEL_NONE = 0,
		PANEL_TL,
		PANEL_TR,
		PANEL_BL,
		PANEL_BR
	};

	int x1, y1;
	int x2, y2;

	float rx1, ry1;
	float rx2, ry2;

	float tx1, ty1;
	float tx2, ty2;

	int panel;

	const VirtKeyDef *pKeyDef;
};

#define DEFAULT_INPUT_FLAGS (FLAG_PERSIST|FLAG_JOY|FLAG_SCREEN|FLAG_STKEY|FLAG_STFNKEY|FLAG_MAIN)

static volatile bool	s_InputReady = false;
static volatile bool	s_InputEnabled = false;

static int          g_vkbTexKbW = 0;
static int          g_vkbTexKbH = 0;

static JNIEnv*      s_curEnv = 0;
static bool         s_isPortrait = false;

static bool			s_showKeyboard = false;
static bool			s_keyboardZoomMode = false;
static bool			s_screenZoomMode = false;
static unsigned int s_curInputFlags = DEFAULT_INPUT_FLAGS;
static unsigned int s_prevInputFlags = DEFAULT_INPUT_FLAGS;
static int			s_joyID = 1;
static bool			s_joyMapToArrowKeys = false;
static int			s_prevZoomPanCount = 0;
static bool			s_waitInputCleared = false;
static float		s_joystickSize = 1.3f;
static float		s_joystickFireSize = 1.3f;
static bool			s_vkbObsessionKeys = false;
//static bool			s_vkbExtraKeys = false;

static GLuint		s_KbTextureID = 0;
static int			s_VkbGPUTexWidth = 0;
static int			s_VkbGPUTexHeight = 0;

static const int	s_MaxNumQuickKeys = 20;
static QuickKey		s_QuickKeys[s_MaxNumQuickKeys];
static int			s_numQuickKeys = 0;

static bool			s_recreateQuickKeys = false;
static GLfloat		s_QuickKeyVerts[36*s_MaxNumQuickKeys];
static GLushort		s_QuickKeyIndices[6*s_MaxNumQuickKeys];
static GLsizei		s_QuickKeyStride = 9 * sizeof(GLfloat); // 3 position, 4 color, 2 texture
static float		s_QuickKeyAlpha = 0.65f;
static float		s_QuickKeyLum = 1.0f;
static float		s_joystickAlpha = 0.65f;

static bool			s_curtouched[2][VKB_MaxTouches] = { false };
static float		s_curtouchX[2][VKB_MaxTouches] = {0};
static float		s_curtouchY[2][VKB_MaxTouches] = {0};

static BitFlags*	s_curButtons[2] = {0};
static BitFlags*	s_changedButtons = 0;

static const int	MaxButtonDown = 10;
static int			s_buttonDown[2][MaxButtonDown];
static int			s_numButtonDown[2] = {0};

static int			s_curIndex = 0;
static int          s_curInputLayer = InputLayer_None;

static GLfloat		s_VkbVerts[36];
static GLushort		s_VkbIndices[6] = { 0, 1, 2, 0, 2, 3 };
static GLsizei		s_VkbStride = 9 * sizeof(GLfloat); // 3 position, 4 color, 2 texture
static float		s_VkbAlpha = 0.65f;
static float		s_VkbLum = 1.0f;

static const int	s_MaxVkbPresses = 20;
static GLfloat		s_VkbPressedVerts[36*s_MaxVkbPresses];
static GLushort		s_VkbPressedIndices[6*s_MaxVkbPresses];
static GLsizei		s_VkbPressedStride = 9 * sizeof(GLfloat); // 3 position, 4 color, 2 texture
static int			s_VkbCurNumPresses = 0;
//static float		s_VkbPressAlpha = 0.1f;
//static float		s_VkbPressLum = 0.3f;

static float		s_VkbMinZoom = 0.3f;
static bool			s_VkbZoomInited = false;
static float		s_vkbZoom = 1.0f;
static float		s_vkbPanX = 0;
static float		s_vkbPanY = 0;

static int			s_curScreenZoomPreset = -1;
static int			s_curKeyboardZoomPreset = VkbZoom_Fit;

static int			s_mouseButtonIgnoreQuickKeyIdx[2]; // HACK
static float		s_mouseSpeed = 1;

static bool         s_autoHide = false;
static bool         s_autoHideInput = false;
static float        s_autoHideAlpha = 1;
static float        s_autoHideDelay = 0;
static long         s_autoHideTicks = 0;

static bool			s_hideAll = false;
static bool			s_showJoystickOnly = false;
static bool			s_hideExtraJoyKeys = false;
static bool			s_hideShortcutKeys = false;
//static bool			s_hideTurboKey = false;

static const long	kInitKBRepeatDelay = 150;
static const long	kMinKBRepeatDelay = 25;
static const long	kKBRepeatDelayDeltaRate = 10;

static bool			s_nonTouchKB = false;
static int			s_curKBKeyFocus = VKB_KEY_SPACE;
static Uint32		s_lastKBFocusRepeat = 0;
static Uint32		s_KBRepeatDelay = kInitKBRepeatDelay;

static bool			s_hideJoystick = false;
static float		s_keySizeVX = 1;
static float		s_keySizeVY = 1;
static float		s_keySizeHX = 1;
static float		s_keySizeHY = 1;

static KeyCallback*	s_keyCallbacks = 0;

static BitFlags*	s_jKeyPresses = new BitFlags(VKB_KEY_NumOf);
static float*		s_jAxis = new float [kNumAxis];

static ShortcutMap*	s_shortcutMap = new ShortcutMap();

static int			s_mouseFinger = -1;
static int			s_mouseMoved = 0;
static float		s_prevMouseX = 0;
static float		s_prevMouseY = 0;
static float		s_mouseDx = 0;
static float		s_mouseDy = 0;

static bool			s_LMB = false;
static bool			s_RMB = false;

static int			s_prevFingerCount = 0;
static int			s_mousePresses = 0;
static float		s_mousePressTime = 0;
static bool         s_mouseMoveOnly = false;
static float        s_mouseDragStartX = 0;
static float        s_mouseDragStartY = 0;

static float		_prevHWMouseX = MAXFLOAT;
static float		_prevHWMouseY = MAXFLOAT;
static float		_curHWMouseX = MAXFLOAT;
static float		_curHWMouseY = MAXFLOAT;
static int			_curHWMouseBtns = 0;

static float        _curDPADMouseAccel = 0.1f;

static VirtJoy*		_virtJoy = 0;
static bool         _virtJoyEnabled = false;
static bool         _virtJoyFloating = false;
static float        _virtJoyDeadZone = 0;
static float        _virtJoyDiagSensitivity = 0.5f;


static void VirtKB_Create();
static void VirtKB_CreateTextures();
static void VirtKB_DestroyTextures();
static void VirtKB_SetupShader();
static void VirtKB_DestroyShader();
static void VirtKB_ClearQuickKeys();
static void VirtKB_CreateQuickKeys();
static void VirtKB_InitCallbacks();

static void VirtKB_UpdateQuickKeyVerts();

static void VirtKB_UpdateVkbVerts();
static void VirtKB_GetVkbScreenOffset(int *x, int *y);
static void VirtKB_ZoomVKB(float absChange);
static void VirtKB_PanVKB(float absX, float absY);
static void VirtKB_resetVkbPresses();
static void VirtKB_addVkbPress(int vkbKeyID, bool focusKey);

static void VirtKB_NavLeft();
static void VirtKB_NavRight();
static void VirtKB_NavUp();
static void VirtKB_NavDown();

static const VirtKeyDef *VirtKB_VkbHitTest(float x, float y);

static void VirtKB_updateInput();
static void VirtKB_updateTouchMouse();
static void VirtKB_updateHardwareMouse();
static void VirtKB_updateVirtMouseEmu();

static void VirtKB_clearMousePresses();

static void VirtKB_onRender(JNIEnv *env);
static void VirtKB_RenderVerts(RTShader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numQuads);
static void VirtKB_UpdateRectVerts(	GLfloat *v, float x1, float y1, float x2, float y2,
								float u1, float v1, float u2, float v2,
								float r, float g, float b, float a);
static void VirtKB_UpdatePolyVerts(GLfloat *v, float *x, float *y, float *tu, float *tv, float r, float g, float b, float a);

static void VirtKB_ToggleTurboSpeed(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ToggleKeyboard(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtkKB_ScreenZoomToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtkKB_VkbZoomToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtkKB_ScreenPresetToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtkKB_KeyboardPresetToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static bool VirtkKB_KeyboardSetPreset(int preset);
static void VirtKB_ToggleShowUI(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_TogglePause(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_MJToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_MouseLB(bool down);
static void VirtKB_MouseRB(bool down);
static void VirtKB_ToggleAutoFire(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_QuickSaveState(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_QuickLoadState(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ShowSoftMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ShowSoftFloppyMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ShowFloppyAInsert(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ShowFloppyBInsert(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_ShowSettingsMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);

static void VirtKB_updateAutoHide(bool touched);

const SpriteDef* VirtKB_findSpriteDef(const char *name)
{
    for (int i = 0; i < g_vkbSpriteDefsCount; ++i)
    {
        if (strcmp(g_vkbSpriteDefs[i].name, name) == 0)
        {
            return &g_vkbSpriteDefs[i];
        }
    }
    return 0;
}

void VirtKB_RefreshKB()
{
	s_recreateQuickKeys = true;
}

int VirtKB_GetJoystickPort()
{
	return s_joyID;
}


void _clearJoystickInput()
{
    Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_LEFT);
    Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_RIGHT);
    Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_UP);
    Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_DOWN);
    Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_FIRE);
}

void VirtKB_SetJoystickPort(int port)
{
	if (port < 0 || port >= JOYSTICK_COUNT)
	{
		return;
	}

	// clear cur joystick
    _clearJoystickInput();

	s_joyID = port;
}

void VirtKB_MapJoysticksToArrowKeys(bool map)
{
	s_joyMapToArrowKeys = map;
	s_recreateQuickKeys = true;
}

void VirtKB_SetMouseEmuDirect()
{
	if (!s_isPortrait && (s_showKeyboard || s_screenZoomMode))  { s_prevInputFlags &= ~(FLAG_MOUSEBUTTON); }
	else									                    { s_curInputFlags &= ~(FLAG_MOUSEBUTTON); }

	s_recreateQuickKeys = true;

	VirtKB_clearMousePresses();
}

void VirtKB_SetMouseEmuButtons()
{
	if (!s_isPortrait && (s_showKeyboard || s_screenZoomMode))  { s_prevInputFlags |= (FLAG_MOUSEBUTTON); }
	else									                    { s_curInputFlags |= (FLAG_MOUSEBUTTON); }

	s_recreateQuickKeys = true;

	VirtKB_clearMousePresses();
}

void VirtKB_SetMouseEmuSpeed(float speed)
{
	s_mouseSpeed = speed;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_updateInput(JNIEnv * env, jobject obj,
		jboolean t0, jfloat tx0, jfloat ty0,
		jboolean t1, jfloat  tx1, jfloat ty1,
		jboolean t2, jfloat  tx2, jfloat ty2,
		jfloat mouseX, jfloat mouseY, jint mouseBtns,
		jintArray keyPresses, jfloatArray curAxis)
{
	if (!(s_InputReady&s_InputEnabled)) return;

	s_curEnv = env;

	{
		int numVals = (env)->GetArrayLength(keyPresses);
		jint* keyPressVals = (env)->GetIntArrayElements(keyPresses, 0);

		for (int i = 0; i < numVals; ++i)
		{
			int v = keyPressVals[i];
			s_jKeyPresses->_flags[i] = *((uint32_t*)(&v));
		}

		(env)->ReleaseIntArrayElements(keyPresses, keyPressVals, JNI_ABORT);
		//(env)->DeleteLocalRef(keyPresses); // explicitly releasing to assist garbage collection, though not required
	}

	{
		int numVals = (env)->GetArrayLength(curAxis);
		jfloat* axisVals = (env)->GetFloatArrayElements(curAxis, 0);

		for (int i = 0; i < numVals; ++i)
		{
			s_jAxis[i] = axisVals[i];
		}

		(env)->ReleaseFloatArrayElements(curAxis, axisVals, JNI_ABORT);
		//(env)->DeleteLocalRef(curAxis); // explicitly releasing to assist garbage collection, though not required
	}

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];

	curtouched[0] = t0;	curtouchX[0] = tx0;	curtouchY[0] = ty0;
	curtouched[1] = t1;	curtouchX[1] = tx1;	curtouchY[1] = ty1;
	curtouched[2] = t2;	curtouchX[2] = tx2;	curtouchY[2] = ty2;

	if (_prevHWMouseX == MAXFLOAT)
	{
		_prevHWMouseX = mouseX;
		_prevHWMouseY = mouseY;
	}
	_curHWMouseX = mouseX;
	_curHWMouseY = mouseY;
	_curHWMouseBtns = mouseBtns;

	bool hasTouches = t0 || t1 || t2;
    VirtKB_updateAutoHide(hasTouches);

    if (!hasTouches) {
        s_curInputLayer = InputLayer_None;
    }

	VirtKB_updateInput();

	if (s_recreateQuickKeys)
	{
		VirtKB_CreateQuickKeys();
		s_recreateQuickKeys = false;
	}

	s_curIndex = 1 - s_curIndex;

	s_curEnv = 0;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleMouseActive(JNIEnv * env, jobject obj)
{
	if (!(s_InputReady&s_InputEnabled)) return;

	VirtKB_SetMouseActive(!VirtKB_getMouseActive());
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetMouseActive(JNIEnv * env, jobject obj)
{
	return VirtKB_getMouseActive();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetScreenScalePreset(JNIEnv * env, jobject obj, jint preset)
{
	if (!(s_InputReady&s_InputEnabled)) return;

	s_curScreenZoomPreset = (preset) % ScreenZoom_NumOf;
	Renderer_setZoomPreset(s_curScreenZoomPreset);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleVirtKeyboard(JNIEnv * env, jobject obj, jboolean nonTouch)
{
	if (!(s_InputReady&s_InputEnabled)) return;

	s_nonTouchKB = nonTouch;
	VirtKB_ToggleKeyboard(0, 0, true);
}

void VirtKB_EnableInput(bool enable)
{
	s_InputEnabled = enable;
	s_recreateQuickKeys = true;
}

void VirtKB_OnSurfaceCreated()
{
	s_KbTextureID = 0;
}

int VirtKB_OnSurfaceChanged(int width, int height, int prevW, int prevH)
{
	int prevPortrait = s_isPortrait;

	VirtKB_CleanUp();
	VirtKB_Create();

	if (prevW == 0 || prevH == 0) // first init
	{
		if (s_isPortrait)
		{
			//if (!s_showKeyboard)
			//{
			//	VirtKB_ToggleKeyboard(0, 0, true); // show portrait by default in portrait
			//}
		}
	}
	else // re-inits
	{
		if (width != 0 && height != 0 && width != prevW && height != prevH)
		{
			if (s_curScreenZoomPreset >= 0)
			{
				Renderer_setZoomPreset(s_curScreenZoomPreset);
			}
			if (s_VkbZoomInited)
			{
				VirtkKB_KeyboardSetPreset(s_curKeyboardZoomPreset);
			}

			// reset and restore any current flags/modes
			{
				bool showKeyboard = s_showKeyboard;
				bool showMouse = (!prevPortrait && (s_showKeyboard || s_screenZoomMode)) ? (s_prevInputFlags & FLAG_MOUSE) != 0 : (s_curInputFlags & FLAG_MOUSE) != 0;
				bool mouseButtons = ((s_curInputFlags | s_prevInputFlags) & FLAG_MOUSEBUTTON) != 0;

		        s_curInputFlags = DEFAULT_INPUT_FLAGS;
		        s_prevInputFlags = s_curInputFlags;
				s_keyboardZoomMode = false;
				s_screenZoomMode = false;
				s_showKeyboard = false;

		        if (mouseButtons)
		        {
			        s_curInputFlags |= FLAG_MOUSEBUTTON;
			        s_prevInputFlags |= FLAG_MOUSEBUTTON;
		        }

		        if (showMouse)
		        {
		            VirtKB_MJToggle(0, 0, true);
		        }
		        if (showKeyboard)
		        {
					VirtKB_ToggleKeyboard(0, 0, true);
		        }
			}
		}

	}

	s_InputReady = true;

	return 0;
}

void VirtKB_CleanUp()
{
	if (_virtJoy != 0)
	{
		delete _virtJoy;
		_virtJoy = 0;
	}

	VirtKB_DestroyShader();
	VirtKB_ClearQuickKeys();
	VirtKB_DestroyTextures();

	for (int i = 0; i < 2; ++i)
	{
		delete s_curButtons[i];
		s_curButtons[i] = 0;
	}
	delete s_changedButtons;
	s_changedButtons = 0;

	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < VKB_MaxTouches; ++i)
		{
			s_curtouched[j][i] = false;
			s_curtouchX[j][i] = 0;
			s_curtouchY[j][i] = 0;
		}
	}

	// TODO: clear keys and joystick states in hatari

	s_InputReady = false;
}

void VirtKB_Create()
{
	int scrWidth = getScreenWidth();
    int scrHeight = getScreenHeight();
    s_isPortrait = scrWidth < scrHeight;

    {
        g_vkbTexKbW = 0;
        g_vkbTexKbH = 0;

        const SpriteDef *vkbSprite = VirtKB_findSpriteDef("vkbd");
        if (vkbSprite != 0)
        {
            g_vkbTexKbW = vkbSprite->w;
            g_vkbTexKbH = vkbSprite->h;
        }
    }

    VirtKB_InitCallbacks();
	s_jKeyPresses->clearAll();

	Joy_SetCustomEmu(1);

	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < VKB_MaxTouches; ++i)
		{
			s_curtouched[j][i] = false;
			s_curtouchX[j][i] = 0;
			s_curtouchY[j][i] = 0;
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		s_curButtons[i] = new BitFlags(VKB_KEY_NumOf);

		memset(s_buttonDown[i], 0, sizeof(int)*MaxButtonDown);
		s_numButtonDown[i] = 0;
	}
	s_changedButtons = new BitFlags(VKB_KEY_NumOf);

	VirtKB_CreateTextures();

    if (_virtJoy == 0)
    {
        _virtJoy = new VirtJoy();
        _virtJoy->create(s_KbTextureID, s_VkbGPUTexWidth, s_VkbGPUTexHeight, _virtJoyDeadZone, s_joystickSize, _virtJoyDiagSensitivity,
                         s_joystickAlpha, s_autoHideAlpha, _virtJoyFloating);
    }

	VirtKB_CreateQuickKeys();

	VirtKB_SetupShader();

	if (s_curScreenZoomPreset == -1)
	{
		s_curScreenZoomPreset = ScreenZoom_Fit;
		Renderer_setZoomPreset(s_curScreenZoomPreset);
	}

	if (!s_VkbZoomInited)
	{
		int preset = (s_isPortrait && !s_nonTouchKB) ? VkbZoom_2 : VkbZoom_Fit;
		if (VirtkKB_KeyboardSetPreset(preset))
		{
			s_VkbZoomInited = true;
		}
	}

	VirtKB_UpdateVkbVerts();
}

void VirtKB_DestroyTextures()
{
	if (s_KbTextureID != 0)
	{
	    glDeleteTextures(1, &s_KbTextureID);
	    s_KbTextureID = 0;
	    s_VkbGPUTexWidth = 0;
	    s_VkbGPUTexHeight = 0;
	}
}

void VirtKB_CreateTextures()
{
	int kbTexSize = 0;
	unsigned char *srcTex = uncompressGZ(g_vkbTexZ, g_vkbTexZSize, &kbTexSize);
	if (srcTex)
	{
		int fullWidth = roundUpPower2(g_vkbTexFullW);
		int fullHeight = roundUpPower2(g_vkbTexFullH);

		if (fullWidth < fullHeight) fullWidth = fullHeight;
		if (fullHeight < fullWidth) fullHeight = fullWidth;

		unsigned char *kbFullTex = (unsigned char *)memalign(128, fullWidth*fullHeight*4);
		if (kbFullTex == NULL)
		{
			free(srcTex);
			return;
		}
		memset(kbFullTex, 0, fullWidth*fullHeight*4);
		{
			//int srcStride8 = g_vkbTexFullW;
			int srcStride8 = g_vkbTexFullW*4;
			int dstStride32 = fullWidth;

			for (int y = 0; y < g_vkbTexFullH; ++y)
			{
				unsigned char *src = srcTex + (y*srcStride8);
				unsigned int *dst = ((unsigned int*)kbFullTex) + (y*dstStride32);
				for (int x = 0; x < g_vkbTexFullW; ++x)
				{
					unsigned char r = *src++;
					unsigned char g = *src++;
					unsigned char b = *src++;
					unsigned char a = *src++;
					*dst = a << 24 | b << 16 | g << 8 | r;

					dst++;
				}
			}
		}

		glGenTextures(1, &s_KbTextureID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, s_KbTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fullWidth, fullHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, kbFullTex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		free(kbFullTex);
		free(srcTex);

		s_VkbGPUTexWidth = fullWidth;
	    s_VkbGPUTexHeight = fullHeight;
	}
}

static void VirtKB_ClearQuickKeys()
{
	s_numQuickKeys = 0;
	s_waitInputCleared = true;
}

static bool addQuickKey(int x1, int y1, int x2, int y2,
						float rx1, float ry1, float rx2, float ry2,
						float txo1, float tyo1, float txo2, float tyo2,
						int panel,
						const VirtKeyDef *vkd)
{
	if (s_numQuickKeys >= s_MaxNumQuickKeys)
	{
		return false;
	}

	if (!(s_curInputFlags & vkd->flags))
	{
		return false;
	}

	if ((s_curInputFlags & FLAG_JOY) != 0)
	{
		if ((vkd->flags & FLAG_MOUSEBUTTON) != 0)
		{
			return false;
		}
	}

	float tw = (float)s_VkbGPUTexWidth;
	float th = (float)s_VkbGPUTexHeight;

	QuickKey *qk = &s_QuickKeys[s_numQuickKeys];

	qk->pKeyDef = vkd;

	qk->panel = panel;

	qk->x1 = x1; qk->y1 = y1;
	qk->x2 = x2; qk->y2 = y2;

	qk->rx1 = rx1; qk->ry1 = ry1;
	qk->rx2 = rx2; qk->ry2 = ry2;

	qk->tx1 = (float)(vkd->qv[0]/*+txo1*/)/tw;
	qk->ty1 = (float)(vkd->qv[1]/*+tyo1*/)/th;
	qk->tx2 = (float)(vkd->qv[2]/*+txo2*/)/tw;
	qk->ty2 = (float)(vkd->qv[3]/*+tyo2*/)/th;

	++s_numQuickKeys;
	return true;
}

static void _onSTKeyEventCallback(const VirtKeyDef *vk, uint32_t uParam1, bool down)
{
	IKBD_PressSTKey(vk->scancode, down);
}

static void _onMouseButtonEventCallback(const VirtKeyDef *vk, uint32_t uParam1, bool down)
{
	if (vk->id == VKB_KEY_MOUSELB)
	{
		VirtKB_MouseLB(down);
	}
	else
	{
		VirtKB_MouseRB(down);
	}
}

static void _onJoystickEventCallback(const VirtKeyDef *vk, uint32_t uParam1, bool down)
{
	if ((s_showKeyboard && s_nonTouchKB) || VirtKB_getMouseActive())
	{
		return;
	}

	int joybit = GET_LOWORD(uParam1);
	int clearbit = GET_HIWORD(uParam1);

	if (down)
	{
		Joy_CustomDown(s_joyID, joybit, clearbit);
	}
	else
	{
		Joy_CustomUp(s_joyID, joybit);
	}
}

static void _onJoystick2EventCallback(const VirtKeyDef *vk, uint32_t uParam1, bool down)
{
	if ((s_showKeyboard && s_nonTouchKB) || VirtKB_getMouseActive())
	{
		return;
	}

	int joybit = GET_LOWORD(uParam1);
	int clearbit = GET_HIWORD(uParam1);

	const int joy2id = 0;

	if (down)
	{
		Joy_CustomDown(joy2id, joybit, clearbit);
	}
	else
	{
		Joy_CustomUp(joy2id, joybit);
	}
}

static void VirtKB_InitCallbacks()
{
	if (s_keyCallbacks != 0) { return; }

	s_keyCallbacks = new KeyCallback [g_vkbKeyDefsSize];
	memset(s_keyCallbacks, 0, sizeof(KeyCallback) * g_vkbKeyDefsSize);

	for (int i = 0; i < g_vkbKeyDefsSize; ++i)
	{
		const VirtKeyDef* vk = &g_vkbKeyDefs[i];
		KeyCallback* kcb = &s_keyCallbacks[i];

		if (vk->flags & (FLAG_CUSTOMKEY))
		{
			switch (vk->id)
			{
				case VKB_KEY_KEYBOARDTOGGLE:		kcb->onKeyEvent = VirtKB_ToggleKeyboard; break;
				case VKB_KEY_KEYBOARDTOGGLE_SEL:	kcb->onKeyEvent = VirtKB_ToggleKeyboard; break;
				case VKB_KEY_SCREENZOOM:			kcb->onKeyEvent = VirtkKB_ScreenZoomToggle; break;
				case VKB_KEY_SCREENZOOM_SEL:		kcb->onKeyEvent = VirtkKB_ScreenZoomToggle; break;
				case VKB_KEY_KEYBOARDZOOM:			kcb->onKeyEvent = VirtkKB_VkbZoomToggle; break;
				case VKB_KEY_KEYBOARDZOOM_SEL:		kcb->onKeyEvent = VirtkKB_VkbZoomToggle; break;
				case VKB_KEY_SCREENPRESETS:			kcb->onKeyEvent = VirtkKB_ScreenPresetToggle; break;
				case VKB_KEY_KEYBOARDPRESETS:		kcb->onKeyEvent = VirtkKB_KeyboardPresetToggle; break;
				case VKB_KEY_MOUSETOGGLE:			kcb->onKeyEvent = VirtKB_MJToggle; break;
				case VKB_KEY_JOYTOGGLE:				kcb->onKeyEvent = VirtKB_MJToggle; break;

				case VKB_KEY_TURBOSPEED:			kcb->onKeyEvent = VirtKB_ToggleTurboSpeed; break;
				case VKB_KEY_NORMALSPEED:			kcb->onKeyEvent = VirtKB_ToggleTurboSpeed; break;

				case VKB_KEY_TOGGLEUI:				kcb->onKeyEvent = VirtKB_ToggleShowUI; break;
				case VKB_KEY_PAUSE:					kcb->onKeyEvent = VirtKB_TogglePause; break;

				case VKB_KEY_AUTOFIRE:				kcb->onKeyEvent = VirtKB_ToggleAutoFire; break;

				case VKB_KEY_QUICKSAVESTATE:		kcb->onKeyEvent = VirtKB_QuickSaveState; break;
				case VKB_KEY_QUICKLOADSTATE:		kcb->onKeyEvent = VirtKB_QuickLoadState; break;

				case VKB_KEY_ANDROID_MENU:			kcb->onKeyEvent = VirtKB_ShowSoftMenu; break;
				case VKB_KEY_FLOPPY_MENU:			kcb->onKeyEvent = VirtKB_ShowSoftFloppyMenu; break;
				case VKB_KEY_FLOPPYA_INSERT:		kcb->onKeyEvent = VirtKB_ShowFloppyAInsert; break;
				case VKB_KEY_FLOPPYB_INSERT:		kcb->onKeyEvent = VirtKB_ShowFloppyBInsert; break;
				case VKB_KEY_SETTINGS_MENU:			kcb->onKeyEvent = VirtKB_ShowSettingsMenu; break;
			}
		}
		else if (vk->flags & (FLAG_STKEY|FLAG_STFNKEY))
		{
			kcb->onKeyEvent = _onSTKeyEventCallback;
		}
		else if (vk->flags & (FLAG_JOY))
		{
			switch (vk->id)
			{
				case VKB_KEY_JOYLEFT:	kcb->onKeyEvent = _onJoystickEventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_LEFT, ATARIJOY_BITMASK_RIGHT); break;
				case VKB_KEY_JOYRIGHT:	kcb->onKeyEvent = _onJoystickEventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_RIGHT, ATARIJOY_BITMASK_LEFT); break;
				case VKB_KEY_JOYUP:		kcb->onKeyEvent = _onJoystickEventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_UP, ATARIJOY_BITMASK_DOWN); break;
				case VKB_KEY_JOYDOWN:	kcb->onKeyEvent = _onJoystickEventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_DOWN, ATARIJOY_BITMASK_UP); break;
				case VKB_KEY_JOYFIRE:	kcb->onKeyEvent = _onJoystickEventCallback; kcb->uParam1 = ATARIJOY_BITMASK_FIRE; break;

				case VKB_KEY_JOY2LEFT:	kcb->onKeyEvent = _onJoystick2EventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_LEFT, ATARIJOY_BITMASK_RIGHT); break;
				case VKB_KEY_JOY2RIGHT:	kcb->onKeyEvent = _onJoystick2EventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_RIGHT, ATARIJOY_BITMASK_LEFT); break;
				case VKB_KEY_JOY2UP:	kcb->onKeyEvent = _onJoystick2EventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_UP, ATARIJOY_BITMASK_DOWN); break;
				case VKB_KEY_JOY2DOWN:	kcb->onKeyEvent = _onJoystick2EventCallback; kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_DOWN, ATARIJOY_BITMASK_UP); break;
				case VKB_KEY_JOY2FIRE:	kcb->onKeyEvent = _onJoystick2EventCallback; kcb->uParam1 = ATARIJOY_BITMASK_FIRE; break;
			}
		}
		else if (vk->flags & (FLAG_MOUSEBUTTON))
		{
			kcb->onKeyEvent = _onMouseButtonEventCallback;
		}
	}
}

// TODO: call on exit (not called currently)
void VirtKB_DeinitCallbacks()
{
	if (s_keyCallbacks != 0)
	{
		delete [] s_keyCallbacks;
		s_keyCallbacks = 0;
	}
}

// TODO: customizable layout
void VirtKB_CreateQuickKeys()
{
	VirtKB_ClearQuickKeys();

	if (s_hideAll)
	{
		//return;
	}

	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	int joyAreaMinY = 0;
	int joyAreaMaxX = scrwidth;

	float sscale = (float)scrwidth/1024.0f;

	if (s_isPortrait) {
		 // portrait, make buttons a bit bigger
		 sscale *= 1.4f;
	}

	// top left keys
	if (!s_showJoystickOnly && !s_hideAll && !s_nonTouchKB)
	{
		int keyOffsetX = (int)ceilf(10*sscale);
		int keyOffsetY = (int)ceilf(10*sscale);
		int keyBtnSizeX = (int)ceilf(60*sscale*s_keySizeVX);
		int keyBtnSizeY = (int)ceilf(60*sscale*s_keySizeVY);
		int keyMarginY = (int)ceilf(2*sscale);

		//bool isFullScreen = Renderer_isFullScreenStretch();
		int vkbKeys[] = {	VKB_KEY_KEYBOARDTOGGLE, VKB_KEY_KEYBOARDZOOM, VKB_KEY_KEYBOARDPRESETS,
							VKB_KEY_SCREENZOOM, VKB_KEY_SCREENPRESETS, VKB_KEY_MOUSETOGGLE, VKB_KEY_JOYTOGGLE};
		int numKeys = sizeof(vkbKeys)/sizeof(int);

		int curKeyY = keyOffsetY;
		for (int i = 0; i < numKeys; ++i)
		{
			int k = vkbKeys[i];

			//if (isFullScreen && k == VKB_KEY_SCREENZOOM) { continue; }
			if (s_isPortrait) {
				if (!s_keyboardZoomMode && k == VKB_KEY_KEYBOARDPRESETS) {
					continue;
				}
				if (s_keyboardZoomMode && k == VKB_KEY_KEYBOARDTOGGLE) {
					continue;
				}
			}

			if (s_showKeyboard && k == VKB_KEY_KEYBOARDTOGGLE)  	{ k = VKB_KEY_KEYBOARDTOGGLE_SEL; }
			if (s_keyboardZoomMode && k == VKB_KEY_KEYBOARDZOOM)	{ k = VKB_KEY_KEYBOARDZOOM_SEL; }
			if (s_screenZoomMode && k == VKB_KEY_SCREENZOOM) 	    { k = VKB_KEY_SCREENZOOM_SEL; }

			if (!addQuickKey(0, curKeyY, keyOffsetX+keyBtnSizeX, curKeyY+keyBtnSizeY,
						keyOffsetX, curKeyY, keyOffsetX+keyBtnSizeX, curKeyY+keyBtnSizeY,
						2, 2, -2, -2, QuickKey::PANEL_TL, &g_vkbKeyDefs[k])) { continue; }

			curKeyY = s_QuickKeys[s_numQuickKeys-1].y2 + keyMarginY;
		}

		joyAreaMinY = curKeyY;
	}

	// top right keys
	//if (!s_showJoystickOnly && !(s_hideShortcutKeys && s_hideTurboKey))
	//if (!s_showJoystickOnly && !(s_hideShortcutKeys))
	{
		int keyOffsetX = (int)ceilf(10*sscale);
		int keyOffsetY = (int)ceilf(10*sscale);
		int keyBtnSizeX = (int)ceilf(60*sscale*s_keySizeVX);
		int keyBtnSizeY = (int)ceilf(60*sscale*s_keySizeVY);
		int keyMarginY = (int)ceilf(2*sscale);

		const int kMaxTRKeys = 16; // TODO: FIXME
		int vkbKeys[kMaxTRKeys];
		int numKeys = 0;

		//if (!s_hideTurboKey)
		//{
		//	vkbKeys[numKeys++] = VKB_KEY_NORMALSPEED;
		//}

        if (!s_showJoystickOnly && !(s_hideShortcutKeys) && !s_hideAll)
        {
            if (s_vkbObsessionKeys)
            {
                vkbKeys[numKeys++] = VKB_KEY_NORMALSPEED;
                vkbKeys[numKeys++] = VKB_KEY_F1; vkbKeys[numKeys++] = VKB_KEY_F2; vkbKeys[numKeys++] = VKB_KEY_F3; vkbKeys[numKeys++] = VKB_KEY_ANDROID_MENU;
            }
            else
            {
                if (!s_hideShortcutKeys)
                {
                    const int *shortcutKeys = s_shortcutMap->getCurAnchorList(ShortcutMap::kAnchorTR);
                    for (int k = 0; k < ShortcutMap::kMaxKeys[ShortcutMap::kAnchorTR]; ++k)
                    {
                        int vkId = shortcutKeys[k];
                        if (vkId >= 0)
                        {
                            vkbKeys[numKeys++] = vkId;
                        }
                    }

                    //vkbKeys[numKeys++] = VKB_KEY_Y; vkbKeys[numKeys++] = VKB_KEY_N; vkbKeys[numKeys++] = VKB_KEY_1; vkbKeys[numKeys++] = VKB_KEY_2;
                    //if (s_vkbExtraKeys)
                    //{
                    //	vkbKeys[numKeys++] = VKB_KEY_RETURN;
                    //}
                }
            }
        }
        else
        {
            // always show menu key
            vkbKeys[numKeys++] = VKB_KEY_ANDROID_MENU;
        }

		int curKeyY = keyOffsetY;
		for (int i = 0; i < numKeys; ++i)
		{
			if (vkbKeys[i] == VKB_KEY_NORMALSPEED) // speed button hack
			{
				bool turbo = getTurboSpeed()!=0;
				vkbKeys[i] = turbo ? VKB_KEY_TURBOSPEED : VKB_KEY_NORMALSPEED;
			}

			if (!addQuickKey(scrwidth - keyOffsetX-keyBtnSizeX, curKeyY, scrwidth, curKeyY+keyBtnSizeY,
						scrwidth-keyOffsetX-keyBtnSizeX, curKeyY, scrwidth-keyOffsetX, curKeyY+keyBtnSizeY,
						2, 2, -2, -2, QuickKey::PANEL_TR, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyY = s_QuickKeys[s_numQuickKeys-1].y2 + keyMarginY;
		}
	}

	const int kFireDefaultSize = 100;

	// mouse keys (bottom left)
	s_mouseButtonIgnoreQuickKeyIdx[0] = -1;
	s_mouseButtonIgnoreQuickKeyIdx[1] = -1;
	{
		int keyOffsetX = (int)ceilf(30*sscale);
		int keyOffsetY = (int)ceilf(30*sscale);
		int keyBtnSizeX = (int)ceilf(60*sscale*s_keySizeHX);
		int keyBtnSizeY = (int)ceilf(60*sscale*s_keySizeHY);
		int keyMarginX = (int)ceilf(64*sscale);
		int fireBtnSize = (int)ceilf(kFireDefaultSize*sscale*s_joystickFireSize);

		int vkbKeys[] = {VKB_KEY_MOUSELB, VKB_KEY_MOUSERB};
		int x1Overlap[] = {0, (int)(keyMarginX*0.8f)};
		int x2Overlap[] = {(int)(keyMarginX*0.8f), 0};
		int numKeys = sizeof(vkbKeys)/sizeof(int);

		int curKeyX = keyOffsetX;

		for (int i = 0; i < numKeys; ++i)
		{
			bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
			int btnSizeX = isFire ? fireBtnSize : keyBtnSizeX;	// the fire button is a special case (bigger hit area)
			int btnSizeY = isFire ? fireBtnSize : keyBtnSizeY;	// the fire button is a special case (bigger hit area)
			if (!addQuickKey(curKeyX-x1Overlap[i], scrheight-keyOffsetY-btnSizeY, curKeyX+btnSizeX+x2Overlap[i], scrheight,
						curKeyX, scrheight-keyOffsetY-btnSizeY, curKeyX+btnSizeX, scrheight-keyOffsetY,
						2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyX = s_QuickKeys[s_numQuickKeys-1].x2 - x2Overlap[i] + keyMarginX;

			if (s_mouseButtonIgnoreQuickKeyIdx[0] == -1) { s_mouseButtonIgnoreQuickKeyIdx[0] = s_numQuickKeys-1; } // HACK
			else if (s_mouseButtonIgnoreQuickKeyIdx[1] == -1) { s_mouseButtonIgnoreQuickKeyIdx[1] = s_numQuickKeys-1; } // HACK
		}
	}

	// bottom right keys
	bool joyMode = (s_curInputFlags & FLAG_JOY) != 0;
	int obsessionButtonSize = 120;
	if (!s_hideAll)
	{
		int keyOffsetX = (int)ceilf(30*sscale);
		int keyOffsetY = (int)ceilf(30*sscale);
		int keyBtnSizeX = (int)ceilf(60*sscale*s_keySizeHX);
		int keyBtnSizeY = (int)ceilf(60*sscale*s_keySizeHY);
		int keyMarginX = (int)ceilf(2*sscale);
		int fireBtnSize = (int)ceilf((s_vkbObsessionKeys?obsessionButtonSize:kFireDefaultSize)*sscale*s_joystickFireSize);

		const int kMaxBRKeys = 16; // TODO: FIXME
		int vkbKeysNormal[kMaxBRKeys];
		int numNormalKeys = 0;
		const int *shortcutKeys = s_shortcutMap->getCurAnchorList(ShortcutMap::kAnchorBR);
		if (!s_hideJoystick)
		{
			vkbKeysNormal[numNormalKeys++] = VKB_KEY_JOYFIRE;
		}
		for (int k = 0; k < ShortcutMap::kMaxKeys[ShortcutMap::kAnchorBR]; ++k)
		{
			int vkId = shortcutKeys[k];
			if (vkId >= 0)
			{
				vkbKeysNormal[numNormalKeys++] = vkId;
			}
		}

		//int vkbKeysNormal[] = {VKB_KEY_JOYFIRE, VKB_KEY_SPACE, VKB_KEY_LEFTSHIFT, VKB_KEY_ALTERNATE};
		//int vkbKeysExtra[] = {VKB_KEY_JOYFIRE, VKB_KEY_SPACE, VKB_KEY_LEFTSHIFT, VKB_KEY_ALTERNATE, VKB_KEY_CONTROL};
		int vkbKeysHidden[] = {VKB_KEY_JOYFIRE};
		int vkbKeysObsession[] = {VKB_KEY_RIGHTSHIFT_BUTTON, VKB_KEY_DOWNARROW};
		//int numNormalKeys = sizeof(vkbKeysNormal)/sizeof(int);
		//int numExtraKeys = sizeof(vkbKeysExtra)/sizeof(int);
		int numHiddenKeys = sizeof(vkbKeysHidden)/sizeof(int);
		int numObsessionKeys = sizeof(vkbKeysObsession)/sizeof(int);

		//int* vkbKeys = (joyMode&&s_vkbObsessionKeys) ? vkbKeysObsession : (s_hideExtraJoyKeys ? vkbKeysHidden : (s_vkbExtraKeys ? vkbKeysExtra : vkbKeysNormal));
		//int numKeys = (joyMode&&s_vkbObsessionKeys) ? numObsessionKeys : (s_hideExtraJoyKeys ? numHiddenKeys : (s_vkbExtraKeys ? numExtraKeys : numNormalKeys));
		int* vkbKeys = (joyMode&&s_vkbObsessionKeys) ? vkbKeysObsession : (s_hideExtraJoyKeys ? vkbKeysHidden : vkbKeysNormal);
		int numKeys = (joyMode&&s_vkbObsessionKeys) ? numObsessionKeys : (s_hideExtraJoyKeys ? numHiddenKeys : numNormalKeys);

		if (!s_vkbObsessionKeys && joyMode && s_hideJoystick && s_hideExtraJoyKeys)
		{
			numKeys = 0;
		}

		int curKeyX = scrwidth - keyOffsetX;

		for (int i = 0; i < numKeys; ++i)
		{
			if (vkbKeys[i] == VKB_KEY_NORMALSPEED) // speed button hack
			{
				bool turbo = getTurboSpeed()!=0;
				vkbKeys[i] = turbo ? VKB_KEY_TURBOSPEED : VKB_KEY_NORMALSPEED;
			}

			bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
			bool isBigButton = isFire || (vkbKeys[i] == VKB_KEY_LEFTSHIFT_BUTTON) || (vkbKeys[i] == VKB_KEY_RIGHTSHIFT_BUTTON);
			int btnSizeX = isBigButton ? fireBtnSize : keyBtnSizeX;	// the fire button is a special case (bigger hit area)
			int btnSizeY = isBigButton ? fireBtnSize : keyBtnSizeY;	// the fire button is a special case (bigger hit area)
			if (!addQuickKey(curKeyX-btnSizeX, scrheight-keyOffsetY-btnSizeY, curKeyX, scrheight,
						curKeyX-btnSizeX, scrheight-keyOffsetY-btnSizeY, curKeyX, scrheight-keyOffsetY,
						2, 2, -2, -2, QuickKey::PANEL_BR, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyX = s_QuickKeys[s_numQuickKeys-1].x1 - keyMarginX - ((i==0 && !s_hideJoystick)? keyOffsetX : 0);
		}

		joyAreaMaxX = curKeyX;
	}

    bool enableVirtJoystick = false;
	if (joyMode)
	{
		// bottom left keys
		if (s_vkbObsessionKeys && !s_hideAll)
		{
			int keyOffsetX = (int)ceilf(30*sscale);
			int keyOffsetY = (int)ceilf(30*sscale);
			int keyBtnSizeX = (int)ceilf(60*sscale*s_keySizeHX);
			int keyBtnSizeY = (int)ceilf(60*sscale*s_keySizeHY);
			int keyMarginX = (int)ceilf(2*sscale);
			int fireBtnSize = (int)ceilf(obsessionButtonSize*sscale*s_joystickFireSize);

			int vkbKeys[] = {VKB_KEY_LEFTSHIFT_BUTTON, VKB_KEY_SPACE};
			int numKeys = sizeof(vkbKeys)/sizeof(int);

			int curKeyX = keyOffsetX;

			for (int i = 0; i < numKeys; ++i)
			{
				bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
				bool isBigButton = isFire || (vkbKeys[i] == VKB_KEY_LEFTSHIFT_BUTTON) || (vkbKeys[i] == VKB_KEY_RIGHTSHIFT_BUTTON);
				int btnSizeX = isBigButton ? fireBtnSize : keyBtnSizeX;	// the fire button is a special case (bigger hit area)
				int btnSizeY = isBigButton ? fireBtnSize : keyBtnSizeY;	// the fire button is a special case (bigger hit area)
				if (!addQuickKey(curKeyX, scrheight-keyOffsetY-btnSizeY, curKeyX+btnSizeX, scrheight,
							curKeyX, scrheight-keyOffsetY-btnSizeY, curKeyX+btnSizeX, scrheight-keyOffsetY,
							2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

				curKeyX = s_QuickKeys[s_numQuickKeys-1].x2 + keyMarginX + ((i==0)? keyOffsetX : 0);
			}

			joyAreaMaxX = curKeyX;
		}
		else if (!s_hideJoystick && !s_hideAll)
		{
            enableVirtJoystick = _virtJoyEnabled;

			// joystick dir - from bottom left
			if (!_virtJoyEnabled)
			{
				int keyOffsetX = (int)ceilf(30*sscale);
				int keyOffsetY = (int)ceilf(30*sscale);
				int keyBtnSize = (int)ceilf(s_joystickSize*60.0f*sscale);
				int keyMarginY = (int)ceilf(2*sscale);

				int joyAreaMinWidth = keyOffsetX + (keyBtnSize*3);
				int joyAreaMinHeight = keyOffsetY + (keyBtnSize*3);

				if ((scrheight - joyAreaMinY) < joyAreaMinHeight)
				{
					joyAreaMinY = scrheight - joyAreaMinHeight;
				}
				if (joyAreaMaxX < joyAreaMinWidth)
				{
					joyAreaMaxX = joyAreaMinWidth;
				}

				// left
				if (!addQuickKey(0, joyAreaMinY, keyOffsetX+keyBtnSize, scrheight,
							keyOffsetX, scrheight-keyOffsetY-keyBtnSize*2, keyOffsetX+keyBtnSize, scrheight-keyOffsetY-keyBtnSize,
							2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_LEFTARROW:VKB_KEY_JOYLEFT])) { }

				// right
				if (!addQuickKey(keyOffsetX+keyBtnSize*2, joyAreaMinY, joyAreaMaxX, scrheight,
							keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY-keyBtnSize*2, keyOffsetX+keyBtnSize*3, scrheight-keyOffsetY-keyBtnSize,
							2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_RIGHTARROW:VKB_KEY_JOYRIGHT])) { }

				// up
				if (!addQuickKey(0, joyAreaMinY, joyAreaMaxX, scrheight-keyOffsetY-keyBtnSize*2,
							keyOffsetX+keyBtnSize, scrheight-keyOffsetY-keyBtnSize*3, keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY-keyBtnSize*2,
							2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_UPARROW:VKB_KEY_JOYUP])) { }

				// down
				if (!addQuickKey(0, scrheight-keyOffsetY-keyBtnSize, joyAreaMaxX, scrheight,
							keyOffsetX+keyBtnSize, scrheight-keyOffsetY-keyBtnSize, keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY,
							2, 2, -2, -2, QuickKey::PANEL_BL, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_DOWNARROW:VKB_KEY_JOYDOWN])) { }
			}
		}
	}

	if (_virtJoy != 0)
    {
        _virtJoy->setEnabled(enableVirtJoystick);
        _virtJoy->setJoyArea(0, joyAreaMinY, joyAreaMaxX, scrheight);
    }

	VirtKB_UpdateQuickKeyVerts();
}

static void VirtKB_DestroyShader()
{
	//Renderer_removeRenderCallback(VirtKB_onRender);
}

static void VirtKB_SetupShader()
{
	Renderer_addRenderCallback(VirtKB_onRender);

	int indexes[6] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < s_MaxNumQuickKeys; ++i)
	{
		GLushort *ind = &s_QuickKeyIndices[i*6];
		GLushort offset = i << 2;
		for (int j = 0; j < 6; ++j)
		{
			ind[j] = indexes[j] + offset;
		}
	}

	for (int i = 0; i < s_MaxVkbPresses; ++i)
	{
		GLushort *ind = &s_VkbPressedIndices[i*6];
		GLushort offset = i << 2;
		for (int j = 0; j < 6; ++j)
		{
			ind[j] = indexes[j] + offset;
		}
	}
}

static void VirtKB_RenderVerts(RTShader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numQuads)
{
	if (pShader == 0 || !pShader->_ready)
	{
		return;
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glUseProgram(pShader->_shaderProgram);

	if (pShader->_paramHandles[RTShader::ShaderParam_Pos] >= 0)
	{
		glVertexAttribPointer(pShader->_paramHandles[RTShader::ShaderParam_Pos], 3, GL_FLOAT, GL_FALSE, vstride, v);
		glEnableVertexAttribArray(pShader->_paramHandles[RTShader::ShaderParam_Pos]);
	}

	if (pShader->_paramHandles[RTShader::ShaderParam_Color] >= 0)
	{
		glVertexAttribPointer(pShader->_paramHandles[RTShader::ShaderParam_Color], 4, GL_FLOAT, GL_FALSE, vstride, v+3);
		glEnableVertexAttribArray(pShader->_paramHandles[RTShader::ShaderParam_Color]);
	}
	if (pShader->_paramHandles[RTShader::ShaderParam_TexCoord] >= 0)
	{
		glVertexAttribPointer(pShader->_paramHandles[RTShader::ShaderParam_TexCoord], 2, GL_FLOAT, GL_FALSE, vstride, v+7);
		glEnableVertexAttribArray(pShader->_paramHandles[RTShader::ShaderParam_TexCoord]);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	if (pShader->_paramHandles[RTShader::ShaderParam_Sampler] >= 0)
	{
		glUniform1i(pShader->_paramHandles[RTShader::ShaderParam_Sampler], 0);
	}

	glDrawElements(GL_TRIANGLES, 6*numQuads, GL_UNSIGNED_SHORT, ind);
}

static void VirtKB_onRender(JNIEnv *env)
{
	RTShader *pShader = Renderer_getColorModShader();

	// keyboard
	if (s_showKeyboard)
	{
		VirtKB_RenderVerts(pShader, s_VkbVerts, s_VkbStride, s_KbTextureID, s_VkbIndices, 1);

		// kb presses
		if (s_VkbCurNumPresses > 0)
		{
			GLuint pressedTexID = getWhiteTexture();
			VirtKB_RenderVerts(pShader, s_VkbPressedVerts, s_VkbPressedStride, pressedTexID, s_VkbPressedIndices, s_VkbCurNumPresses);
		}
	}

	// Quick Keys
	VirtKB_RenderVerts(pShader, s_QuickKeyVerts, s_QuickKeyStride, s_KbTextureID, s_QuickKeyIndices, s_numQuickKeys);

	if (_virtJoyEnabled && _virtJoy != 0)
	{
		_virtJoy->render();
	}
}

static void VirtKB_UpdateQuickKeyVerts()
{
	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	for (int i = 0; i < s_numQuickKeys; ++i)
	{
		struct QuickKey *qk = &s_QuickKeys[i];

		GLfloat *v = &s_QuickKeyVerts[i*36];

		float x1 = ((qk->rx1/(float)scrwidth)*2.0f) - 1.0f;
		float y1 = 1.0f - ((qk->ry1/(float)scrheight)*2.0f);
		float x2 = ((qk->rx2/(float)scrwidth)*2.0f) - 1.0f;
		float y2 = 1.0f - ((qk->ry2/(float)scrheight)*2.0f);

		float tx1 = qk->tx1, ty1 = qk->ty1;
		float tx2 = qk->tx2, ty2 = qk->ty2;

		float a = (qk->panel == QuickKey::PANEL_BL || qk->panel == QuickKey::PANEL_BR) ? s_joystickAlpha : s_QuickKeyAlpha;
        a *= s_autoHideAlpha;

		float r = s_QuickKeyLum;
		float g = s_QuickKeyLum;
		float b = s_QuickKeyLum;

		VirtKB_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, r, g, b, a);
	}
}

void updateQuickKeyColor(int qkID, QuickKey *qk, bool down)
{
	GLfloat *v = &s_QuickKeyVerts[qkID*36];

//	float a = down ? 0.5f : s_QuickKeyAlpha;
//	float r = s_QuickKeyLum;
//	float g = s_QuickKeyLum;
//	float b = down ? 1.0f : s_QuickKeyLum;

	float a = (qk->panel == QuickKey::PANEL_BL || qk->panel == QuickKey::PANEL_BR) ? s_joystickAlpha : s_QuickKeyAlpha;
    a *= s_autoHideAlpha;

	float r = down ? 0.1f : s_QuickKeyLum;
	float g = down ? 0.1f : s_QuickKeyLum;
	float b = s_QuickKeyLum;

	int c = 3;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a;
}

// touch priority (overlap cases) -> quick keys -> vkb -> joy/mouse
void VirtKB_updateInput()
{
	int prevIndex = 1 - s_curIndex;

	bool *prevtouched = s_curtouched[prevIndex];
	float *prevtouchX = s_curtouchX[prevIndex];
	float *prevtouchY = s_curtouchY[prevIndex];

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];

	bool hasTouches = false;

	BitFlags *prevButtons = s_curButtons[prevIndex];
	BitFlags *curButtons = s_curButtons[s_curIndex];

	int *prevButtonDownSet = s_buttonDown[prevIndex];
	int numPrevButtonDown = s_numButtonDown[prevIndex];
	int *curButtonDownSet = s_buttonDown[s_curIndex];
	int numCurButtonDown = 0;

	curButtons->clearAll();
	//s_recreateQuickKeys = false;
	VirtKB_resetVkbPresses();

	// retrieve current touches and check quick keys
	for (int i = 0; i < VKB_MaxTouches; ++i)
	{
		if (curtouched[i])
		{
			hasTouches = true;

			// quick keys
			if (s_curInputLayer == 0 || s_curInputLayer == InputLayer_QuickKeys || s_curInputLayer == InputLayer_VirtController)
			{
				for (int c = 0; c < s_numQuickKeys; ++c)
				{
					QuickKey *qk = &s_QuickKeys[c];
					const VirtKeyDef *vk = qk->pKeyDef;

					if (vk->flags & s_curInputFlags)
					{
						if (numCurButtonDown < MaxButtonDown
						 && curtouchX[i] >= qk->x1 && curtouchX[i] < qk->x2
						 && curtouchY[i] >= qk->y1 && curtouchY[i] < qk->y2)
						{
							curButtons->setBit(vk->id);
							curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vk->id, c);
							++numCurButtonDown;
							s_curInputLayer = InputLayer_QuickKeys;
						}
					}
				}
			}
		}
	}

	// screen / vkb zoom
	if (s_curInputLayer == 0 || s_curInputLayer == InputLayer_Zoom)
	{
		if (s_keyboardZoomMode || s_screenZoomMode)
		{
			int zoomPanCount = 0;
			zoomPanCount += curtouched[0] ? 1 : 0;
			zoomPanCount += curtouched[1] ? 1 : 0;

			if (zoomPanCount > 1 && s_prevZoomPanCount > 1)
			{
				//zoom
				float zx1 = prevtouchX[0] - prevtouchX[1], zy1 = prevtouchY[0] - prevtouchY[1];
				float zx2 = curtouchX[0] - curtouchX[1], zy2 = curtouchY[0] - curtouchY[1];
				float dist1 = sqrtf(zx1*zx1 + zy1*zy1);
				float dist2 = sqrtf(zx2*zx2 + zy2*zy2);

				if (s_keyboardZoomMode)		VirtKB_ZoomVKB(dist2 - dist1);
				else if (s_screenZoomMode)	Renderer_zoomEmuScreen(dist2 - dist1);

				s_curInputLayer = InputLayer_Zoom;
			}
			else if (zoomPanCount > 0 && s_prevZoomPanCount > 0)
			{
				// pan
				int finger = curtouched[0] ? 0 : 1;
				float px1 = curtouchX[finger] - prevtouchX[finger], py1 = curtouchY[finger] - prevtouchY[finger];

				if (s_keyboardZoomMode)		VirtKB_PanVKB(px1, py1);
				else if (s_screenZoomMode)	Renderer_panEmuScreen(px1, py1);

				s_curInputLayer = InputLayer_Zoom;
			}

			s_prevZoomPanCount = zoomPanCount;
		}
	}

	// vkb
	if (s_curInputLayer == 0 || s_curInputLayer == InputLayer_VirtKB)
	{
		if (s_showKeyboard && !(s_keyboardZoomMode || s_screenZoomMode))
		{
			for (int i = 0; i < VKB_MaxTouches; ++i)
			{
				if (numCurButtonDown >= MaxButtonDown)
				{
					break;
				}

				if (curtouched[i])
				{
					const VirtKeyDef *vk = VirtKB_VkbHitTest(curtouchX[i], curtouchY[i]);
					if (vk)
					{
						VirtKB_addVkbPress(vk->id, false);
						curButtons->setBit(vk->id);
						curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vk->id, INVALID_QUICK_KEY_ID);
						++numCurButtonDown;
						s_curInputLayer = InputLayer_VirtKB;
					}
				}
			}
		}
	}

	// input from android layer
	for (int f = 0; f < s_jKeyPresses->_flagSize32; ++f)
	{
		uint32_t g = s_jKeyPresses->_flags[f];
		if (g != 0)
		{
			int offset = (f << 5);
			for (int i = 0; i < 32; ++i)
			{
				if ((g & (1<<i)) != 0)
				{
					int vkID = offset + i;
					if (vkID < VKB_KEY_NumOf && !curButtons->getBit(vkID))
					{
						if (numCurButtonDown < MaxButtonDown)
						{
							//Debug_Printf("Input: %d\n", vkID);

							curButtons->setBit(vkID);
							curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vkID, INVALID_QUICK_KEY_ID);
							++numCurButtonDown;
						}
					}
				}
			}
		}
	}

	// add cur key focus (vkb) (non-touch mode)
	{
		if (s_showKeyboard && s_nonTouchKB)
		{
			Uint32 nowMS = SDL_GetTicks();
            Uint32 elapsedMS = nowMS - s_lastKBFocusRepeat;
            bool canNav = (elapsedMS > s_KBRepeatDelay) ? true: false;

            //Debug_Printf("~~~~~~~~~~~~~~~~~~~~~ CAN NAV: %d, elapsed: %d, repeatDelay: %ld, now: %ld, lastRepeat: %ld", canNav, elapsedMS, s_KBRepeatDelay, nowMS, s_lastKBFocusRepeat);

            bool navPressed = false;

			if (curButtons->getBit(VKB_KEY_NAVLEFT))		{ if (canNav) { VirtKB_NavLeft(); s_lastKBFocusRepeat = nowMS; } navPressed = true; }
			else if (curButtons->getBit(VKB_KEY_NAVRIGHT))	{ if (canNav) { VirtKB_NavRight(); s_lastKBFocusRepeat = nowMS; } navPressed = true; }
			else if (curButtons->getBit(VKB_KEY_NAVUP))		{ if (canNav) { VirtKB_NavUp(); s_lastKBFocusRepeat = nowMS; } navPressed = true; }
			else if (curButtons->getBit(VKB_KEY_NAVDOWN))	{ if (canNav) { VirtKB_NavDown(); s_lastKBFocusRepeat = nowMS; } navPressed = true; }

			if (curButtons->getBit(VKB_KEY_NAVBTN))
			{
				//VirtKB_NavAction();
				if (numCurButtonDown < MaxButtonDown)
				{
					int vkID = s_curKBKeyFocus;
					if (vkID >= 0 && vkID < VKB_KEY_NumOf)
					{
						const VirtKeyDef *vk = &g_vkbKeyDefs[vkID];

						VirtKB_addVkbPress(vk->id, false);
						curButtons->setBit(vk->id);
						curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vk->id, INVALID_QUICK_KEY_ID);
						++numCurButtonDown;
					}
				}
			}

			if (!navPressed)
			{
				s_lastKBFocusRepeat = 0;
				s_KBRepeatDelay = kInitKBRepeatDelay;
			}
			else
			{
				if (canNav)
				{
					s_KBRepeatDelay -= kKBRepeatDelayDeltaRate;
					s_KBRepeatDelay = (s_KBRepeatDelay < kMinKBRepeatDelay) ? kMinKBRepeatDelay : s_KBRepeatDelay;

				}
			}

			VirtKB_addVkbPress(s_curKBKeyFocus, true);

			if (hasTouches) // TODO: only if touch outside keyboard
			{
				VirtKB_ToggleKeyboard(0, 0, true); // hide keyboard
			}
		}
	}

	// new arcade virtual joystick
    if (_virtJoyEnabled && _virtJoy != 0)
    {
        _virtJoy->update(s_curInputLayer, s_curIndex, s_curtouched, s_curtouchX, s_curtouchY, VKB_MaxTouches);
    }
	if (s_curInputLayer == 0 || s_curInputLayer == InputLayer_VirtController || s_curInputLayer == InputLayer_QuickKeys) // joystick fire buttons on quick keys...
	{
		if (_virtJoyEnabled && _virtJoy != 0 && _virtJoy->isEnabled())
	    {
	        const int virtJoy1VKIDMap[] = {VKB_KEY_JOYUP, VKB_KEY_JOYDOWN, VKB_KEY_JOYLEFT, VKB_KEY_JOYRIGHT};
	        const int virtJoy2VKIDMap[] = {VKB_KEY_JOY2UP, VKB_KEY_JOY2DOWN, VKB_KEY_JOY2LEFT, VKB_KEY_JOY2RIGHT};

	        const int *virtJoyVKIDMap = (s_joyID==0) ? virtJoy2VKIDMap : virtJoy1VKIDMap;

	        unsigned int vjBtnDown = _virtJoy->getBtnFlags();
	        if (vjBtnDown != 0)
	        {
	            for (int i = 0; i < 4; ++i)
	            {
	                if (numCurButtonDown >= MaxButtonDown)
	                {
	                    break;
	                }

	                if ((vjBtnDown & (1<<i)) != 0)
	                {
	                    int vkID = virtJoyVKIDMap[i];
	                    curButtons->setBit(vkID);
	                    curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vkID, INVALID_QUICK_KEY_ID);
	                    ++numCurButtonDown;
	                    s_curInputLayer = InputLayer_VirtController;
	                }
	            }
	        }
	    }
    }

	s_numButtonDown[s_curIndex] = numCurButtonDown;

	// update button states
	s_changedButtons->xorAll(prevButtons, curButtons);

	// button releases
	for (int i = 0; i < numPrevButtonDown; ++i)
	{
		int vkID = GET_BUTTON_DOWN_VKID(prevButtonDownSet[i]);
		if (s_changedButtons->getBit(vkID))
		{
			const VirtKeyDef *vk = &g_vkbKeyDefs[vkID];
			KeyCallback* kcb = &s_keyCallbacks[vkID];
			int qkID = GET_BUTTON_DOWN_QKID(prevButtonDownSet[i]);
			QuickKey *qk = (qkID==INVALID_QUICK_KEY_ID) ? 0 : &s_QuickKeys[qkID];

			if (kcb->onKeyEvent != 0)
			{
				(*(kcb->onKeyEvent))(vk, kcb->uParam1, false);
			}

			if (qk != 0)
			{
				updateQuickKeyColor(qkID, qk, false);
			}
		}
	}

	// don't process new presses when we switch key layouts until fingers are off the screen
	if (s_waitInputCleared)
	{
		if (numCurButtonDown == 0)
		{
			s_waitInputCleared = false;
			Debug_Printf("Input Cleared");
		}
		return;
	}

	// button presses
	uint32_t prevInputFlags = s_curInputFlags;

	for (int i = 0; i < numCurButtonDown; ++i)
	{
		int vkID = GET_BUTTON_DOWN_VKID(curButtonDownSet[i]);
		if (s_changedButtons->getBit(vkID))
		{
			const VirtKeyDef *vk = &g_vkbKeyDefs[vkID];
			KeyCallback* kcb = &s_keyCallbacks[vkID];
			int qkID = GET_BUTTON_DOWN_QKID(curButtonDownSet[i]);
			QuickKey *qk = (qkID==INVALID_QUICK_KEY_ID) ? 0 : &s_QuickKeys[qkID];

			if (kcb->onKeyEvent != 0)
			{
				(*(kcb->onKeyEvent))(vk, kcb->uParam1, true);
			}

			if (qk != 0)
			{
				updateQuickKeyColor(qkID, qk, true);
			}
		}
	}

	// update mouse
	if (s_curInputLayer == 0 || s_curInputLayer == InputLayer_VirtController || s_curInputLayer == InputLayer_QuickKeys) // mouse buttons on quick keys...
	{
		if (((prevInputFlags&s_curInputFlags)&FLAG_MOUSE))
		{
			VirtKB_updateTouchMouse();
			if (s_prevFingerCount > 0)
			{
				s_curInputLayer = InputLayer_VirtController;
			}
		}

		VirtKB_updateHardwareMouse();
		VirtKB_updateVirtMouseEmu();

		if (s_mouseDx != 0.0f || s_mouseDy != 0.0f)
		{
			float emuScrZoomX = Renderer_getEmuScreenZoomX();
			float emuScrZoomY = Renderer_getEmuScreenZoomY();

			//s_mouseMoved = 1;

			float sX = 1;
			float sY = (STRes == ST_MEDIUM_RES) ? 0.5f : 1;
			sX *= s_mouseSpeed / emuScrZoomX;
			sY *= s_mouseSpeed / emuScrZoomY;

			int dx = (int)(s_mouseDx * sX);
			int dy = (int)(s_mouseDy * sY);
			if (dx != 0 || dy != 0)
			{
				hatari_onMouseMoved(dx, dy);
				s_mouseDx -= (float)dx/sX ; s_mouseDy -= (float)dy/sY;
			}
		}

		bool lmb = s_LMB || ((_curHWMouseBtns & 1)!=0);
		bool rmb = s_RMB || ((_curHWMouseBtns & 2)!=0);

		if (lmb)	{ Keyboard.bLButtonDown |= BUTTON_MOUSE; }
		else		{ Keyboard.bLButtonDown &= ~BUTTON_MOUSE; }

		if (rmb)	{ Keyboard.bRButtonDown |= BUTTON_MOUSE; }
		else		{ Keyboard.bRButtonDown &= ~BUTTON_MOUSE; }
	}

	//if (s_recreateQuickKeys)
	//{
	//	VirtKB_CreateQuickKeys();
	//}
}

static void VirtKB_updateHardwareMouse()
{
	if (_prevHWMouseX != MAXFLOAT)
	{
		float dmx = _curHWMouseX - _prevHWMouseX;
		float dmy = _curHWMouseY - _prevHWMouseY;
		_prevHWMouseX = _curHWMouseX;
		_prevHWMouseY = _curHWMouseY;

		if (dmx != 0.0f || dmy != 0.0f)
		{
			s_mouseDx += dmx;
			s_mouseDy += dmy;
		}
	}
}

static void VirtKB_updateVirtMouseEmu()
{
	if (isUserEmuPaused())
	{
		return;
	}

	float accel = 25; // TODO: user option

	float jx = s_jAxis[kAxis_X2];
	float jy = s_jAxis[kAxis_Y2];

	bool nonTouchKBActive = (s_showKeyboard && s_nonTouchKB);

	if (VirtKB_getMouseActive() && !nonTouchKBActive)
	{
		float jx1 = s_jAxis[kAxis_X1];
		float jy1 = s_jAxis[kAxis_Y1];

		bool dpadMouse = false;
		if (fabsf(jx1) < 0.001f && fabsf(jy1) < 0.001f) {
			BitFlags *curButtons = s_curButtons[s_curIndex];

			if (curButtons->getBit(VKB_KEY_JOYLEFT))		{ jx1 = -_curDPADMouseAccel; dpadMouse  = true; }
			else if (curButtons->getBit(VKB_KEY_JOYRIGHT))	{ jx1 = _curDPADMouseAccel; dpadMouse  = true; }
			if (curButtons->getBit(VKB_KEY_JOYUP))		    { jy1 = -_curDPADMouseAccel; dpadMouse  = true; }
			else if (curButtons->getBit(VKB_KEY_JOYDOWN))	{ jy1 = _curDPADMouseAccel; dpadMouse  = true; }
		}
		if (dpadMouse) {
			float dT = 1/60.0f; // TODO
			_curDPADMouseAccel += dT;
			if (_curDPADMouseAccel > 1) {
				_curDPADMouseAccel = 1;
			}
		} else {
			_curDPADMouseAccel = 0.1f;
		}

		if (fabsf(jx1) > fabsf(jx))
		{
			jx = jx1;
		}
		if (fabsf(jy1) > fabsf(jy))
		{
			jy = jy1;
		}
	}

	s_mouseDx += jx * accel;
	s_mouseDy += jy * accel;
}

static void VirtKB_updateTouchMouse()
{
	int prevIndex = 1 - s_curIndex;

	bool *prevtouched = s_curtouched[prevIndex];
	float *prevtouchX = s_curtouchX[prevIndex];
	float *prevtouchY = s_curtouchY[prevIndex];

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];

	const float kMoveTimeThresh = 16.0f/60.0f;
	float dt = 1.f/60.f; // TODO: calculate real value
	int fingers = 0;


	int prevMousePresses = s_mousePresses;
	int curMousePresses = 0;//s_mousePresses;


	for (int i = 0; i < VKB_MaxTouches; ++i)
	{
		if (curtouched[i]) { ++fingers; }
	}

	if (s_mouseFinger < 0)
	{
		bool isButtonMode = (s_curInputFlags & FLAG_MOUSEBUTTON) != 0;

		for (int i = 0; i < VKB_MaxTouches; ++i)
		{
			if (curtouched[i])
			{
				// HACK
				if (isButtonMode)
				{
					bool isMouseMove = true;
					for (int h = 0; h < 2; ++h)
					{
						int qkIdx = s_mouseButtonIgnoreQuickKeyIdx[h];
						if (qkIdx >= 0)
						{
							QuickKey *qk = &s_QuickKeys[qkIdx];
							const VirtKeyDef *vk = qk->pKeyDef;

							if (curtouchX[i] >= qk->x1 && curtouchX[i] < qk->x2
							 && curtouchY[i] >= qk->y1 && curtouchY[i] < qk->y2)
							{
								isMouseMove = false;
								break;
							}
						}
					}

					if (!isMouseMove)
					{
						continue;
					}
				}

				s_mouseFinger = i;
				//Debug_Printf("MouseFinger set: %d", s_mouseFinger);

				s_prevMouseX = curtouchX[i];
				s_prevMouseY = curtouchY[i];
				s_mouseDragStartX = s_prevMouseX;
				s_mouseDragStartY = s_prevMouseY;
				s_mouseMoved = 0;

				s_mousePresses = 0;
				s_mousePressTime = 0;

				s_mouseMoveOnly = false;
			}
		}
	}
	else
	{
		if (curtouched[s_mouseFinger])
		{
			if (!s_mouseMoveOnly)
			{
				s_mousePressTime += dt;
				if (s_mousePressTime >= kMoveTimeThresh)
				{
					curMousePresses = fingers;
				}
			}
		}
		else
		{
			//Debug_Printf("MouseFinger cleared: %d", s_mouseFinger);
			curMousePresses = (!s_mouseMoved && s_mousePressTime < kMoveTimeThresh) ? (s_prevFingerCount>0?s_prevFingerCount:1) : 0;//s_mouseMoveOnly ? 0 : s_prevFingerCount);

			s_mouseFinger = -1;
			s_mousePressTime = 0;
			s_mouseDx = 0;
			s_mouseDy = 0;
		}
	}

	if (s_mouseFinger >= 0)
	{
		float dx = curtouchX[s_mouseFinger] - s_prevMouseX;
		float dy = curtouchY[s_mouseFinger] - s_prevMouseY;

		s_mouseDx += dx;
		s_mouseDy += dy;

		s_prevMouseX = curtouchX[s_mouseFinger];
		s_prevMouseY = curtouchY[s_mouseFinger];

		if (!s_mouseMoveOnly)
		{
			int scrwidth = getScreenWidth();
			int scrheight = getScreenHeight();

			float threshX = scrwidth * 0.01f;
			float threshY = scrheight * 0.01f;

			float mdx = curtouchX[s_mouseFinger] - s_mouseDragStartX;
			float mdy = curtouchY[s_mouseFinger] - s_mouseDragStartY;

			if (fabsf(mdx) > threshX || fabsf(mdx) > threshY)
			{
				s_mouseMoved = 1;

				if (s_mousePressTime <= kMoveTimeThresh) {
					s_mouseMoveOnly = true;
				}
			}
		}
	}

	if (prevMousePresses != curMousePresses)
	{
		if ((s_curInputFlags & (FLAG_MOUSEBUTTON)) == 0)
		{
			if (prevMousePresses & 1) { s_LMB = false; }
			if (prevMousePresses & 2) { s_RMB = false; }

			if (curMousePresses != 0)
			{
				//Debug_Printf("Mouse Pressed: %d", curMousePresses);
				if (curMousePresses & 1) { s_LMB = true; }
				if (curMousePresses & 2) { s_RMB = true; }
			}
			else
			{
				//Debug_Printf("Mouse Released: %d", prevMousePresses);
			}
		}

		s_mousePresses = curMousePresses;
	}

	s_prevFingerCount = fingers;
}

static void VirtKB_resetVkbPresses()
{
	s_VkbCurNumPresses = 0;
}

static void VirtKB_addVkbPress_Rect(const VirtKeyDef *k, bool focusKey);
static void VirtKB_addVkbPress_Poly(const VirtKeyDef *k, bool focusKey);

static void VirtKB_addVkbPress(int vkbKeyID, bool focusKey)
{
	const VirtKeyDef *k = &g_vkbKeyDefs[vkbKeyID];
	if (k->flags & FLAG_POLY)
	{
		VirtKB_addVkbPress_Poly(k, focusKey);
	}
	else
	{
		VirtKB_addVkbPress_Rect(k, focusKey);
	}
}

static void VirtKB_addVkbPress_Rect(const VirtKeyDef *k, bool focusKey)
{
	int numKeys = 1;
	short onekey[1] = {(short)k->id };
	const short* keys = onekey;
	if (k->numRefs > 1)
	{
		numKeys = k->numRefs;
		keys = k->pRefs;
	}

	float curZoomKB = s_vkbZoom;
	int curOffX  = 0, curOffY = 0;
	VirtKB_GetVkbScreenOffset(&curOffX, &curOffY);

	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	for (int i = 0; (i < numKeys) && (s_VkbCurNumPresses < s_MaxVkbPresses); ++i)
	{
		k = &g_vkbKeyDefs[keys[i]];

		float keyX1 = (curOffX + ((float)k->v[0]*curZoomKB));
		float keyX2 = (curOffX + ((float)k->v[2]*curZoomKB));
		float keyY1 = (curOffY + ((float)k->v[1]*curZoomKB));
		float keyY2 = (curOffY + ((float)k->v[3]*curZoomKB));

		// for keys that span multiple row defs
		if (i < numKeys-1)
		{
			const VirtKeyDef *k2 = &g_vkbKeyDefs[keys[i+1]];
			keyY2 = (curOffY + ((float)k2->v[1]*curZoomKB));
		}

		GLfloat *v = &s_VkbPressedVerts[s_VkbCurNumPresses*36];

		float x1 = ((keyX1/(float)scrwidth)*2.0f) - 1.0f;
		float y1 = 1.0f - ((keyY1/(float)scrheight)*2.0f);
		float x2 = ((keyX2/(float)scrwidth)*2.0f) - 1.0f;
		float y2 = 1.0f - ((keyY2/(float)scrheight)*2.0f);

		float tx1 = 0.0f, ty1 = 0.0f;
		float tx2 = 1.0f, ty2 = 1.0f;

		float a = 0.8f;
		float r = 0.1f;
		float g, b;
		if (focusKey)	{ g = 1.0f; b = 0.1f; }
		else			{ g = 0.1f; b = 1.0f; }

		VirtKB_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, r, g, b, a);

		++s_VkbCurNumPresses;
	}
}

static void VirtKB_addVkbPress_Poly(const VirtKeyDef *k, bool focusKey)
{
	float curZoomKB = s_vkbZoom;
	int curOffX  = 0, curOffY = 0;
	VirtKB_GetVkbScreenOffset(&curOffX, &curOffY);

	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	if (s_VkbCurNumPresses < s_MaxVkbPresses)
	{
		float keyX[4];
		float keyY[4];
		float f;
		for (int j = 0; j < 4; ++j)
		{
			f = (curOffX + ((float)k->v[(j<<1)]*curZoomKB));
			keyX[j] = ((f/(float)scrwidth)*2.0f) - 1.0f;

			f = (curOffY + ((float)k->v[(j<<1)+1]*curZoomKB));
			keyY[j] = 1.0f - ((f/(float)scrheight)*2.0f);
		}

		GLfloat *v = &s_VkbPressedVerts[s_VkbCurNumPresses*36];

		float texU[4] = {0, 0, 1, 1};
		float texV[4] = {0, 1, 1, 0};

		float a = 0.8f;
		float r = 0.1f;
		float g, b;
		if (focusKey)	{ g = 1.0f; b = 0.1f; }
		else			{ g = 0.1f; b = 1.0f; }

		VirtKB_UpdatePolyVerts(v, keyX, keyY, texU, texV, r, g, b, a);

		++s_VkbCurNumPresses;
	}
}

static void VirtKB_UpdateRectVerts(	GLfloat *v, float x1, float y1, float x2, float y2,
								float u1, float v1, float u2, float v2,
								float r, float g, float b, float a)
{
	float x[4] = {x1, x1, x2, x2 };
	float y[4] = {y1, y2, y2, y1 };
	float tu[4] = {u1, u1, u2, u2 };
	float tv[4] = {v1, v2, v2, v1 };

	int c = 0;
	for (int i = 0; i < 4; ++i)
	{
		v[c++] = x[i];	v[c++] = y[i];	v[c++] = 0;
		v[c++] = r;		v[c++] = g;		v[c++] = b;		v[c++] = a;
		v[c++] = tu[i];	v[c++] = tv[i];
	}
}


static void VirtKB_UpdatePolyVerts(GLfloat *v, float *x, float *y, float *tu, float *tv, float r, float g, float b, float a)
{
	int c = 0;
	for (int i = 0; i < 4; ++i)
	{
		v[c++] = x[i];	v[c++] = y[i];	v[c++] = 0;
		v[c++] = r;		v[c++] = g;		v[c++] = b;		v[c++] = a;
		v[c++] = tu[i];	v[c++] = tv[i];
	}
}

static void VirtKB_ZoomVKB(float absChange)
{
	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	if (scrwidth > 0 && scrheight > 0)
	{
		float nScale = 2.0f / (float)(scrwidth + scrheight);
		float deltaZ = absChange * nScale;

		s_vkbZoom += deltaZ;
		if (s_vkbZoom < s_VkbMinZoom)
		{
			s_vkbZoom = s_VkbMinZoom;
		}

		VirtKB_UpdateVkbVerts();
	}
}

static void VirtKB_PanVKB(float absDX, float absDY)
{
	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	if (scrwidth > 0 && scrheight > 0)
	{
		float nScale = 1.0f;//2.0f / (float)(gScrWidth + gScrHeight);
		float dX = absDX * nScale;
		float dY = absDY * nScale;

		s_vkbPanX += dX;
		s_vkbPanY += dY;

		VirtKB_UpdateVkbVerts();
	}
}

static void VirtKB_GetVkbScreenOffset(int *x, int *y)
{
	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	float px1 = s_vkbPanX + (scrwidth - g_vkbTexKbW*s_vkbZoom) / 2;
	float py1 = s_vkbPanY + (scrheight - g_vkbTexKbH*s_vkbZoom);

	*x = (int)px1;
	*y = (int)py1;
}

static void VirtKB_UpdateVkbVerts()
{
	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();
	if (scrwidth > 0 && scrheight > 0)
	{
		GLfloat *v = s_VkbVerts;

		int tw = s_VkbGPUTexWidth;
		int th = s_VkbGPUTexHeight;

		int scrwidth = getScreenWidth();
		int scrheight = getScreenHeight();

		int kbx1 = 0, kbx2 = 0;
		VirtKB_GetVkbScreenOffset(&kbx1, &kbx2);

		float px1 = kbx1;
		float py1 = kbx2;
		float px2 = px1 + g_vkbTexKbW*s_vkbZoom;
		float py2 = py1 + g_vkbTexKbH*s_vkbZoom;

		float x1 = ((px1/(float)scrwidth)*2.0f) - 1.0f;
		float y1 = 1.0f - ((py1/(float)scrheight)*2.0f);
		float x2 = ((px2/(float)scrwidth)*2.0f) - 1.0f;
		float y2 = 1.0f - ((py2/(float)scrheight)*2.0f);

		float tx1 = 0.0f, ty1 = 0.0f;
		float tx2 = (float)g_vkbTexKbW/(float)tw, ty2 = (float)g_vkbTexKbH/(float)th;

		float a = s_VkbAlpha * s_autoHideAlpha;
		float r = s_VkbLum;
		float g = s_VkbLum;
		float b = s_VkbLum;

		VirtKB_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, r, g, b, a);
	}
}

static bool VirtKB_PointInPoly(float x, float y, const short *v, int numVerts)
{
	int vtid = (numVerts-1)<<1;
	float sx = v[vtid], sy = v[vtid+1];
	float ex = v[0], ey = v[1];

	vtid = 2;
	int bInside = 0, bYFlag0 = sy >= y, bYFlag1;
	for (int i = numVerts+1; --i; )
	{
		bYFlag1 = (ey >= y);
		if (bYFlag0 != bYFlag1)
		{
			if ( ((ey-y) * (sx-ex) >= (ex-x) * (sy-ey)) == bYFlag1 )
			{
				bInside = !bInside;
			}
		}

		bYFlag0 = bYFlag1;
		sx = ex; sy = ey;
		ex = v[vtid++]; ey = v[vtid++];
	}
	return (bInside!=0);
}

static const VirtKeyDef *VirtKB_VkbHitTest(float x, float y)
{
	float curZoomKB = s_vkbZoom;
	int curOffX  = 0, curOffY = 0;
	VirtKB_GetVkbScreenOffset(&curOffX, &curOffY);

	// convert to vkb space
	x = (x - curOffX) / curZoomKB;
	y = (y - curOffY) / curZoomKB;

	const RowSearch *row = 0;
	for (int i = 0; i < g_vkbRowSearchSize; ++i)
	{
		const RowSearch *curRow = &g_vkbRowSearch[i];
		if (y >= curRow->minY && y < curRow->maxY)
		{
			row = curRow;
			break;
		}
	}

	const VirtKeyDef *vk;
	if (row)
	{
		vk = &g_vkbKeyDefs[row->minID];
		if (vk->flags & FLAG_STFNKEY)
		{
			// just search through all the fn keys since there aren't that many
			for (int i = row->minID; i <= row->maxID; ++i)
			{
				vk = &g_vkbKeyDefs[i];
				if (VirtKB_PointInPoly(x, y, vk->v, 4))
				{
					return vk;
				}
			}
		}
		else
		{
			int minID = row->minID;
			int maxID = row->maxID;
			int curID = minID + ((maxID-minID)>>1);
			for (;;)
			{
				vk = &g_vkbKeyDefs[curID];
				if (x >= vk->v[0])
				{
					if (x < vk->v[2])
					{
						return vk;
					}
					else
					{
						minID = curID+1;
						if (minID > maxID)
						{
							return 0;
						}
					}
				}
				else
				{
					maxID = curID-1;
					if (maxID < minID)
					{
						return 0;
					}
				}

				curID = minID + ((maxID-minID)>>1);
			}
		}
	}

	return 0;
}

static void VirtKB_ToggleTurboSpeed(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		int curTurbo = getTurboSpeed();
		setTurboSpeed(1-curTurbo);

		s_recreateQuickKeys = true;
	}
}

static int _VirtKB_FindCurFocusRow()
{
	const RowSearch *row = 0;
	for (int i = 0; i < g_vkbRowSearchSize; ++i)
	{
		const RowSearch *curRow = &g_vkbRowSearch[i];
		if (s_curKBKeyFocus >= curRow->minID && s_curKBKeyFocus <= curRow->maxID)
		{
			return i;
		}
	}
	return -1;
}

static int _findClosestKey(int curVKeyID, int nextRowIdx, int dir)
{
	// cmp avg x
	const VirtKeyDef* curKey = &g_vkbKeyDefs[curVKeyID];
	int curKeyVKBID = curKey->id;
	float curX = (curKey->v[0] + curKey->v[2]) * 0.5f;

	float minDiff = MAXFLOAT;
	int minKey = -1;
	// just do a linear search through row of keys
	const RowSearch *nextRow = &g_vkbRowSearch[nextRowIdx];
	for (int i = nextRow->minID; i <= nextRow->maxID; ++i)
	{
		const VirtKeyDef* cmpKey = &g_vkbKeyDefs[i];
		float cmpX = (cmpKey->v[0] + cmpKey->v[2]) * 0.5f;
		float diff = curX - cmpX;
		if (diff < 0.0f) { diff *= -1.0f; }
		if (diff < minDiff)
		{
			minDiff = diff;
			minKey = i;
		}
	}
	if (minKey < 0) { minKey = nextRow->minID; }

	const VirtKeyDef* minKeyDef = &g_vkbKeyDefs[minKey];
	int minKeyVKBID = minKeyDef->id;
	if (minKeyVKBID == curKeyVKBID) // (big keys)
	{
		if (dir != 0)
		{
			// search one more row
			int lastRowIdx = nextRowIdx + dir;
			if (lastRowIdx >= 0 && lastRowIdx < g_vkbRowSearchSize)
			{
				return _findClosestKey(curVKeyID, lastRowIdx, 0);
			}
		}
	}

	return minKey;
}

static void VirtKB_NavLeft()
{
	int curRowIdx = _VirtKB_FindCurFocusRow();
	if (curRowIdx >= 0)
	{
		const RowSearch *curRow = &g_vkbRowSearch[curRowIdx];
		if (s_curKBKeyFocus > curRow->minID)
		{
			--s_curKBKeyFocus;
		}
	}
}
static void VirtKB_NavRight()
{
	int curRowIdx = _VirtKB_FindCurFocusRow();
	if (curRowIdx >= 0)
	{
		const RowSearch *curRow = &g_vkbRowSearch[curRowIdx];
		if (s_curKBKeyFocus < curRow->maxID)
		{
			++s_curKBKeyFocus;
		}
	}
}
static void VirtKB_NavUp()
{
	int curRowIdx = _VirtKB_FindCurFocusRow();
	if (curRowIdx > 0)
	{
		int nextRowIdx = curRowIdx - 1;
		s_curKBKeyFocus = _findClosestKey(s_curKBKeyFocus, nextRowIdx, -1);
	}
}
static void VirtKB_NavDown()
{
	int curRowIdx = _VirtKB_FindCurFocusRow();
	if (curRowIdx >= 0 && curRowIdx < (g_vkbRowSearchSize-1))
	{
		int nextRowIdx = curRowIdx + 1;
		s_curKBKeyFocus = _findClosestKey(s_curKBKeyFocus, nextRowIdx, 1);
	}
}

static void VirtKB_ToggleKeyboard(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		s_showKeyboard = !s_showKeyboard;
		if (s_showKeyboard)
		{
			s_prevInputFlags = s_curInputFlags;
			if (s_isPortrait) {
				s_curInputFlags |= FLAG_VKB;
			} else {
				s_curInputFlags = FLAG_PERSIST|FLAG_VKB;
			}
		}
		else
		{
			if (s_isPortrait) {
				s_curInputFlags &= ~FLAG_VKB;
			} else {
				s_curInputFlags = s_prevInputFlags;
			}
		}
		s_recreateQuickKeys = true;
		s_prevZoomPanCount = 0;
		s_keyboardZoomMode = 0;
		s_screenZoomMode = 0;

		VirtKB_clearMousePresses();
	}
}

static void VirtkKB_ScreenZoomToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		VirtKB_setScreenZoomMode(!s_screenZoomMode);
	}
}

void VirtKB_setScreenZoomMode(bool set)
{
	if (s_screenZoomMode == set)
	{
		return;
	}

	s_screenZoomMode = set;

	if (s_screenZoomMode)
	{
		s_prevInputFlags = s_curInputFlags;
		s_curInputFlags = FLAG_SCREEN | FLAG_SCREEN2;

		s_keyboardZoomMode = 0;
	}
	else
	{
		s_curInputFlags = s_prevInputFlags;
	}
	s_recreateQuickKeys = true;
	s_prevZoomPanCount = 0;

	VirtKB_clearMousePresses();
}

static void VirtkKB_VkbZoomToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		s_keyboardZoomMode = !s_keyboardZoomMode;
		s_prevZoomPanCount = 0;
		s_recreateQuickKeys = true;

		VirtKB_clearMousePresses();

		if (s_isPortrait)
		{
			if (s_keyboardZoomMode)
			{
				s_prevInputFlags = s_curInputFlags;
				s_curInputFlags &= ~(FLAG_SCREEN | FLAG_SCREEN2 | FLAG_MOUSE | FLAG_JOY);// | FLAG_MOUSEBUTTON);
			}
			else
			{
				s_curInputFlags = s_prevInputFlags;
			}
		}

	}
}

static void VirtkKB_ScreenPresetToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		s_curScreenZoomPreset = (s_curScreenZoomPreset + 1) % ScreenZoom_NumOf;
		Renderer_setZoomPreset(s_curScreenZoomPreset);
	}
}

void VirtKB_setDefaultScreenZoomPreset()
{
	s_curScreenZoomPreset = ScreenZoom_Fit;
	Renderer_setZoomPreset(s_curScreenZoomPreset);
}

static void VirtkKB_KeyboardPresetToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		VirtkKB_KeyboardSetPreset((s_curKeyboardZoomPreset + 1) % VkbZoom_NumOf);
	}
}

void VirtKB_setDefaultVKBZoomPreset()
{
	if (s_VkbZoomInited) {
		VirtkKB_KeyboardSetPreset(VkbZoom_Fit);
	}
}

static bool VirtkKB_KeyboardSetPreset(int preset)
{
	if (g_vkbTexKbW > 0)
	{
		s_curKeyboardZoomPreset = preset;

		int scrwidth = getScreenWidth();
		int scrheight = getScreenHeight();
		switch (preset)
		{
			case VkbZoom_1:
			{
				s_vkbZoom = (float)scrwidth / (float)500;
				float px1 = -(14*s_vkbZoom);
				float py1 = scrheight-((g_vkbTexKbH-14)*s_vkbZoom);
				s_vkbPanX = px1 - ((scrwidth - (g_vkbTexKbW*s_vkbZoom)) / 2);
				s_vkbPanY = py1 - (scrheight - (g_vkbTexKbH*s_vkbZoom));
				break;
			}
			case VkbZoom_2:
			{
				s_vkbZoom = (float)scrwidth / (float)604;
				float px1 = -(14*s_vkbZoom);
				float py1 = scrheight-((g_vkbTexKbH-14)*s_vkbZoom);
				s_vkbPanX = px1 - ((scrwidth - (g_vkbTexKbW*s_vkbZoom)) / 2);
				s_vkbPanY = py1 - (scrheight - (g_vkbTexKbH*s_vkbZoom));
				break;
			}
			case VkbZoom_Fit:
			default:
			{
				s_vkbZoom = (float)scrwidth / (float)g_vkbTexKbW;
				s_vkbPanX = 0;
				s_vkbPanY = 0;
				break;
			}
		}

		if (s_isPortrait) {
			// anchor keyboard above joystick controls
			float jsize = (_virtJoy != 0) ? ((_virtJoy->calcScreenRadiusPixels() * 2) + 32): (scrheight * (1/3.0f));
			s_vkbPanY -= jsize;
		}

		VirtKB_UpdateVkbVerts();
		return true;
	}

	return false;
}

void VirtKB_SetNonTouchKB(bool nonTouch)
{
	s_nonTouchKB = nonTouch;

	if (s_nonTouchKB) {
		// force full keyboard
		VirtkKB_KeyboardSetPreset(VkbZoom_Fit);
	}

	VirtKB_RefreshKB();
}

static void VirtKB_MJToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		if (!s_isPortrait && (s_showKeyboard || s_screenZoomMode))	{ s_prevInputFlags ^= (FLAG_MOUSE|FLAG_JOY); }
		else									                    { s_curInputFlags ^= (FLAG_MOUSE|FLAG_JOY); }

		s_recreateQuickKeys = true;

		VirtKB_clearMousePresses();
	}
}

static void VirtKB_clearMousePresses()
{
	if (s_mousePresses > 0)
	{
		Debug_Printf("Mouse Released: %d", s_mousePresses);

		if (s_mousePresses & 1) { s_LMB = false; }
		if (s_mousePresses & 2) { s_RMB = false; }

		Keyboard.bLButtonDown &= ~BUTTON_MOUSE;
		Keyboard.bRButtonDown &= ~BUTTON_MOUSE;

		s_mousePresses = 0;
	}

	s_waitInputCleared = true;

	Debug_Printf("mouse reset");
	s_mouseFinger = -1;
	s_mouseMoved = 0;
	s_prevMouseX = 0;
	s_prevMouseY = 0;
	s_mouseDx = 0;
	s_mouseDy = 0;

	s_prevFingerCount = 0;
	s_mousePresses = 0;
	s_mousePressTime = 0;

	_curDPADMouseAccel = 0.1f;
}

static void VirtKB_MouseLB(bool down)
{
	if (down)	{ s_LMB = true; }
	else		{ s_LMB = false; }
}

static void VirtKB_MouseRB(bool down)
{
	if (down)	{ s_RMB = true; }
	else		{ s_RMB = false; }
}

void VirtKB_SetControlAlpha(float alpha)
{
	s_QuickKeyAlpha = alpha;
	s_VkbAlpha = alpha;
	s_recreateQuickKeys = true;

	VirtKB_UpdateVkbVerts();
}

void VirtKB_SetJoystickAlpha(float alpha)
{
	s_joystickAlpha = alpha;
	s_recreateQuickKeys = true;

	VirtKB_UpdateVkbVerts();

	if (_virtJoy != 0)
    {
        _virtJoy->setAlpha(alpha);
    }
}

void VirtKB_SetJoystickSize(float size)
{
	s_joystickSize = size;
	s_recreateQuickKeys = true;

	if (_virtJoy != 0)
    {
        _virtJoy->setScale(size);
    }
}

void VirtKB_SetJoystickFireSize(float size)
{
	s_joystickFireSize = size;
	s_recreateQuickKeys = true;
}

//void VirtKB_setExtraKeys(bool set)
//{
//	s_vkbExtraKeys = set;
//	s_recreateQuickKeys = true;
//}

void VirtKB_setObsessionKeys(bool set)
{
	//g_doubleBuffer = set ? 0 : 1;

	s_vkbObsessionKeys = set;
	s_recreateQuickKeys = true;
}

void _setAutoHideAlpha(float a)
{
    s_autoHideAlpha = a;
    if (_virtJoy != 0)
    {
        _virtJoy->setAutoHideAlpha(s_autoHideAlpha);
    }

}
void VirtKB_setAutoHide(bool set)
{
    s_autoHide = set;
    _setAutoHideAlpha(set ? 0 : 1);
    s_autoHideInput = false;
    s_autoHideDelay = 0;

    s_recreateQuickKeys = true;
}

void VirtKB_updateAutoHide(bool touched)
{
    if (!s_autoHide && !s_hideAll)
    {
        return;
    }

    if (s_showKeyboard && s_nonTouchKB)
    {
    	touched = true;
    }

    bool refreshVerts = false;

    if (touched != s_autoHideInput)
    {
        s_autoHideInput = touched;
        if (touched)
        {
            _setAutoHideAlpha(1);
            refreshVerts = true;
        }
        if (!touched)
        {
            s_autoHideTicks = SDL_GetTicks();
            s_autoHideDelay = AUTOHIDE_DELAY_SECS;
        }
    }

    if (!s_autoHideInput)
    {
        if (s_autoHideAlpha > 0.0f)
        {
            long nowMS = SDL_GetTicks();
            long elapsedMS = nowMS - s_autoHideTicks;
            float deltaSecs = elapsedMS * (1.0f/1000.0f);

            if (s_autoHideDelay > 0.0f)
            {
                s_autoHideDelay -= deltaSecs;
            }
            else
            {
                float alpha = s_autoHideAlpha - (2.0f * deltaSecs);
                if (alpha < 0.0f)
                {
                    alpha = 0.0f;
                }
                _setAutoHideAlpha(alpha);
            }
            refreshVerts = true;

            s_autoHideTicks = nowMS;
        }
    }

    if (refreshVerts)
    {
        VirtKB_UpdateQuickKeyVerts();
        VirtKB_UpdateVkbVerts();
    }
}

void VirtKB_setHideAll(bool set)
{
	s_hideAll = set;
	s_recreateQuickKeys = true;

	if (_virtJoy != 0)
    {
        _virtJoy->setHide(set);
    }

    if (!s_hideAll)
    {
        s_autoHideInput = true;
        _setAutoHideAlpha(1);

        VirtKB_UpdateQuickKeyVerts();
        VirtKB_UpdateVkbVerts();
    }
}

static void VirtKB_ToggleShowUI(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
        VirtKB_setHideAll(!s_hideAll);
		s_recreateQuickKeys = true;
	}
}

static void VirtKB_TogglePause(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		toggleUserEmuPaused();
	}
}

void VirtKB_setJoystickOnly(bool set)
{
	s_showJoystickOnly = set;
	s_recreateQuickKeys = true;
}

void VirtKB_setHideExtraJoyKeys(bool hide)
{
	s_hideExtraJoyKeys = hide;
	s_recreateQuickKeys = true;
}

void VirtKB_setHideShortcutKeys(bool hide)
{
	s_hideShortcutKeys = hide;
	s_recreateQuickKeys = true;
}

//void VirtKB_setHideTurboKeys(bool hide)
//{
//	s_hideTurboKey = hide;
//	s_recreateQuickKeys = true;
//}

static void VirtKB_ToggleAutoFire(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		int curShortcutAutoFire = getShortcutAutoFire();
		bool newAutoFire = (curShortcutAutoFire < 0 || curShortcutAutoFire == 0) ? true : false;

		setShortcutAutoFire(true, newAutoFire);
	}
}

static void VirtKB_QuickSaveState(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { quickSaveStore(s_curEnv); } }
static void VirtKB_QuickLoadState(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { quickSaveLoad(s_curEnv); } }

static void VirtKB_ShowSoftMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { showSoftMenu(s_curEnv, 0); } }
static void VirtKB_ShowSoftFloppyMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { showSoftMenu(s_curEnv, 1); } }
static void VirtKB_ShowSettingsMenu(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { showOptionsDialog(s_curEnv); } }
static void VirtKB_ShowFloppyAInsert(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { showFloppyAInsert(s_curEnv); } }
static void VirtKB_ShowFloppyBInsert(const VirtKeyDef *keyDef, uint32_t uParam1, bool down) { if (down) { showFloppyBInsert(s_curEnv); } }

void VirtKB_ResetAllInputPresses()
{
/*	Keyboard.bLButtonDown &= ~BUTTON_MOUSE;
	Keyboard.bRButtonDown &= ~BUTTON_MOUSE;

	KeyboardProcessor.Mouse.dx = 0;
	KeyboardProcessor.Mouse.dy = 0;

	Joy_SetCustomEmu(0);
	Joy_SetCustomEmu(1);

	//for (int i = 0; i < 128; ++i)
	//{
	//	IKBD_PressSTKey(i, false);
	//}
*/
}

float VirtKB_getVKBZoom() { return s_vkbZoom; }
float VirtKB_getVKBPanX() { return s_vkbPanX; }
float VirtKB_getVKBPanY() { return s_vkbPanY; }

void VirtKB_SetVKBPanZoom(float kbdZoom, float kbdPanX, float kbdPanY)
{
	s_vkbZoom = kbdZoom;
	s_vkbPanX = kbdPanX;
	s_vkbPanY = kbdPanY;

	if (s_vkbZoom < s_VkbMinZoom)
	{
		s_vkbZoom = s_VkbMinZoom;
	}

	VirtKB_UpdateVkbVerts();
}

bool VirtKB_getMouseActive()
{
	if (!s_isPortrait && (s_showKeyboard || s_screenZoomMode))
	{
		return (s_prevInputFlags & FLAG_MOUSE) != 0;
	}
	return (s_curInputFlags & FLAG_MOUSE) != 0;
}

void VirtKB_SetMouseActive(bool mouseActive)
{
	if (!s_isPortrait && (s_showKeyboard || s_screenZoomMode))
	{
		s_prevInputFlags &= ~(FLAG_MOUSE|FLAG_JOY);
		s_prevInputFlags |= mouseActive ? FLAG_MOUSE : FLAG_JOY;
	}
	else
	{
		s_curInputFlags &= ~(FLAG_MOUSE|FLAG_JOY);
		s_curInputFlags |= mouseActive ? FLAG_MOUSE : FLAG_JOY;
	}

	s_recreateQuickKeys = true;

	VirtKB_clearMousePresses();
}

void VirtKB_setShortcutKeysFromPrefs(const char *pref)
{
	s_shortcutMap->setFromPrefString(pref);
	s_recreateQuickKeys = true;
}

void VirtKB_setHideJoystick(bool set)
{
	s_hideJoystick = set;
	s_recreateQuickKeys = true;
}

void VirtKB_SetKeySizeVX(float size)
{
	s_keySizeVX = size;
	s_recreateQuickKeys = true;
}

void VirtKB_SetKeySizeVY(float size)
{
	s_keySizeVY = size;
	s_recreateQuickKeys = true;
}

void VirtKB_SetKeySizeHX(float size)
{
	s_keySizeHX = size;
	s_recreateQuickKeys = true;
}

void VirtKB_SetKeySizeHY(float size)
{
	s_keySizeHY = size;
	s_recreateQuickKeys = true;
}

void VirtKB_VJStickEnable(bool useVJStick)
{
    _virtJoyEnabled = useVJStick;
    if (_virtJoy != 0)
    {
        _virtJoy->setEnabled(_virtJoyEnabled);
    }

    // clear current joystick buttons
    //_clearJoystickInput();

    s_recreateQuickKeys = true;
}

void VirtKB_VJStickSetFloating(bool vjStickFloating)
{
    _virtJoyFloating = vjStickFloating;
    if (_virtJoy != 0)
    {
        _virtJoy->setFloating(_virtJoyFloating);
    }
}

void VirtKB_VJStickSetDeadZone(float size)
{
    _virtJoyDeadZone = size;
    if (_virtJoy != 0)
    {
        _virtJoy->setDeadZone(size);
    }
}

void VirtKB_VJStickSetDiagSensitivity(float sensitivity)
{
    _virtJoyDiagSensitivity = sensitivity;
    if (_virtJoy != 0)
    {
        _virtJoy->setDiagSensitivity(sensitivity);
    }
}
