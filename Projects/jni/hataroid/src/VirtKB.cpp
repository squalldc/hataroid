#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <sdl.h>

#include "VirtKBDefs.h"
#include "VirtKBTex.h"
#include "uncompressGZ.h"
#include "nativeRenderer_ogles2.h"
#include "VirtKB.h"
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
																						jintArray keyPresses);
	//JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_toggleMouseJoystick(JNIEnv * env, jobject obj);

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

struct QuickKey;
typedef void (*OnKeyEvent)(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);

struct KeyCallback
{
	uint32_t uParam1;
	OnKeyEvent onKeyEvent;
};

struct QuickKey
{
	int x1, y1;
	int x2, y2;

	float rx1, ry1;
	float rx2, ry2;

	float tx1, ty1;
	float tx2, ty2;

	const VirtKeyDef *pKeyDef;
};

#define DEFAULT_INPUT_FLAGS (FLAG_PERSIST|FLAG_JOY|FLAG_SCREEN|FLAG_STKEY|FLAG_STFNKEY|FLAG_MAIN)

static volatile bool	s_InputReady = false;
static volatile bool	s_InputEnabled = false;

static bool			s_showKeyboard = false;
static bool			s_keyboardZoomMode = false;
static bool			s_screenZoomMode = false;
static unsigned int s_curInputFlags = DEFAULT_INPUT_FLAGS;
static unsigned int s_prevInputFlags = DEFAULT_INPUT_FLAGS;
static int			s_joyID = 1;
static bool			s_joyMapToArrowKeys = false;
static int			s_prevZoomPanCount = 0;
static bool			s_waitInputCleared = false;
static float		s_joystickSize = 1;
static bool			s_vkbObsessionKeys = false;
static bool			s_vkbExtraKeys = false;

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

static const int	MaxTouches = 3;
static bool			s_curtouched[2][MaxTouches] = { false };
static float		s_curtouchX[2][MaxTouches] = {0};
static float		s_curtouchY[2][MaxTouches] = {0};

static BitFlags*	s_curButtons[2] = {0};
static BitFlags*	s_changedButtons = 0;

static const int	MaxButtonDown = 10;
static int			s_buttonDown[2][MaxButtonDown];
static int			s_numButtonDown[2] = {0};

static int			s_curIndex = 0;

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

static bool			s_hideAll = false;
static bool			s_showJoystickOnly = false;

static KeyCallback*	s_keyCallbacks = 0;

static BitFlags*	s_jKeyPresses = new BitFlags(VKB_KEY_NumOf);

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
static void VirtKB_addVkbPress(int vkbKeyID);

static const VirtKeyDef *VirtKB_VkbHitTest(float x, float y);

static void VirtKB_updateInput();
static void VirtKB_updateMouse();
static void VirtKB_clearMousePresses();

static void VirtKB_onRender();
static void VirtKB_RenderVerts(Shader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numVerts);
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
static void VirtKB_MJToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down);
static void VirtKB_MouseLB(bool down);
static void VirtKB_MouseRB(bool down);

void VirtKB_RefreshKB()
{
	s_recreateQuickKeys = true;
}

int VirtKB_GetJoystickPort()
{
	return s_joyID;
}

void VirtKB_SetJoystickPort(int port)
{
	if (port < 0 || port >= JOYSTICK_COUNT)
	{
		return;
	}

	// clear cur joystick
	Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_LEFT);
	Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_RIGHT);
	Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_UP);
	Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_DOWN);
	Joy_CustomUp(s_joyID, ATARIJOY_BITMASK_FIRE);

	s_joyID = port;
}

void VirtKB_MapJoysticksToArrowKeys(bool map)
{
	s_joyMapToArrowKeys = map;
	s_recreateQuickKeys = true;
}

void VirtKB_SetMouseEmuDirect()
{
	if (s_showKeyboard || s_screenZoomMode)	{ s_prevInputFlags &= ~(FLAG_MOUSEBUTTON); }
	else									{ s_curInputFlags &= ~(FLAG_MOUSEBUTTON); }

	s_recreateQuickKeys = true;

	VirtKB_clearMousePresses();
}

