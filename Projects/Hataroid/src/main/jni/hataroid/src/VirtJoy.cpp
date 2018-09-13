#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "VirtJoy.h"
#include <GLES2/gl2ext.h>
#include <SDL.h>

#include "VirtKBDefs.h"
#include "VirtKBTex.h"
#include "uncompressGZ.h"
#include "nativeRenderer_ogles2.h"
#include "RTShader.h"
#include "hataroid.h"

#ifndef max
    #define max(a,b) ((a)<(b) ? (b) : (a))
#endif

VirtJoy::VirtJoy()
 :  _enabled(false), _hidden(false), _alpha(1), _autoHideAlpha(1),
    _floating(false), _floatAlpha(1), _floatTimer(0),
    _deadZone(0.1f), _diagSensitivity(0.5f),
    _centerX(0), _centerY(0),
    _joyAreaX1(-1), _joyAreaY1(-1), _joyAreaX2(-1), _joyAreaY2(-1),
    _sizeRatio(0.3f), _scale(1),
    _joyKnobRatio(1), _maxJoyDistRatio(1),
    _curBtnDown(0), _lastJoyFinger(-1), _ready(false),
    _texID(0), _texW(0), _texH(0),
    _verts(0), _indices(0), _vertStrideFloats(0), _numQuads(0),
    _joySprite(0), _joyRingSprite(0)
{
}

VirtJoy::~VirtJoy()
{
    if (_verts != 0)
    {
        free(_verts);
        _verts = 0;
    }
    if (_indices != 0)
    {
        free(_indices);
        _indices = 0;
    }
    _ready = false;
}

void VirtJoy::create(GLuint texID, int texW, int texH, float deadZone, float scale, float diagSensitivity, float alpha, float autoHideAlpha, bool floating)
{
    //int scrW = getScreenWidth();
    //int scrH = getScreenHeight();

    _alpha = alpha;
    _autoHideAlpha = autoHideAlpha;
    _floating = floating;

    _texID = texID;
    _texW = texW;
    _texH = texH;

    _vertStrideFloats = 3+4+2;
    int quadStride = _vertStrideFloats * 4;

    _numQuads = 2;
    int numVerts = _numQuads * 4;
    _verts = (GLfloat*)memalign(128, numVerts*_vertStrideFloats*sizeof(GLfloat));

    int numIndices = _numQuads * 6;
    _indices = (GLushort*)memalign(128, numIndices*sizeof(GLushort));
    int v = 0;
    for (int i = 0; i < _numQuads; ++i)
    {
        int offset = i*4;
        _indices[v++] = offset+0;
        _indices[v++] = offset+1;
        _indices[v++] = offset+2;
        _indices[v++] = offset+0;
        _indices[v++] = offset+2;
        _indices[v++] = offset+3;
    }

    _joySprite = VirtKB_findSpriteDef("jsKnob");
    _joyRingSprite = VirtKB_findSpriteDef("jsRing");

    int uvOffset = 3+4;
    setSpriteUVs(&_verts[uvOffset], _vertStrideFloats, _joySprite);
    setSpriteUVs(&_verts[quadStride + uvOffset], _vertStrideFloats, _joyRingSprite);

    setScale(scale);
    setDeadZone(deadZone);
    setDiagSensitivity(diagSensitivity);

    _joyKnobRatio = (float)max(_joySprite->w, _joySprite->h) / (float)max(_joyRingSprite->w, _joyRingSprite->h);

    _maxJoyDistRatio = (1.0f-_joyKnobRatio)*0.6f;

    _deadZone = 0.4f;
    _curBtnDown = 0;

    _ready = true;
}

void VirtJoy::setTexture(GLuint texID, int texW, int texH)
{
    _texID = texID;
    _texW = texW;
    _texH = texH;
}

void VirtJoy::setFloating(bool floating)
{
    _floating = floating;
    if (!_floating)
    {
        _floatAlpha = 1;
        setScale(_scale);
    }
}

void VirtJoy::setDeadZone(float size)
{
    _deadZone = size;
}

float VirtJoy::calcScreenRadiusPixels()
{
    int scrW = getScreenWidth();
    int scrH = getScreenHeight();
    int refSize = scrH < scrW ? scrH : scrW;

    int joyRingWidth = (int)(refSize * _sizeRatio * _scale * 0.5f);
    return joyRingWidth;
}

void VirtJoy::setScale(float scale)
{
    _scale = scale;

    int joyRingWidth = calcScreenRadiusPixels();
    int scrH = getScreenHeight();

    _centerX = joyRingWidth;
    _centerY = scrH - joyRingWidth;
}

