#ifndef VIRTJOY_H_
#define VIRTJOY_H_

#include <GLES2/gl2.h>
#include <SDL.h>

#include "VirtKBDefs.h"
#include "VirtKB.h"

class RTShader;

class VirtJoy
{
    enum BtnFlags
    {
        Up      = ((unsigned int)1<<0),
        Down    = ((unsigned int)1<<1),
        Left    = ((unsigned int)1<<2),
        Right   = ((unsigned int)1<<3),
    };

    public:
        VirtJoy();
        ~VirtJoy();

    public:
        void create(GLuint texID, int texW, int texH, float deadZone, float scale, float diagSensitivity, float alpha, float autoHideAlpha, bool floating);
        void update(int curInputLayer, int curTouchSet, bool touched[2][VKB_MaxTouches], float touchX[2][VKB_MaxTouches], float touchY[2][VKB_MaxTouches], int maxTouches);
        void render();

        unsigned int getBtnFlags() { return _curBtnDown; }

        void setJoyArea(float x1, float y1, float x2, float y2);

        void setFloating(bool floating);
        void setDeadZone(float size);
        void setScale(float scale);
        void setDiagSensitivity(float sensitivity);

        float calcScreenRadiusPixels();

        bool isEnabled() { return _enabled; }
        void setEnabled(bool enable);

        void setAutoHideAlpha(float alpha);
        void setAlpha(float alpha);
        void setHide(bool hide);

    private:
        void renderVerts(RTShader* pShader, GLfloat* v, GLsizei vstrideFloats, GLuint texID, GLushort* ind, int numQuads);

        void setSpriteUVs(GLfloat *v, int vstrideFloats, const SpriteDef* sprite);
        void updateVerts(GLfloat *v, float jx1, float jy1, float jx2, float jy2, float r, float g, float b, float a);

        bool IsValidJoyTouch(bool down, float x, float y);

    private:
        bool                _enabled;
        bool                _hidden;
        float               _alpha;
        float               _autoHideAlpha;

        bool                _floating;
        float               _floatAlpha;
        Uint32              _floatTimer;
        float               _deadZone;
        float               _diagSensitivity; // > 45deg = overlap for diagonal dir

        float               _centerX;
        float               _centerY;

        float               _joyAreaX1;
        float               _joyAreaY1;
        float               _joyAreaX2;
        float               _joyAreaY2;

        float               _sizeRatio;
        float               _scale;

        float               _joyKnobRatio;
        float               _maxJoyDistRatio;

        unsigned int        _curBtnDown;

        int                 _lastJoyFinger;

        bool                _ready;

    private:
        GLuint              _texID;
        int                 _texW;
        int                 _texH;

        GLfloat*            _verts;
        GLushort*           _indices;
        int                 _vertStrideFloats;
        int                 _numQuads;

    private:
        const SpriteDef*    _joySprite;
        const SpriteDef*    _joyRingSprite;
};

#endif /* VIRTJOY_H_ */