void VirtKB_SetMouseEmuButtons()
{
	if (s_showKeyboard || s_screenZoomMode)	{ s_prevInputFlags |= (FLAG_MOUSEBUTTON); }
	else									{ s_curInputFlags |= (FLAG_MOUSEBUTTON); }

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
		jintArray keyPresses)
{
	if (!(s_InputReady&s_InputEnabled)) return;

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

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];

	curtouched[0] = t0;	curtouchX[0] = tx0;	curtouchY[0] = ty0;
	curtouched[1] = t1;	curtouchX[1] = tx1;	curtouchY[1] = ty1;
	curtouched[2] = t2;	curtouchX[2] = tx2;	curtouchY[2] = ty2;

	VirtKB_updateInput();

	if (s_recreateQuickKeys)
	{
		VirtKB_CreateQuickKeys();
		s_recreateQuickKeys = false;
	}

	s_curIndex = 1 - s_curIndex;
}

/*
JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_toggleMouseJoystick(JNIEnv * env, jobject obj)
{
	if (!(s_InputReady&s_InputEnabled)) return;

	VirtKB_MJToggle(true);
}
*/

void VirtKB_EnableInput(bool enable)
{
	s_InputEnabled = enable;
	s_recreateQuickKeys = true;
}

int VirtKB_OnSurfaceChanged(int width, int height)
{
	VirtKB_CleanUp();
	VirtKB_Create();

	s_InputReady = true;

	return 0;
}

