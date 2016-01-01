#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <jni.h>

#include "hataroid.h"
#include "RTShader.h"

//------------------------------------------------------------------------------------------------------------------
static char* allocStr(const char* s)
{
	char* n = new char[strlen(s)+1];
	strcpy(n, s);
	return n;
}

class ShaderDef
{
	public:
		const char* paramNameMap[RTShader::ShaderParam_Max];
		const char* vsh;
		const char* fsh;

	private:
		static const int kOK		= 0;
		static const int kEOF		= -1;
		static const int kERROR		= -2;

	public:
		ShaderDef();
		~ShaderDef();

		bool parseDef(const char* def);

	private:
		void clear();
		int parseKeyVal(const char* def, int& rCurOffset, bool allowSpaceVal, char* key, int maxKeyLen, char* val, int maxValLen);
		int parseToken(const char* def, int& rCurOffset, bool allowSpace, char *resBuf, int resBufLen);
		int skipToken(const char* def, int& rCurOffset, bool allowSpace, const char* cmpToken, char *resBuf, int resBufLen);
		int skipWhitespace(const char* def, int& rCurOffset);
};

ShaderDef::ShaderDef()
	: vsh(0), fsh(0)
{
	memset(paramNameMap, 0, sizeof(paramNameMap));
}

ShaderDef::~ShaderDef()
{
	clear();
}

void ShaderDef::clear()
{
	for (int i = 0; i < RTShader::ShaderParam_Max; ++i)
	{
		if (paramNameMap[i] != 0)
		{
			delete [] paramNameMap[i];
			paramNameMap[i] = 0;
		}
	}
	if (vsh != 0)
	{
		delete [] vsh;
		vsh = 0;
	}
	if (fsh != 0)
	{
		delete [] fsh;
		fsh = 0;
	}
}

bool ShaderDef::parseDef(const char* def)
{
	clear();

	//printf("starting parse\n");
	const int kMaxKeyBufLen = 256;
	const int kMaxValBufLen = 1024;
	char* key = new char [kMaxKeyBufLen];
	char* val = new char [kMaxValBufLen];
	int curOffset = 0;
	int res;
	while ((res = parseKeyVal(def, curOffset, true, key, kMaxKeyBufLen, val, kMaxValBufLen)) == kOK)
	{
		if (strcasecmp(key, "vsh") == 0)
		{
			if (vsh==0)
			{
			 	vsh = allocStr(val);
			}
			//printf("found vsh: '%s'\n", vsh);
		}
		else if (strcasecmp(key, "fsh") == 0)
		{
			if (fsh==0)
			{
				fsh = allocStr(val);
			}
			//printf("found fsh: '%s'\n", fsh);
		}
		else
		{
			int paramIdx = RTShader::findParamIndex(key);
			if (paramIdx < 0)
			{
				// show warning
				//printf("warning, invalid param: '%s' = '%s'\n", key, val);
			}
			else
			{
				if (paramNameMap[paramIdx] == 0)
				{
					paramNameMap[paramIdx] = allocStr(val);
				}
				//printf("found param: '%s' = '%s'\n", key, paramNameMap[paramIdx]);
			}
		}
	}

	//printf("parse done\n");

	delete [] key;
	delete [] val;

	if (res == kERROR || vsh == 0 || fsh == 0)
	{
		clear();
		return false;
	}
	return true;
}

int ShaderDef::parseKeyVal(const char* def, int& rCurOffset, bool allowSpaceVal, char* key, int maxKeyLen, char* val, int maxValLen)
{
	int res;

	if ((res=skipWhitespace(def, rCurOffset)) != kOK)								{ return res; } // check EOF

	if ((res=parseToken(def, rCurOffset, false, key, maxKeyLen)) != kOK)			{ return kERROR; } // key
	if ((res=skipToken(def, rCurOffset, false, "=", val, maxValLen)) != kOK)		{ return kERROR; } // =
	if ((res=parseToken(def, rCurOffset, allowSpaceVal, val, maxValLen)) != kOK)	{ return kERROR; } // val

	//printf("parsed key val: %s = %s\n", key, val);
	return kOK;
}

int ShaderDef::skipToken(const char* def, int& rCurOffset, bool allowSpace, const char* cmpToken, char *resBuf, int resBufLen)
{
	if (parseToken(def, rCurOffset, false, resBuf, resBufLen) != kOK)
	{
		return kERROR;
	}

	return (strcmp(resBuf, cmpToken) == 0) ? kOK : kERROR;
}

