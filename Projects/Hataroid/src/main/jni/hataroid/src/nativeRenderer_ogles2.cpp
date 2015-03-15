#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL.h>

#include "hataroid.h"
#include "nativeRenderer_ogles2.h"

#include "VirtKBDefs.h"
#include "VirtKB.h"

extern "C"
{
	extern SDL_mutex* g_videoTex_mutex;
	extern int g_videoTex_width;
	extern int g_videoTex_height;
	extern void *g_videoTex_pixels;
	extern int g_surface_width;
	extern int g_surface_height;
	extern int g_videoModeChanged;
	extern int g_videoTex_Bpp;
	extern volatile int g_videoFrameReady;
};

static const char gTextureVertexShader[] =
	"attribute vec4 a_position;   \n"
	"attribute vec2 a_texCoord;   \n"
	"varying vec2 v_texCoord;     \n"
	"void main()                  \n"
	"{                            \n"
	"	gl_Position = a_position; \n"
	"	v_texCoord = a_texCoord;  \n"
	"}                            \n";

static const char gTextureFragmentShader[] =
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"	gl_FragColor = texture2D(s_texture, v_texCoord); \n"
	"}													 \n";

static const char gSimpleVertexShader[] =
	"attribute vec4 a_position;   \n"
	"attribute vec4 a_color;      \n"
	"attribute vec2 a_texCoord;   \n"
	"varying vec4 v_color;        \n"
	"varying vec2 v_texCoord;     \n"
	"void main()                  \n"
	"{                            \n"
	"	gl_Position = a_position; \n"
	"	v_texCoord = a_texCoord;  \n"
	"   v_color = a_color;        \n"
	"}                            \n";

static const char gSimpleFragmentShader[] =
	"precision mediump float;                            \n"
    "varying vec4 v_color;                               \n"
	"varying vec2 v_texCoord;                            \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"	gl_FragColor = v_color * texture2D(s_texture, v_texCoord); \n"
	"}													 \n";

static Shader s_vertTexShader;
static Shader s_simpleShader;

static GLuint gTextureID = 0;
static GLuint gWhiteTexID = 0;

static int gScrWidth = 0;
static int gScrHeight = 0;

static int sTexWidth = 0;
static int sTexHeight = 0;
static GLenum sTexFilter = GL_LINEAR;

static float minZoom = 0.3f;
static float curZoomX = 1.0f;
static float curZoomY = 1.0f;
static float curPanX = 0;
static float curPanY = 0;
static bool fullScreen = false;

static bool dispParamsChanged = true;
static int _delayedPreset = -1;

GLfloat emuScreenVerts[] =
{
	-0.75f, 1.0f, 0.0f, // Position 0
	0.0f, 0.0f,			// TexCoord 0

	-.75f, -1.0f, 0.0f, // Position 1
	0.0f, 1.0f,			// TexCoord 1

	0.75f, -1.0f, 0.0f, // Position 2
	1.0, 1.0f,			// TexCoord 2

	0.75f, 1.0f, 0.0f,	// Position 3
	1.0f, 0.0f			// TexCoord 3
};
GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
GLsizei stride = 5 * sizeof(GLfloat); // 3 for position, 2 for texture

int getScreenWidth()		{ return gScrWidth; }
int getScreenHeight()		{ return gScrHeight; }
Shader *Renderer_getSimpleShader()	{ return &s_simpleShader; }
GLuint getWhiteTexture()	{ return gWhiteTexID; }

static void updateDispParams(int scrWidth, int scrHeight, int texWidth, int texHeight);

static const int MaxRenderCallbacks = 20;
static RenderCallback s_RenderCallbacks[MaxRenderCallbacks];
static int s_numRenderCalbacks = 0;

static void printGLString(const char *name, GLenum s)
{
	const char *v = (const char *) glGetString(s);
	Debug_Printf("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
    	Debug_Printf("after %s() glError (0x%x)\n", op, error);
    }
}