void VirtKB_CleanUp()
{
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
		for (int i = 0; i < MaxTouches; ++i)
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
	VirtKB_InitCallbacks();
	s_jKeyPresses->clearAll();

	Joy_SetCustomEmu(1);

	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < MaxTouches; ++i)
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
	VirtKB_CreateQuickKeys();

	if (!s_VkbZoomInited)
	{
		if (VirtkKB_KeyboardSetPreset(s_curKeyboardZoomPreset))
		{
			s_VkbZoomInited = true;
		}
	}

	VirtKB_UpdateVkbVerts();
	VirtKB_SetupShader();

	if (s_curScreenZoomPreset == -1)
	{
		s_curScreenZoomPreset = ScreenZoom_Fit;
		Renderer_setZoomPreset(s_curScreenZoomPreset);
	}
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
			}
		}
		else if (vk->flags & (FLAG_STKEY|FLAG_STFNKEY))
		{
			kcb->onKeyEvent = _onSTKeyEventCallback;
		}
		else if (vk->flags & (FLAG_JOY))
		{
			kcb->onKeyEvent = _onJoystickEventCallback;

			switch (vk->id)
			{
				case VKB_KEY_JOYLEFT:	kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_LEFT, ATARIJOY_BITMASK_RIGHT); break;
				case VKB_KEY_JOYRIGHT:	kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_RIGHT, ATARIJOY_BITMASK_LEFT); break;
				case VKB_KEY_JOYUP:		kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_UP, ATARIJOY_BITMASK_DOWN); break;
				case VKB_KEY_JOYDOWN:	kcb->uParam1 = MAKE_DWORD(ATARIJOY_BITMASK_DOWN, ATARIJOY_BITMASK_UP); break;
				case VKB_KEY_JOYFIRE:	kcb->uParam1 = ATARIJOY_BITMASK_FIRE; break;
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
		return;
	}

	int scrwidth = getScreenWidth();
	int scrheight = getScreenHeight();

	int joyAreaMinY = 0;
	int joyAreaMaxX = scrwidth;

	float sscale = (float)scrwidth/1024.0f;

	// top left keys
	if (!s_showJoystickOnly)
	{
		int keyOffsetX = (int)ceilf(10*sscale);
		int keyOffsetY = (int)ceilf(10*sscale);
		int keyBtnSize = (int)ceilf(60*sscale);
		int keyMarginY = (int)ceilf(2*sscale);

		//bool isFullScreen = Renderer_isFullScreenStretch();
		int vkbKeys[] = {	VKB_KEY_KEYBOARDTOGGLE, VKB_KEY_SCREENZOOM, VKB_KEY_SCREENPRESETS,
							VKB_KEY_KEYBOARDZOOM, VKB_KEY_KEYBOARDPRESETS, VKB_KEY_MOUSETOGGLE, VKB_KEY_JOYTOGGLE};
		int numKeys = sizeof(vkbKeys)/sizeof(int);

		int curKeyY = keyOffsetY;
		for (int i = 0; i < numKeys; ++i)
		{
			//if (isFullScreen && vkbKeys[i] == VKB_KEY_SCREENZOOM) { continue; }

			if (s_showKeyboard && vkbKeys[i]==VKB_KEY_KEYBOARDTOGGLE) 	{ vkbKeys[i] = VKB_KEY_KEYBOARDTOGGLE_SEL; }
			if (s_keyboardZoomMode && vkbKeys[i]==VKB_KEY_KEYBOARDZOOM)	{ vkbKeys[i] = VKB_KEY_KEYBOARDZOOM_SEL; }
			if (s_screenZoomMode && vkbKeys[i]==VKB_KEY_SCREENZOOM) 	{ vkbKeys[i] = VKB_KEY_SCREENZOOM_SEL; }

			if (!addQuickKey(0, curKeyY, keyOffsetX+keyBtnSize, curKeyY+keyBtnSize,
						keyOffsetX, curKeyY, keyOffsetX+keyBtnSize, curKeyY+keyBtnSize,
						2, 2, -2, -2, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyY = s_QuickKeys[s_numQuickKeys-1].y2 + keyMarginY;
		}

		joyAreaMinY = curKeyY;
	}

	// top right keys
	if (!s_showJoystickOnly)
	{
		int keyOffsetX = (int)ceilf(10*sscale);
		int keyOffsetY = (int)ceilf(10*sscale);
		int keyBtnSize = (int)ceilf(60*sscale);
		int keyMarginY = (int)ceilf(2*sscale);

		int vkbKeysNormal[] = {VKB_KEY_NORMALSPEED, VKB_KEY_Y, VKB_KEY_N, VKB_KEY_1, VKB_KEY_2};
		int vkbKeysExtra[] = {VKB_KEY_NORMALSPEED, VKB_KEY_Y, VKB_KEY_N, VKB_KEY_1, VKB_KEY_2, VKB_KEY_RETURN};
		int vkbKeysObsession[] = {VKB_KEY_NORMALSPEED, VKB_KEY_F1, VKB_KEY_F2, VKB_KEY_F3, VKB_KEY_F4};
		int numNormalKeys = sizeof(vkbKeysNormal)/sizeof(int);
		int numExtraKeys = sizeof(vkbKeysExtra)/sizeof(int);
		int numObsessionKeys = sizeof(vkbKeysObsession)/sizeof(int);
		int* vkbKeys = s_vkbObsessionKeys ? vkbKeysObsession : (s_vkbExtraKeys ? vkbKeysExtra : vkbKeysNormal);
		int numKeys = s_vkbObsessionKeys ? numObsessionKeys : (s_vkbExtraKeys ? numExtraKeys : numNormalKeys);

		int curKeyY = keyOffsetY;
		for (int i = 0; i < numKeys; ++i)
		{
			if (i == 0) // speed button hack
			{
				bool turbo = getTurboSpeed()!=0;
				vkbKeys[i] = turbo ? VKB_KEY_TURBOSPEED : VKB_KEY_NORMALSPEED;
			}

			if (!addQuickKey(scrwidth - keyOffsetX-keyBtnSize, curKeyY, scrwidth, curKeyY+keyBtnSize,
						scrwidth-keyOffsetX-keyBtnSize, curKeyY, scrwidth-keyOffsetX, curKeyY+keyBtnSize,
						2, 2, -2, -2, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyY = s_QuickKeys[s_numQuickKeys-1].y2 + keyMarginY;
		}
	}

	// mouse keys (bottom left)
	s_mouseButtonIgnoreQuickKeyIdx[0] = -1;
	s_mouseButtonIgnoreQuickKeyIdx[1] = -1;
	{
		int keyOffsetX = (int)ceilf(30*sscale);
		int keyOffsetY = (int)ceilf(30*sscale);
		int keyBtnSize = (int)ceilf(60*sscale);
		int keyMarginX = (int)ceilf(64*sscale);
		int fireBtnSize = (int)ceilf(100*sscale);

		int vkbKeys[] = {VKB_KEY_MOUSELB, VKB_KEY_MOUSERB};
		int x1Overlap[] = {0, (int)(keyMarginX*0.8f)};
		int x2Overlap[] = {(int)(keyMarginX*0.8f), 0};
		int numKeys = sizeof(vkbKeys)/sizeof(int);

		int curKeyX = keyOffsetX;

		for (int i = 0; i < numKeys; ++i)
		{
			bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
			int btnSize = isFire ? fireBtnSize : keyBtnSize;	// the fire button is a special case (bigger hit area)
			if (!addQuickKey(curKeyX-x1Overlap[i], scrheight-keyOffsetY-btnSize, curKeyX+btnSize+x2Overlap[i], scrheight,
						curKeyX, scrheight-keyOffsetY-btnSize, curKeyX+btnSize, scrheight-keyOffsetY,
						2, 2, -2, -2, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyX = s_QuickKeys[s_numQuickKeys-1].x2 - x2Overlap[i] + keyMarginX;

			if (s_mouseButtonIgnoreQuickKeyIdx[0] == -1) { s_mouseButtonIgnoreQuickKeyIdx[0] = s_numQuickKeys-1; } // HACK
			else if (s_mouseButtonIgnoreQuickKeyIdx[1] == -1) { s_mouseButtonIgnoreQuickKeyIdx[1] = s_numQuickKeys-1; } // HACK
		}
	}

	// bottom right keys
	bool joyMode = (s_curInputFlags & FLAG_JOY) != 0;
	int obsessionButtonSize = 120;
	{
		int keyOffsetX = (int)ceilf(30*sscale);
		int keyOffsetY = (int)ceilf(30*sscale);
		int keyBtnSize = (int)ceilf(60*sscale);
		int keyMarginX = (int)ceilf(2*sscale);
		int fireBtnSize = (int)ceilf((s_vkbObsessionKeys?obsessionButtonSize:100)*sscale);

		int vkbKeysNormal[] = {VKB_KEY_JOYFIRE, VKB_KEY_SPACE, VKB_KEY_LEFTSHIFT, VKB_KEY_ALTERNATE};
		int vkbKeysExtra[] = {VKB_KEY_JOYFIRE, VKB_KEY_SPACE, VKB_KEY_LEFTSHIFT, VKB_KEY_ALTERNATE, VKB_KEY_CONTROL};
		int vkbKeysObsession[] = {VKB_KEY_RIGHTSHIFT_BUTTON, VKB_KEY_DOWNARROW};
		int numNormalKeys = sizeof(vkbKeysNormal)/sizeof(int);
		int numExtraKeys = sizeof(vkbKeysExtra)/sizeof(int);
		int numObsessionKeys = sizeof(vkbKeysObsession)/sizeof(int);
		int* vkbKeys = (joyMode&&s_vkbObsessionKeys) ? vkbKeysObsession : (s_vkbExtraKeys ? vkbKeysExtra : vkbKeysNormal);
		int numKeys = (joyMode&&s_vkbObsessionKeys) ? numObsessionKeys : (s_vkbExtraKeys ? numExtraKeys : numNormalKeys);

		int curKeyX = scrwidth - keyOffsetX;

		for (int i = 0; i < numKeys; ++i)
		{
			bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
			bool isBigButton = isFire || (vkbKeys[i] == VKB_KEY_LEFTSHIFT_BUTTON) || (vkbKeys[i] == VKB_KEY_RIGHTSHIFT_BUTTON);
			int btnSize = isBigButton ? fireBtnSize : keyBtnSize;	// the fire button is a special case (bigger hit area)
			if (!addQuickKey(curKeyX-btnSize, scrheight-keyOffsetY-btnSize, curKeyX, scrheight,
						curKeyX-btnSize, scrheight-keyOffsetY-btnSize, curKeyX, scrheight-keyOffsetY,
						2, 2, -2, -2, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

			curKeyX = s_QuickKeys[s_numQuickKeys-1].x1 - keyMarginX - ((i==0)? keyOffsetX : 0);
		}

		joyAreaMaxX = curKeyX;
	}

	if (joyMode && s_vkbObsessionKeys)
	{
		// bottom left keys
		{
			int keyOffsetX = (int)ceilf(30*sscale);
			int keyOffsetY = (int)ceilf(30*sscale);
			int keyBtnSize = (int)ceilf(60*sscale);
			int keyMarginX = (int)ceilf(2*sscale);
			int fireBtnSize = (int)ceilf(obsessionButtonSize*sscale);

			int vkbKeys[] = {VKB_KEY_LEFTSHIFT_BUTTON, VKB_KEY_SPACE};
			int numKeys = sizeof(vkbKeys)/sizeof(int);

			int curKeyX = keyOffsetX;

			for (int i = 0; i < numKeys; ++i)
			{
				bool isFire = (vkbKeys[i] == VKB_KEY_JOYFIRE);
				bool isBigButton = isFire || (vkbKeys[i] == VKB_KEY_LEFTSHIFT_BUTTON) || (vkbKeys[i] == VKB_KEY_RIGHTSHIFT_BUTTON);
				int btnSize = isBigButton ? fireBtnSize : keyBtnSize;	// the fire button is a special case (bigger hit area)
				if (!addQuickKey(curKeyX, scrheight-keyOffsetY-btnSize, curKeyX+btnSize, scrheight,
							curKeyX, scrheight-keyOffsetY-btnSize, curKeyX+btnSize, scrheight-keyOffsetY,
							2, 2, -2, -2, &g_vkbKeyDefs[vkbKeys[i]])) { continue; }

				curKeyX = s_QuickKeys[s_numQuickKeys-1].x2 + keyMarginX + ((i==0)? keyOffsetX : 0);
			}

			joyAreaMaxX = curKeyX;
		}
	}
	else
	{
		// joystick dir - from bottom left
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
						2, 2, -2, -2, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_LEFTARROW:VKB_KEY_JOYLEFT])) { }

			// right
			if (!addQuickKey(keyOffsetX+keyBtnSize*2, joyAreaMinY, joyAreaMaxX, scrheight,
						keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY-keyBtnSize*2, keyOffsetX+keyBtnSize*3, scrheight-keyOffsetY-keyBtnSize,
						2, 2, -2, -2, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_RIGHTARROW:VKB_KEY_JOYRIGHT])) { }

			// up
			if (!addQuickKey(0, joyAreaMinY, joyAreaMaxX, scrheight-keyOffsetY-keyBtnSize*2,
						keyOffsetX+keyBtnSize, scrheight-keyOffsetY-keyBtnSize*3, keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY-keyBtnSize*2,
						2, 2, -2, -2, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_UPARROW:VKB_KEY_JOYUP])) { }

			// down
			if (!addQuickKey(0, scrheight-keyOffsetY-keyBtnSize, joyAreaMaxX, scrheight,
						keyOffsetX+keyBtnSize, scrheight-keyOffsetY-keyBtnSize, keyOffsetX+keyBtnSize*2, scrheight-keyOffsetY,
						2, 2, -2, -2, &g_vkbKeyDefs[s_joyMapToArrowKeys?VKB_KEY_DOWNARROW:VKB_KEY_JOYDOWN])) { }
		}
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

static void VirtKB_RenderVerts(Shader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numVerts)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glUseProgram(pShader->shaderProgram);

	glVertexAttribPointer(pShader->paramHandles[ShaderParam_Pos], 3, GL_FLOAT, GL_FALSE, vstride, v);
	glVertexAttribPointer(pShader->paramHandles[ShaderParam_Color], 4, GL_FLOAT, GL_FALSE, vstride, v+3);
	glVertexAttribPointer(pShader->paramHandles[ShaderParam_TexCoord], 2, GL_FLOAT, GL_FALSE, vstride, v+7);

	glEnableVertexAttribArray(pShader->paramHandles[ShaderParam_Pos]);
	glEnableVertexAttribArray(pShader->paramHandles[ShaderParam_Color]);
	glEnableVertexAttribArray(pShader->paramHandles[ShaderParam_TexCoord]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(pShader->paramHandles[ShaderParam_Sampler], 0);

	glDrawElements(GL_TRIANGLES, 6*numVerts, GL_UNSIGNED_SHORT, ind);
}

static void VirtKB_onRender()
{
	Shader *pShader = Renderer_getSimpleShader();

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

		float a = s_QuickKeyAlpha;
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

	float a = s_QuickKeyAlpha;
	float r = down ? 0.1f : s_QuickKeyLum;
	float g = down ? 0.1f : s_QuickKeyLum;
	float b = s_QuickKeyLum;

	int c = 3;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a; c += 5;
	v[c++] = r; v[c++] = g; v[c++] = b; v[c++] = a;
}

void VirtKB_updateInput()
{
	int prevIndex = 1 - s_curIndex;

	bool *prevtouched = s_curtouched[prevIndex];
	float *prevtouchX = s_curtouchX[prevIndex];
	float *prevtouchY = s_curtouchY[prevIndex];

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];

	BitFlags *prevButtons = s_curButtons[prevIndex];
	BitFlags *curButtons = s_curButtons[s_curIndex];

	int *prevButtonDownSet = s_buttonDown[prevIndex];
	int numPrevButtonDown = s_numButtonDown[prevIndex];
	int *curButtonDownSet = s_buttonDown[s_curIndex];
	int numCurButtonDown = 0;

	curButtons->clearAll();
	//s_recreateQuickKeys = false;
	VirtKB_resetVkbPresses();

	// retrieve current touches
	for (int i = 0; i < MaxTouches; ++i)
	{
		if (curtouched[i])
		{
			// quick keys
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
					}
				}
			}
		}
	}

	// vkb
	if (s_keyboardZoomMode || s_screenZoomMode)
	{
		if (numCurButtonDown == 0)
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
			}
			else if (zoomPanCount > 0 && s_prevZoomPanCount > 0)
			{
				// pan
				int finger = curtouched[0] ? 0 : 1;
				float px1 = curtouchX[finger] - prevtouchX[finger], py1 = curtouchY[finger] - prevtouchY[finger];

				if (s_keyboardZoomMode)		VirtKB_PanVKB(px1, py1);
				else if (s_screenZoomMode)	Renderer_panEmuScreen(px1, py1);
			}

			s_prevZoomPanCount = zoomPanCount;
		}
	}
	else if (s_showKeyboard)
	{
		for (int i = 0; i < MaxTouches; ++i)
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
					VirtKB_addVkbPress(vk->id);
					curButtons->setBit(vk->id);
					curButtonDownSet[numCurButtonDown] = MAKE_BUTTON_DOWN_SET(vk->id, INVALID_QUICK_KEY_ID);
					++numCurButtonDown;
				}
			}
		}
	}

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
	if (((prevInputFlags&s_curInputFlags)&FLAG_MOUSE))
	{
		VirtKB_updateMouse();
	}

	//if (s_recreateQuickKeys)
	//{
	//	VirtKB_CreateQuickKeys();
	//}
}