int ShaderDef::parseToken(const char* def, int& rCurOffset, bool allowSpace, char *resBuf, int resBufLen)
{
	if (skipWhitespace(def, rCurOffset) != kOK)
	{
		return kERROR;
	}

	int rlen = 0;
	bool done = false;
	while (!done)
	{
		int c = def[rCurOffset];
		if (c == 0 || c == '\t' || c == '\r' || c == '\n' || (!allowSpace && (c == ' ')))
		{
			// trim trailing whitespace
			resBuf[rlen] = 0; // check far below ensures we have enough space
			--rlen;
			while (rlen >= 0)
			{
				c = resBuf[rlen];
				if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
				{
					break;
				}
				resBuf[rlen] = 0;
				--rlen;
			}
			++rlen;
			return (rlen > 0) ? kOK : kERROR;
		}

		resBuf[rlen++] = c;
		if (rlen >= resBufLen)
		{
			return kERROR; // out of buf space
		}

		++rCurOffset;
	}

	return kERROR;
}

int ShaderDef::skipWhitespace(const char* def, int& rCurOffset)
{
	bool done = false;
	while (!done)
	{
		int c = def[rCurOffset];
		if (c == 0)
		{
			return kEOF;
		}

		done = (c != ' ' && c != '\t' && c != '\r' && c != '\n');
		rCurOffset += (done ? 0 : 1);
	}

	return kOK;
}

//------------------------------------------------------------------------------------------------------------------

const char* RTShader::kParamNames[ShaderParam_Max] =
{
	"pos",			//ShaderParam_Pos = 0,
	"uv",			//ShaderParam_TexCoord,
	"color",		//ShaderParam_Color,

	"tex",			//ShaderParam_Sampler,

	"texsize",		//ShaderParam_TexSize,
	"texsizepow2",	//ShaderParam_TexSizePow2,

	"screensize",	//ShaderParam_OutputSize,
};

const int RTShader::kParamLocTypes[ShaderParam_Max] =
{
	ParamLocType_Attrib,	//ShaderParam_Pos = 0,
	ParamLocType_Attrib,	//ShaderParam_TexCoord,
	ParamLocType_Attrib,	//ShaderParam_Color,

	ParamLocType_Uniform,	//ShaderParam_Sampler,

	ParamLocType_Uniform,	//ShaderParam_TexSize,
	ParamLocType_Uniform,	//ShaderParam_TexSizePow2,

	ParamLocType_Uniform,	//ShaderParam_OutputSize,
};

int RTShader::findParamIndex(const char *paramName)
{
	for (int i = 0; i < RTShader::ShaderParam_Max; ++i)
	{
		if (strcasecmp(RTShader::kParamNames[i], paramName) == 0)
		{
			return i;
		}
	}
	return -1;
}

int RTShader::getParamLocType(int paramIdx)
{
	return RTShader::kParamLocTypes[paramIdx];
}

RTShader::RTShader()
:	_ready(false),
	_shaderProgram(0),
	_shaderPath(0),
	_shaderName(0),
	_vertShader(0),
	_fragShader(0)
{
	resetParamHandles();
}

RTShader::~RTShader()
{
	unloadShader();
}

void RTShader::resetParamHandles()
{
	for (int i = 0; i < ShaderParam_Max; ++i)
	{
		_paramHandles[i] = -1;
	}
}

bool RTShader::isShaderName(const char* shaderName)
{
	if (_shaderName != 0 && shaderName != 0)
	{
		return (strcmp(_shaderName, shaderName) == 0);
	}
	return false;
}

bool RTShader::loadShader(JNIEnv* env, const char* shaderName, const char* shaderPath)
{
	unloadShader();

	bool res = false;

	char* shaderDefData = loadFile(env, shaderPath, true);
	if (shaderDefData != 0)
	{
		ShaderDef* def = new ShaderDef();
		if (def->parseDef(shaderDefData))
		{
			printf("\nload shader def OK: %s\n", shaderPath);

			res = initShader(env, shaderName, def);
		}
		else
		{
			printf("\nload shader def FAIL: %s\n", shaderPath);
		}
		delete def;

		delete [] shaderDefData;
	}

	return res;
}