void unloadShader(Shader *shader)
{
	if (shader->shaderProgram != 0)
	{
		glDeleteProgram(shader->shaderProgram);
	}
	memset(shader, 0, sizeof(Shader));
}

GLuint loadShaderSource(GLenum shaderType, const char* pSource)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader)
	{
		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen)
			{
				char* buf = (char*) malloc(infoLen);
				if (buf)
				{
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					Debug_Printf("Could not compile shader %d:\n%s\n", shaderType, buf);
					free(buf);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource)
{
	GLuint vertexShader = loadShaderSource(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader)
	{
		return 0;
	}

	GLuint pixelShader = loadShaderSource(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader)
	{
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program)
	{
		glAttachShader(program, vertexShader);
		checkGlError("glAttachShaderV");
		glAttachShader(program, pixelShader);
		checkGlError("glAttachShaderP");
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			GLint bufLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength)
			{
				char* buf = (char*) malloc(bufLength);
				if (buf)
				{
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					Debug_Printf("Could not link program:\n%s\n", buf);
					free(buf);
				}
			}
			glDeleteProgram(program);
			program = 0;
		}
	}
	return program;
}

bool createShader(Shader *dst, const char* pVertexSource, const char* pFragmentSource, const char* attribNames[ShaderParam_Max])
{
	memset(dst, 0, sizeof(Shader));

	dst->shaderProgram = createProgram(pVertexSource, pFragmentSource);
	if (!dst->shaderProgram)
	{
		Debug_Printf("Could not create program.");
		return false;
	}

	for (int i = 0; i < ShaderParam_Max; ++i)
	{
		if (attribNames[i])
		{
			dst->paramHandles[i] = glGetAttribLocation(dst->shaderProgram, attribNames[i]);
			checkGlError("glGetAttribLocation");
			Debug_Printf("glGetAttribLocation(\"%s\") = %d\n", attribNames[i], dst->paramHandles[i]);
		}
	}

	return true;
}

bool Renderer_addRenderCallback(RenderCallback pCallback)
{
	if (s_numRenderCalbacks < MaxRenderCallbacks)
	{
		s_RenderCallbacks[s_numRenderCalbacks] = pCallback;
		++s_numRenderCalbacks;
		return true;
	}
	return false;
}

void deleteWhiteTex2D()
{
	if (gWhiteTexID != 0)
	{
	    glDeleteTextures(1, &gWhiteTexID);
	    gWhiteTexID = 0;
	}
}

void allocWhiteTex2D()
{
	deleteWhiteTex2D();

	glGenTextures(1, &gWhiteTexID);

	// Bind the texture unit
	glActiveTexture(GL_TEXTURE0);
	checkGlError("glActiveTexture");

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, gWhiteTexID);
	checkGlError("glBindTexture");

	int w = 2, h = 2;
	GLuint *whitePixels = (GLuint*)memalign(4, w*h*sizeof(GLuint));
	for (int i = 0; i < w*h; ++i)
	{
		whitePixels[i] = 0xffffffff;
	}

	// Alloc the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixels);
	checkGlError("glTexSubImage2D");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	free(whitePixels);
}

void Renderer_setFilterEmuScreeen(bool filter)
{
	sTexFilter = filter ? GL_LINEAR : GL_NEAREST;
	if (gTextureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, gTextureID);

		// Set the filtering mode
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sTexFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sTexFilter);
	}
}