static int s_mouseFinger = -1;
static int s_mouseMoved = 0;
static float s_prevMouseX = 0;
static float s_prevMouseY = 0;
static float s_mouseDx = 0;
static float s_mouseDy = 0;

static int s_prevFingerCount = 0;
static int s_mousePresses = 0;
static float s_mousePressTime = 0;

static bool s_mouseQuickPress = false;

static void VirtKB_updateMouse()
{
	int prevIndex = 1 - s_curIndex;

	bool *prevtouched = s_curtouched[prevIndex];
	float *prevtouchX = s_curtouchX[prevIndex];
	float *prevtouchY = s_curtouchY[prevIndex];

	bool *curtouched = s_curtouched[s_curIndex];
	float *curtouchX = s_curtouchX[s_curIndex];
	float *curtouchY = s_curtouchY[s_curIndex];


	float dt = 1.f/60.f; // TODO: calculate real value
	int fingers = 0;



	int prevMousePresses = s_mousePresses;
	int curMousePresses = s_mousePresses;


	int quickPressed = false;
	if (s_mouseQuickPress)
	{
		quickPressed = true;
		if ((s_curInputFlags & (FLAG_MOUSEBUTTON)) == 0)
		{
			if (prevMousePresses & 1) { Keyboard.bLButtonDown &= ~BUTTON_MOUSE; }
			if (prevMousePresses & 2) { Keyboard.bRButtonDown &= ~BUTTON_MOUSE; }
		}

//Debug_Printf("Mouse Released: %d", prevMousePresses);
		s_mousePresses = 0;
	}

	for (int i = 0; i < MaxTouches; ++i)
	{
		if (curtouched[i]) { ++fingers; }
	}

	if (s_mouseFinger < 0)
	{
		bool isButtonMode = (s_curInputFlags & FLAG_MOUSEBUTTON) != 0;

		for (int i = 0; i < MaxTouches; ++i)
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
				s_mouseMoved = 0;

				s_mousePresses = 0;
				s_mousePressTime = 0;
			}
		}
	}
	else
	{
		if (curtouched[s_mouseFinger])
		{
			if (!s_mouseMoved && !s_mouseQuickPress)
			{
				s_mousePressTime += 1.0f/60.0f;//dt;
				if (s_mousePressTime > 8.0f/60.0f)
				{
					s_mouseMoved = 1;
					curMousePresses = fingers;
				}
			}
		}
		else
		{
//Debug_Printf("MouseFinger cleared: %d", s_mouseFinger);
			s_mouseFinger = -1;

			s_mousePressTime = 0;
			curMousePresses = s_mouseMoved ? 0: s_prevFingerCount;
			s_mouseQuickPress = s_mouseMoved ? false : true;

			s_mouseDx = 0;
			s_mouseDy = 0;
		}
	}

	float emuScrZoomX = Renderer_getEmuScreenZoomX();
	float emuScrZoomY = Renderer_getEmuScreenZoomY();

	if (s_mouseFinger >= 0)
	{
		s_mouseDx += curtouchX[s_mouseFinger] - s_prevMouseX;
		s_mouseDy += curtouchY[s_mouseFinger] - s_prevMouseY;

		s_prevMouseX = curtouchX[s_mouseFinger];
		s_prevMouseY = curtouchY[s_mouseFinger];

		if (s_mouseDx != 0.0f || s_mouseDy != 0.0f)
		{
			s_mouseMoved = 1;

			float sX = (STRes == ST_LOW_RES) ? 0.5f : 1;
			float sY = 1;//(STRes == ST_MEDIUM_RES || STRes == ST_LOW_RES) ? 0.5f : 1;
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
	}

	if (prevMousePresses != curMousePresses)
	{
if (s_mouseQuickPress)
{
//Debug_Printf("Quick Pressed");
}
		if ((s_curInputFlags & (FLAG_MOUSEBUTTON)) == 0)
		{
			if (prevMousePresses & 1) { Keyboard.bLButtonDown &= ~BUTTON_MOUSE; }
			if (prevMousePresses & 2) { Keyboard.bRButtonDown &= ~BUTTON_MOUSE; }

			if (curMousePresses != 0)
			{
//Debug_Printf("Mouse Pressed: %d", curMousePresses);
				if (curMousePresses & 1) { Keyboard.bLButtonDown |= BUTTON_MOUSE; }
				if (curMousePresses & 2) { Keyboard.bRButtonDown |= BUTTON_MOUSE; }
			}
			else
			{
//Debug_Printf("Mouse Released: %d", prevMousePresses);
			}
		}

		s_mousePresses = curMousePresses;
	}

	if (quickPressed)
	{
		s_mouseQuickPress = false;
	}

	s_prevFingerCount = fingers;
}