bool RTShader::loadShaderDirect(const char* shaderName, const char* pVertexSource, const char* pFragmentSource, const char* attribNames[RTShader::ShaderParam_Max])
{
	unloadShader();
	return createShader(shaderName, pVertexSource, pFragmentSource, attribNames);
}

void RTShader::unloadShader()
{
	_ready = false;

	if (_shaderProgram != 0)
	{
		glDeleteProgram(_shaderProgram);
		_shaderProgram = 0;
	}

	if (_vertShader != 0)
	{
		glDeleteShader(_vertShader);
		_vertShader = 0;
	}

	if (_fragShader != 0)
	{
		glDeleteShader(_fragShader);
		_fragShader = 0;
	}

	resetParamHandles();

	if (_shaderPath != 0)
	{
		delete [] _shaderPath;
		_shaderPath = 0;
	}

	if (_shaderName != 0)
	{
		delete [] _shaderName;
		_shaderName = 0;
	}
}

char* RTShader::loadFile(JNIEnv* env, const char* path, bool nullTerm)
{
	if (path != 0)
	{
		if (strncasecmp(path, "asset://", 8) == 0)
		{
			int len = 0;
			char *buf = (char*)hataroid_getAssetDataDirect(env, &path[8], nullTerm?1:0, &len);
			return buf;
		}
		else if (strncasecmp(path, "file://", 7) == 0)
		{
			// TODO:
		}
	}

	return 0;
}

bool RTShader::initShader(JNIEnv * env, const char* shaderName, ShaderDef* def)
{
	if (env == 0)
	{
		return false;
	}

	bool res = false;

	char *vsh = loadFile(env, def->vsh, true);
	char *fsh = loadFile(env, def->fsh, true);

	if (vsh != 0 && fsh != 0)
	{
		Debug_Printf("loading shaders from assets");
		if (!(res = createShader(shaderName, vsh, fsh, def->paramNameMap)))
		{
			Debug_Printf("Could not create shader.");
		}
	}
	if (vsh != 0) { delete [] vsh; }
	if (fsh != 0) { delete [] fsh; }

	return res;
}

bool RTShader::createShader(const char* shaderName, const char* pVertexSource, const char* pFragmentSource, const char* attribNames[RTShader::ShaderParam_Max])
{
	if (_shaderName != 0)
	{
		delete [] _shaderName;
	}
	_shaderName = allocStr(shaderName);

	_shaderProgram = createProgram(pVertexSource, pFragmentSource);
	if (!_shaderProgram)
	{
		Debug_Printf("Could not create program.");
		return false;
	}

	for (int i = 0; i < RTShader::ShaderParam_Max; ++i)
	{
		if (attribNames[i])
		{
			int attrLocType = getParamLocType(i);
			if (attrLocType == RTShader::ParamLocType_Uniform)
			{
				_paramHandles[i] = glGetUniformLocation(_shaderProgram, attribNames[i]);
				checkGlError("glGetUniformLocation");
				Debug_Printf("%s: glGetUniformLocation(\"%s\") = %d\n", _shaderName, attribNames[i], _paramHandles[i]);
			}
			else if (attrLocType == RTShader::ParamLocType_Attrib)
			{
				_paramHandles[i] = glGetAttribLocation(_shaderProgram, attribNames[i]);
				checkGlError("glGetAttribLocation");
				Debug_Printf("%s: glGetAttribLocation(\"%s\") = %d\n", _shaderName, attribNames[i], _paramHandles[i]);
			}
		}
	}

	_ready = true;
	return true;
}

GLuint RTShader::createProgram(const char* pVertexSource, const char* pFragmentSource)
{
	_vertShader = compileShaderSource(GL_VERTEX_SHADER, pVertexSource);
	if (!_vertShader)
	{
		return 0;
	}

	_fragShader = compileShaderSource(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!_fragShader)
	{
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program)
	{
		glAttachShader(program, _vertShader);
		checkGlError("glAttachShaderV");

		glAttachShader(program, _fragShader);
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

GLuint RTShader::compileShaderSource(GLenum shaderType, const char* pSource)
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

bool RTShader::checkGlError(const char* op)
{
	bool hasErrors = false;

	for (GLint error = glGetError(); error; error = glGetError())
	{
		Debug_Printf("after %s() glError (0x%x)\n", op, error);
		hasErrors = true;
	}

	return hasErrors;
}
