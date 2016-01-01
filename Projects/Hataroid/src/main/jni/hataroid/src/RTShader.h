#ifndef __RTSHADER_H__
#define __RTSHADER_H__

#include <GLES2/gl2.h>
#include <jni.h>

class ShaderDef;

class RTShader
{
	public:
		enum
		{
			ShaderParam_Pos = 0,
			ShaderParam_TexCoord,
			ShaderParam_Color,

			ShaderParam_Sampler,

			ShaderParam_TexSize,
			ShaderParam_TexSizePow2,

			ShaderParam_OutputSize,

			ShaderParam_Max
		};
		enum
		{
			ParamLocType_Uniform = 0,
			ParamLocType_Attrib,
		};

		static const char* kParamNames[ShaderParam_Max];
		static const int kParamLocTypes[ShaderParam_Max];
		static int findParamIndex(const char *paramName);
		static int getParamLocType(int paramIdx);

	public:
		bool		_ready;
		GLuint		_shaderProgram;
		GLint		_paramHandles[ShaderParam_Max];
		const char*	_shaderPath;
		const char* _shaderName;

	private:
		GLuint		_vertShader;
		GLuint		_fragShader;

	public:
		RTShader();
		~RTShader();

		bool isShaderName(const char* shaderName);

		bool loadShader(JNIEnv* env, const char* shaderName, const char* shaderPath);
		bool loadShaderDirect(const char* shaderName, const char* pVertexSource, const char* pFragmentSource, const char* attribNames[RTShader::ShaderParam_Max]);

		void unloadShader();

	private:
		char* loadFile(JNIEnv* env, const char* path, bool nullTerm);

		void resetParamHandles();
		bool initShader(JNIEnv * env, const char* shaderName, ShaderDef* def);
		bool createShader(const char* shaderName, const char* pVertexSource, const char* pFragmentSource, const char* attribNames[RTShader::ShaderParam_Max]);
		GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);
		GLuint compileShaderSource(GLenum shaderType, const char* pSource);
		bool checkGlError(const char *op);
};

#endif //__RTSHADER_H__
