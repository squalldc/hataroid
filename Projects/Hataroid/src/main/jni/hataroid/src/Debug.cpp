#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <SDL.h>

#include "nativeRenderer_ogles2.h"
#include "Debug.h"
#include "RTShader.h"
#include "BitFlags.h"
#include "hataroid.h"

#include <sound.h>

bool                g_dbgShowSoundBuffer = 0;

static const int	s_maxDebugQuads = 20;
static GLfloat		s_dbgVerts[36*s_maxDebugQuads];
static GLushort		s_dbgIndices[6*s_maxDebugQuads];
static GLsizei		s_dbgStride = 9 * sizeof(GLfloat); // 3 position, 4 color, 2 texture
static int			s_dbgNumQuads = 0;

static void Debug_CleanUp();
static void Debug_Create();
static void Debug_DestroyTextures();
static void Debug_CreateTextures();
static void Debug_DestroyShader();
static void Debug_SetupShader();
static void Debug_RenderVerts(RTShader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numQuads);
static void Debug_onRender(JNIEnv *env);
static void Debug_UpdateRectVerts(	GLfloat *v, float x1, float y1, float x2, float y2,
								float u1, float v1, float u2, float v2,
								float r, float g, float b, float a);

int Debug_OnSurfaceChanged(int width, int height)
{
	Debug_CleanUp();
	Debug_Create();

	return 0;
}

static void Debug_CleanUp()
{
	Debug_DestroyShader();
	Debug_DestroyTextures();

	s_dbgNumQuads = 0;
}

static void Debug_Create()
{
	Debug_CreateTextures();
	Debug_SetupShader();
}

static void Debug_DestroyTextures()
{
}

static void Debug_CreateTextures()
{
}

static void Debug_DestroyShader()
{
	//Renderer_removeRenderCallback(Debug_onRender);
}

static void Debug_SetupShader()
{
	Renderer_addRenderCallback(Debug_onRender);

	int indexes[6] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < s_maxDebugQuads; ++i)
	{
		GLushort *ind = &s_dbgIndices[i*6];
		GLushort offset = i << 2;
		for (int j = 0; j < 6; ++j)
		{
			ind[j] = indexes[j] + offset;
		}
	}
}

static void Debug_RenderVerts(RTShader *pShader, GLfloat *v, GLsizei vstride, GLuint texID, GLushort *ind, int numQuads)
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

static void Debug_RenderAudio(JNIEnv *env)
{
	RTShader *pShader = Renderer_getColorModShader();

	{
		//-1,-1 to 1, 1 (bl -> tr)
		if (s_dbgNumQuads < (s_maxDebugQuads-1))
		{
			float tx1 = 0.0f, ty1 = 0.0f;
			float tx2 = 1.0f, ty2 = 1.0f;

			float barLen = 0.7f;

			float x1 = 0, y1 = 0.95f;
			float x2 = barLen, y2 = 0.99f;

			{
				GLfloat *v = &s_dbgVerts[s_dbgNumQuads*36];
				Debug_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, 1.0f, 1.0f, 1.0f, 0.5f);

				++s_dbgNumQuads;
			}
			{
				float fill = nGeneratedSamples / (float)MIXBUFFER_SIZE;
				x2 = barLen * fill;

				GLfloat *v = &s_dbgVerts[s_dbgNumQuads*36];
				Debug_UpdateRectVerts(v, x1, y1, x2, y2, tx1, ty1, tx2, ty2, 1.0f, 0.3f, 0.3f, 0.8f);

				++s_dbgNumQuads;
			}
		}
	}
}

static void Debug_onRender(JNIEnv *env)
{
	RTShader *pShader = Renderer_getColorModShader();

	s_dbgNumQuads = 0;

	if (g_dbgShowSoundBuffer)
	{
		Debug_RenderAudio(env);
	}

	if (s_dbgNumQuads > 0)
	{
		GLuint dbgTexID = getWhiteTexture();
		Debug_RenderVerts(pShader, s_dbgVerts, s_dbgStride, dbgTexID, s_dbgIndices, s_dbgNumQuads);
	}
}

static void Debug_UpdateRectVerts(	GLfloat *v, float x1, float y1, float x2, float y2,
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
