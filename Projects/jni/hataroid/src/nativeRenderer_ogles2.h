#ifndef __NATIVERENDERER_OGLES2_H__
#define __NATIVERENDERER_OGLES2_H__

#include <GLES2/gl2.h>

typedef void (*RenderCallback)(void);

enum
{
	ShaderParam_Pos		= 0,
	ShaderParam_TexCoord,
	ShaderParam_Color,
	ShaderParam_Sampler,

	ShaderParam_Max
};

struct Shader
{
	GLuint shaderProgram;
	GLuint paramHandles[ShaderParam_Max];
};

extern Shader *Renderer_getSimpleShader();
extern GLuint getWhiteTexture();

extern void cleanupGraphics();
extern bool setupGraphics(int w, int h);
extern void renderFrame();

extern bool Renderer_addRenderCallback(RenderCallback pCallback);

extern int getScreenWidth();
extern int getScreenHeight();

extern float Renderer_getEmuScreenZoom();
extern void Renderer_zoomEmuScreen(float absChange);
extern void Renderer_panEmuScreen(float absX, float absY);
extern void Renderer_setFilterEmuScreeen(bool filter);

extern unsigned int roundUpPower2(unsigned int v);

#endif //__NATIVERENDERER_OGLES2_H__