void allocVideoTex2D(int scrWidth, int scrHeight, int texWidth, int texHeight, GLenum format, GLenum formatDataType)
{
	if (gTextureID != 0)
	{
	    glDeleteTextures(1, &gTextureID);
	    gTextureID = 0;
	}

	glGenTextures(1, &gTextureID);

	// Bind the texture unit
	glActiveTexture(GL_TEXTURE0);
	checkGlError("glActiveTexture");

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, gTextureID);
	checkGlError("glBindTexture");

    // Alloc the texture
	glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, formatDataType, NULL);
	checkGlError("glTexSubImage2D");

	sTexWidth = texWidth;
	sTexHeight = texHeight;

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sTexFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sTexFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void updateVideoTex2D(GLuint textureid, GLubyte * pixels, int scrWidth, int scrHeight, int width, int height, GLenum format, GLenum formatDataType)
{
	// Bind the texture unit
	glActiveTexture(GL_TEXTURE0);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, textureid);

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sTexFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sTexFilter);

	// Load the texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, scrHeight, format, formatDataType, pixels);
}

void cleanupGraphics()
{
	unloadShader(&s_vertTexShader);
	unloadShader(&s_simpleShader);

	if (gTextureID != 0)
	{
	    glDeleteTextures(1, &gTextureID);
	    gTextureID = 0;
	}

	deleteWhiteTex2D();

	gScrWidth = 0;
	gScrHeight = 0;

	s_numRenderCalbacks = 0;
}

char *_loadShaderFile(const char *path, JNIEnv *env)
{
	int len = 0;
	int id = 0;
	char *buf = (char*)hataroid_getAssetDataRef(env, &path[8], 0, &len, &id);
	if (buf)
	{
		char *bufZ = new char [len+1];
		memcpy(bufZ, buf, len);
		bufZ[len] = 0;

		hataroid_releaseAssetDataRef(id);

		return bufZ;
	}
	return 0;
}

bool setupGraphics(int w, int h, JNIEnv * env)
{
	cleanupGraphics();

	gScrWidth = w;
	gScrHeight = h;

	printGLString("Version", GL_VERSION);
	printGLString("Vendor", GL_VENDOR);
	printGLString("Renderer", GL_RENDERER);
	printGLString("Extensions", GL_EXTENSIONS);
	
	Debug_Printf("setupGraphics(%d, %d)", w, h);

	{
		bool res = true;

		/*
		char *vsh = _loadShaderFile("asset://shaders/simple.vsh", env);
		char *fsh = _loadShaderFile("asset://shaders/simple.fsh", env);
		if (vsh != 0 && fsh != 0)
		{
			Debug_Printf("loading simple shaders from assets");
			const char *shaderParamNames[ShaderParam_Max] = { "a_position", "a_texCoord", 0, "s_texture" };
			if (!createShader(&s_vertTexShader, vsh, fsh, shaderParamNames))
			{
				Debug_Printf("Could not create shader.");
				res = false;
			}
		}
		else
		*/
		{
			// safe fallback
			Debug_Printf("loading simple shaders in memory");
			const char *shaderParamNames[ShaderParam_Max] = { "a_position", "a_texCoord", 0, "s_texture" };
			if (!createShader(&s_vertTexShader, gTextureVertexShader, gTextureFragmentShader, shaderParamNames))
			{
				Debug_Printf("Could not create shader.");
				res = false;
			}
		}

		//if (vsh != 0) { delete [] vsh; }
		//if (fsh != 0) { delete [] fsh; }

		if (!res)
		{
			return res;
		}
	}
	{
		Debug_Printf("loading ui shaders in memory");
		const char *shaderParamNames[ShaderParam_Max] = { "a_position", "a_texCoord", "a_color", "a_texCoord" };
		if (!createShader(&s_simpleShader, gSimpleVertexShader, gSimpleFragmentShader, shaderParamNames))
		{
			Debug_Printf("Could not create shader.");
			return false;
		}
	}

    glViewport(0, 0, w, h);
	checkGlError("glViewport");

	glClearColor(0, 0, 0, 1.0f);
	checkGlError("glClearColor");
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	checkGlError("glClear");

	dispParamsChanged = true;

	allocWhiteTex2D();

	return true;
}

float Renderer_getScreenZoomXRaw() { return curZoomX; }
float Renderer_getScreenZoomYRaw() { return curZoomY; }
float Renderer_getScreenPanXRaw() { return curPanX; }
float Renderer_getScreenPanYRaw() { return curPanY; }