void VirtJoy::setDiagSensitivity(float sensitivity)
{
    float minAngle = (45/180.0f) * M_PI;
    float maxAngle = (90/180.0f) * M_PI;

    float threshAngle = minAngle + ((maxAngle-minAngle)*sensitivity);
    _diagSensitivity = cosf(threshAngle);
}

void VirtJoy::setJoyArea(float x1, float y1, float x2, float y2)
{
    _joyAreaX1 = x1;
    _joyAreaY1 = y1;
    _joyAreaX2 = x2;
    _joyAreaY2 = y2;
}

void VirtJoy::setEnabled(bool enable)
{
    _enabled = enable;
    if (!_enabled)
    {
        _curBtnDown = 0;
    }
}

void VirtJoy::setAlpha(float alpha)
{
    _alpha = alpha;
}

void VirtJoy::setAutoHideAlpha(float alpha)
{
    _autoHideAlpha = alpha;
}

void VirtJoy::setHide(bool hide)
{
    _hidden = hide;
}

bool VirtJoy::IsValidJoyTouch(bool down, float x, float y)
{
    if (down)
    {
        if (x >= _joyAreaX1 && x <= _joyAreaX2
         && y >= _joyAreaY1 && y <= _joyAreaY2)
        {
            return true;
        }
    }
    return false;
}

void VirtJoy::update(int curInputLayer, int curTouchSet, bool touched[2][VKB_MaxTouches], float touchX[2][VKB_MaxTouches], float touchY[2][VKB_MaxTouches], int maxTouches)
{
    _curBtnDown = 0;

    if (!_ready || !_enabled)
    {
        return;
    }

    int joyRingWidth = calcScreenRadiusPixels();
    int joyKnobWidth = (int)(joyRingWidth * _joyKnobRatio);

    int prevIndex = 1 - curTouchSet;

    //bool *prevtouched = touched[prevIndex];
    //float *prevtouchX = touchX[prevIndex];
    //float *prevtouchY = touchY[prevIndex];

    bool *curtouched = touched[curTouchSet];
    float *curtouchX = touchX[curTouchSet];
    float *curtouchY = touchY[curTouchSet];

    // check previous joystick down
    if (_lastJoyFinger >= 0)
    {
        if (!IsValidJoyTouch(curtouched[_lastJoyFinger], curtouchX[_lastJoyFinger], curtouchY[_lastJoyFinger]))
        {
            if (!_floating || !curtouched[_lastJoyFinger])
            {
                _lastJoyFinger = -1;
                _floatTimer = SDL_GetTicks();
            }
        }
    }

    // update new joystick down
    if (_lastJoyFinger == -1)
    {
        for (int i = 0; i < maxTouches; ++i)
        {
            if (IsValidJoyTouch(curtouched[i], curtouchX[i], curtouchY[i]))
            {
                _lastJoyFinger = i;
                _floatAlpha = 1;
                if (_floating)
                {
                    _centerX = curtouchX[i];
                    _centerY = curtouchY[i];
                }
                break;
            }
        }
    }

    // get joystick center
    float joyPosX = _centerX;
    float joyPosY = _centerY;
    float curJoyDist = 0;
    float curJoyDistNorm = 0;

    float joyDirX = 0;
    float joyDirY = 0;

    float maxJoyDist = _maxJoyDistRatio * joyRingWidth;

    if (_lastJoyFinger >= 0)
    {
        joyPosX = curtouchX[_lastJoyFinger];
        joyPosY = curtouchY[_lastJoyFinger];

        float diffX = joyPosX - _centerX;
        float diffY = joyPosY - _centerY;

        curJoyDist = sqrtf(diffX*diffX + diffY*diffY);
        if (curJoyDist > 0)
        {
            joyDirX = diffX / curJoyDist;
            joyDirY = diffY / curJoyDist;

            if (curJoyDist > maxJoyDist)
            {
                joyPosX = _centerX + joyDirX*maxJoyDist;
                joyPosY = _centerY + joyDirY*maxJoyDist;
                curJoyDist = maxJoyDist;
            }

            curJoyDistNorm = curJoyDist / maxJoyDist;
        }
    }

    joyPosX = _centerX;
    joyPosY = _centerY;

	bool acceptInput = (curInputLayer == 0 || curInputLayer == InputLayer_VirtController || curInputLayer == InputLayer_QuickKeys); // joystick fire buttons on quick keys...

    if (acceptInput && curJoyDistNorm > 0 && curJoyDistNorm >= _deadZone)
    {
        joyPosX = 0;
        joyPosY = 0;

        // u, d, l, r
        float dirX[4] = {0, 0, -1, 1};
        float dirY[4] = {-1, 1, 0, 0};
        for (int i = 0; i < 4; ++i)
        {
            float dp = joyDirX*dirX[i] + joyDirY*dirY[i];
            if (dp >= _diagSensitivity)
            {
                _curBtnDown |= (1<<i);
                joyPosX += dirX[i];
                joyPosY += dirY[i];
            }
        }

        float d = joyPosX * joyPosX + joyPosY * joyPosY;
        if (d > 0.0f)
        {
            d = sqrtf(d);
            float s = maxJoyDist / d;
            joyPosX = _centerX + joyPosX*s;
            joyPosY = _centerY + joyPosY*s;
        }
    }

    GLfloat *v = _verts;

    float r = 1, g = 1, b = 1, a = _alpha * _floatAlpha * _autoHideAlpha;

    {
        float jx1 = joyPosX - joyKnobWidth;
        float jy1 = joyPosY - joyKnobWidth;
        float jx2 = joyPosX + joyKnobWidth;
        float jy2 = joyPosY + joyKnobWidth;

        updateVerts(v, jx1, jy1, jx2, jy2, r, g, b, a);
    }

    {
        float jx1 = _centerX - joyRingWidth;
        float jy1 = _centerY - joyRingWidth;
        float jx2 = _centerX + joyRingWidth;
        float jy2 = _centerY + joyRingWidth;

        updateVerts(v+(4*_vertStrideFloats), jx1, jy1, jx2, jy2, r, g, b, a);
    }

    if (_lastJoyFinger == -1)
    {
        if (_floating && _floatAlpha > 0.0f)
        {
            Uint32 nowMS = SDL_GetTicks();
            Uint32 elapsedMS = nowMS - _floatTimer;
            if (elapsedMS > 0)
            {
                _floatAlpha -= 2.0f*(elapsedMS/1000.0f);
                if (_floatAlpha <= 0.0f)
                {
                    _floatAlpha = 0;
                }
            }
            _floatTimer = nowMS;
        }
    }
}