static void VirtKB_resetVkbPresses()
{
	s_VkbCurNumPresses = 0;
}

static void VirtKB_addVkbPress_Rect(const VirtKeyDef *k);
static void VirtKB_addVkbPress_Poly(const VirtKeyDef *k);

static void VirtKB_addVkbPress(int vkbKeyID)
{
	const VirtKeyDef *k = &g_vkbKeyDefs[vkbKeyID];
	if (k->flags & FLAG_POLY)
	{
		VirtKB_addVkbPress_Poly(k);
	}
	else
	{
		VirtKB_addVkbPress_Rect(k);
	}
}

static void VirtKB_addVkbPress_Rect(const VirtKeyDef *k)
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
		float g = 0.1f;
		float b = 1.0f;

		VirtKB_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, r, g, b, a);

		++s_VkbCurNumPresses;
	}
}

static void VirtKB_addVkbPress_Poly(const VirtKeyDef *k)
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
		float g = 0.1f;
		float b = 1.0f;

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

		float a = s_VkbAlpha;
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
	extern const int g_vkbRowSearchSize;
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

static void VirtKB_ToggleKeyboard(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		s_showKeyboard = !s_showKeyboard;
		if (s_showKeyboard)
		{
			s_prevInputFlags = s_curInputFlags;
			s_curInputFlags = FLAG_PERSIST|FLAG_VKB;
		}
		else
		{
			s_curInputFlags = s_prevInputFlags;
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

		VirtKB_UpdateVkbVerts();
		return true;
	}

	return false;
}