void Renderer_setScreenPanZoomRaw(float scrZoomX, float scrZoomY, float scrPanX, float scrPanY)
{
	curZoomX = scrZoomX;
	curZoomY = scrZoomY;
	curPanX = scrPanX;
	curPanY = scrPanY;

	if (curZoomX < minZoom)
	{
		curZoomX = minZoom;
	}
	if (curZoomY < minZoom)
	{
		curZoomY = minZoom;
	}

	dispParamsChanged = true;
	_delayedPreset = -1;
}

float Renderer_getEmuScreenZoomX()
{
	int texW = g_videoTex_width;
	int emuScrW = g_surface_width;

	GLfloat texWRatio = (float)emuScrW / (float)texW;
	GLfloat scrScale = gScrWidth/1024.0f;

	return (scrScale*curZoomX) / texWRatio;
//	return curZoomX;
}

float Renderer_getEmuScreenZoomY()
{
	int texH = g_videoTex_height;
	int emuScrH = g_surface_height;

	GLfloat texHRatio = (float)emuScrH / (float)texH;
	GLfloat scrScale = gScrHeight/552.0f;

	return (scrScale*curZoomY) / texHRatio;
//	return curZoomY;
}

void Renderer_zoomEmuScreen(float absChange)
{
	if (gScrWidth > 0 && gScrHeight > 0)
	{
		dispParamsChanged = true;

		float nScale = 2.0f / (float)(gScrWidth + gScrHeight);
		float deltaZ = absChange * nScale;

		{
			float ratio = curZoomY / curZoomX;
			curZoomX += deltaZ;
			if (curZoomX < minZoom)
			{
				curZoomX = minZoom;
			}
			curZoomY = ratio * curZoomX;
		}
	}
}

void Renderer_setZoomPreset(int preset)
{
	int texW = g_videoTex_width;
	int texH = g_videoTex_height;
	int emuScrW = g_surface_width;
	int emuScrH = g_surface_height;

	if (gScrWidth > 0 && gScrHeight > 0 && emuScrW > 0 && emuScrH > 0)
	{
		dispParamsChanged = true;
		_delayedPreset = -1;

		curPanX = 0;
		curPanY = 0;

		switch (preset)
		{
			case ScreenZoom_1:
			case ScreenZoom_2:
			case ScreenZoom_3:
			case ScreenZoom_4:
			{
				curZoomX = (emuScrW*(preset-ScreenZoom_1+1))/(float)gScrWidth;
				curZoomY = (emuScrH*(preset-ScreenZoom_1+1))/(float)gScrHeight;
				break;
			}
			case ScreenZoom_Stretch:
			{
				curZoomX = 1;
				curZoomY = 1;
				break;
			}
			case ScreenZoom_Fit:
			default:
			{
				curZoomY = 1;
				curZoomX = curZoomY * ((float)emuScrW/emuScrH) * ((float)gScrHeight/gScrWidth);
				break;
			}
		}

		//Debug_Printf("******** SET ZOOOM PRESET: %d, %fx%f\n", preset, curZoomX, curZoomY);
	}
	else
	{
		_delayedPreset = preset;
	}
}

void Renderer_panEmuScreen(float absX, float absY)
{
	if (gScrWidth > 0 && gScrHeight > 0)
	{
		dispParamsChanged = true;

		float dX = (2.0f*absX) / (float)gScrWidth;
		float dY = (2.0f*absY) / (float)gScrHeight;

		curPanX += dX;
		curPanY -= dY;
	}
}

void Renderer_setFullScreenStretch(bool fs)
{
	fullScreen = fs;
	if (fullScreen)
	{
		Renderer_setZoomPreset(ScreenZoom_Stretch);
	}
	else
	{
		Renderer_setZoomPreset(ScreenZoom_Fit);
	}
	dispParamsChanged = true;
}

bool Renderer_isFullScreenStretch()
{
	return fullScreen;
}

