#ifndef __NATIVERENDERER_OGLES2_H__
#define __NATIVERENDERER_OGLES2_H__

#include <GLES2/gl2.h>
#include <jni.h>

class RTShader;

typedef void (*RenderCallback)(void);

extern bool Renderer_setScreenShader(const char *shaderName);

extern RTShader* Renderer_getColorModShader();
extern GLuint getWhiteTexture();

extern void cleanupGraphics();
extern bool setupGraphics(int w, int h, JNIEnv * env);
extern void renderFrame(JNIEnv* env);

extern bool Renderer_addRenderCallback(RenderCallback pCallback);

extern int getScreenWidth();
extern int getScreenHeight();

extern float Renderer_getEmuScreenZoomX();
extern float Renderer_getEmuScreenZoomY();
extern void Renderer_zoomEmuScreen(float absChange);
extern void Renderer_panEmuScreen(float absX, float absY);
extern void Renderer_setFilterEmuScreeen(bool filter);
extern void Renderer_setFullScreenStretch(bool fs);
extern bool Renderer_isFullScreenStretch();
extern void Renderer_refreshDispParams();

extern void Renderer_setZoomPreset(int preset);

extern float Renderer_getScreenZoomXRaw();
extern float Renderer_getScreenZoomYRaw();
extern float Renderer_getScreenPanXRaw();
extern float Renderer_getScreenPanYRaw();
extern void Renderer_setScreenPanZoomRaw(float scrZoomX, float scrZoomY, float scrPanX, float scrPanY);

extern unsigned int roundUpPower2(unsigned int v);

#endif //__NATIVERENDERER_OGLES2_H__