void VirtJoy::render()
{
    if (!_ready || !_enabled || _hidden)
    {
        return;
    }

    RTShader *pShader = Renderer_getColorModShader();

    renderVerts(pShader, _verts, _vertStrideFloats, _texID, _indices, _numQuads);
}

void VirtJoy::updateVerts(GLfloat *v, float jx1, float jy1, float jx2, float jy2,
                                       float r, float g, float b, float a)
{
    int scrW = getScreenWidth();
    int scrH = getScreenHeight();

    float x1 = ((jx1 / (float) scrW) * 2.0f) - 1.0f;
    float y1 = 1.0f - ((jy1 / (float) scrH) * 2.0f);
    float x2 = ((jx2 / (float) scrW) * 2.0f) - 1.0f;
    float y2 = 1.0f - ((jy2/ (float) scrH) * 2.0f);

    float x[4] = {x1, x1, x2, x2};
    float y[4] = {y1, y2, y2, y1};

    int c = 0;
    for (int i = 0; i < 4; ++i)
    {
        v[c++] = x[i];	v[c++] = y[i];	v[c++] = 0;
        v[c++] = r;		v[c++] = g;		v[c++] = b;		v[c++] = a;
        c += 2;
    }
}


void VirtJoy::setSpriteUVs(GLfloat *v, int vstrideFloats, const SpriteDef* sprite)
{
    float tx1 = sprite->x1 / (float)_texW;
    float ty1 = sprite->y1 / (float)_texH;
    float tx2 = sprite->x2 / (float)_texW;
    float ty2 = sprite->y2 / (float)_texH;

    int i = 0;            v[i] = tx1; v[i+1] = ty1;
    i += vstrideFloats;   v[i] = tx1; v[i+1] = ty2;
    i += vstrideFloats;   v[i] = tx2; v[i+1] = ty2;
    i += vstrideFloats;   v[i] = tx2; v[i+1] = ty1;
}

void VirtJoy::renderVerts(RTShader *pShader, GLfloat *v, GLsizei vstrideFloats, GLuint texID, GLushort *ind, int numQuads)
{
    if (pShader == 0 || !pShader->_ready)
    {
        return;
    }

    GLsizei vstride = vstrideFloats * sizeof(GLfloat);

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