void Renderer_refreshDispParams()
{
	dispParamsChanged = true;
}

void updateDispParams(int scrWidth, int scrHeight, int texWidth, int texHeight)
{
	if (texWidth > 0 && texHeight > 0)
	{
		dispParamsChanged = false;

		GLfloat *v = emuScreenVerts;

		GLfloat texX = 0.0f;
		GLfloat texY = 0.0f;
		GLfloat texW = (float)scrWidth / (float)texWidth;
		GLfloat texH = (float)scrHeight / (float)texHeight;

		{
			 v[0] = -curZoomX+curPanX;  v[1] = curZoomY+curPanY;		// Position 0
			 v[5] = -curZoomX+curPanX;  v[6] = -curZoomY+curPanY;		// Position 1
			v[10] = curZoomX+curPanX;	v[11] = -curZoomY+curPanY;		// Position 2
			v[15] = curZoomX+curPanX;	v[16] = curZoomY+curPanY;		// Position 3
		}

		 v[3] = texX;  v[4] = texY;		// TexCoord 0
		 v[8] = texX;  v[9] = texH;		// TexCoord 1
		v[13] = texW; v[14] = texH;		// TexCoord 2
		v[18] = texW; v[19] = texY;		// TexCoord 3
	}
}

void renderFrame()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// update texture
	bool validTex = false;
	//if (g_videoTex_mutex)
	if (g_videoFrameReady)
	{
		if (0 == SDL_LockMutex(g_videoTex_mutex))
		{
			GLubyte *emuScrPixels = (GLubyte*)g_videoTex_pixels;
			if (emuScrPixels)
			{
				int texW = g_videoTex_width;
				int texH = g_videoTex_height;
				int emuScrW = g_surface_width;
				int emuScrH = g_surface_height;
				GLenum fmt = (g_videoTex_Bpp == 32) ? GL_RGBA : GL_RGB;
				GLenum fmtDataType = (g_videoTex_Bpp == 32) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5;

				if (_delayedPreset >= 0)
				{
					Renderer_setZoomPreset(_delayedPreset);
				}

				// copy the texture
				if (gTextureID == 0 || g_videoModeChanged)
				{
					allocVideoTex2D(emuScrW, emuScrH, texW, texH, fmt, fmtDataType);
					g_videoModeChanged = 0;
					dispParamsChanged = true;
				}
				if (dispParamsChanged)
				{
					updateDispParams(emuScrW, emuScrH, texW, texH);
				}
				updateVideoTex2D(gTextureID, emuScrPixels, emuScrW, emuScrH, texW, texH, fmt, fmtDataType);

				//g_videoFrameReady = false;
				validTex = true;
				SDL_UnlockMutex(g_videoTex_mutex);
			}
		}
	}

	if (validTex)
	{
		// draw emu screen
		{
			glUseProgram(s_vertTexShader.shaderProgram);

			{
				// Load the vertex position
				glVertexAttribPointer(s_vertTexShader.paramHandles[ShaderParam_Pos], 3, GL_FLOAT, GL_FALSE, stride, emuScreenVerts);

				// Load the texture coordinate
				glVertexAttribPointer(s_vertTexShader.paramHandles[ShaderParam_TexCoord], 2, GL_FLOAT, GL_FALSE, stride, emuScreenVerts+3);

				glEnableVertexAttribArray(s_vertTexShader.paramHandles[ShaderParam_Pos]);
				glEnableVertexAttribArray(s_vertTexShader.paramHandles[ShaderParam_TexCoord]);
			}

			// Bind the texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gTextureID);

			// Set the sampler texture unit to 0
			glUniform1i(s_vertTexShader.paramHandles[ShaderParam_Sampler], 0);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
		}

		for (int r = 0; r < s_numRenderCalbacks; ++r)
		{
			(*s_RenderCallbacks[r])();
		}
	}
}

unsigned int roundUpPower2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	v += (v == 0);

	return v;
}