static void VirtKB_MJToggle(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		if (s_showKeyboard || s_screenZoomMode)	{ s_prevInputFlags ^= (FLAG_MOUSE|FLAG_JOY); }
		else									{ s_curInputFlags ^= (FLAG_MOUSE|FLAG_JOY); }

		s_recreateQuickKeys = true;

		VirtKB_clearMousePresses();
	}
}

static void VirtKB_clearMousePresses()
{
	if (s_mousePresses > 0)
	{
		Debug_Printf("Mouse Released: %d", s_mousePresses);

		if (s_mousePresses & 1) { Keyboard.bLButtonDown &= ~BUTTON_MOUSE; }
		if (s_mousePresses & 2) { Keyboard.bRButtonDown &= ~BUTTON_MOUSE; }

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

	s_mouseQuickPress = false;
}

static void VirtKB_MouseLB(bool down)
{
	if (down)	{ Keyboard.bLButtonDown |= BUTTON_MOUSE; }
	else		{ Keyboard.bLButtonDown &= ~BUTTON_MOUSE; }
}

static void VirtKB_MouseRB(bool down)
{
	if (down)	{ Keyboard.bRButtonDown |= BUTTON_MOUSE; }
	else		{ Keyboard.bRButtonDown &= ~BUTTON_MOUSE; }
}

void VirtKB_SetControlAlpha(float alpha)
{
	s_QuickKeyAlpha = alpha;
	s_VkbAlpha = alpha;
	s_recreateQuickKeys = true;

	VirtKB_UpdateVkbVerts();
}

void VirtKB_SetJoystickSize(float size)
{
	s_joystickSize = size;
	s_recreateQuickKeys = true;
}

void VirtKB_setExtraKeys(bool set)
{
	s_vkbExtraKeys = set;
	s_recreateQuickKeys = true;
}

void VirtKB_setObsessionKeys(bool set)
{
	g_doubleBuffer = set ? 0 : 1;

	s_vkbObsessionKeys = set;
	s_recreateQuickKeys = true;
}

void VirtKB_setHideAll(bool set)
{
	s_hideAll = set;
	s_recreateQuickKeys = true;
}

static void VirtKB_ToggleShowUI(const VirtKeyDef *keyDef, uint32_t uParam1, bool down)
{
	if (down)
	{
		s_hideAll = !s_hideAll;
		s_recreateQuickKeys = true;
	}
}

void VirtKB_setJoystickOnly(bool set)
{
	s_showJoystickOnly = set;
	s_recreateQuickKeys = true;
}
