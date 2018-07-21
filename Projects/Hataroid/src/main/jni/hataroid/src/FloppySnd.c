#include <SDL_types.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sound.h>
#include <statusbar.h>

#include "hataroid.h"
#include "FloppySnd.h"

extern int nFrameSkips;

#define SRC_SND_FREQ            44100
#define ON_FADE_FRAMES          10
#define SEEK_SHORT_FRAMES       4
#define SEEK_LONG_FRAMES        8
#define MOTOR_STOP_FRAMES       90 // 1.8sec at 50hz //TODO

static short*       _refMotorOnSnd = 0;
static int          _refMotorOnLen = 0;
static short*       _refSeekLongSnd = 0;
static int          _refSeekLongLen = 0;
static short*       _refSeekShortSnd = 0;
static int          _refSeekShortLen = 0;

static short*       _motorOnSnd = 0;
static int          _motorOnBufLen = 0;
static int          _motorOnLen = 0;
static short*       _seekLongSnd = 0;
static int          _seekLongBufLen = 0;
static int          _seekLongLen = 0;
static short*       _seekShortSnd = 0;
static int          _seekShortBufLen = 0;
static int          _seekShortLen = 0;

static bool         _enabled = false;
static bool         _setup = false;

static int          _reqFreq = SRC_SND_FREQ;
static float        _reqVol = 0;

static int          _curFreq = 0;
static float        _curVol = 0;

static int          _onFrame = 0;
static int          _seekFrame = 0;

static int          _curOnPos = 0;
static int          _curSeekLongPos = 0;
static int          _curSeekShortPos = 0;

static int          _prevTrack = 0;
static int          _seeking = false;
static int          _stopDelay = 0;


int _FloppySnd_Resample(short* src, int srcLen, int outFreq, float outVol, short** dst, int* dstBufLen);
inline int _min(int a, int b) { return (a < b) ? a : b; }

int FloppySnd_Init(JNIEnv* curEnv)
{
    //_refMotorOnSnd = (short*)hataroid_getAssetDataDirect(curEnv, "audio/test.raw", 0, &_refMotorOnLen);
    _refMotorOnSnd = (short*)hataroid_getAssetDataDirect(curEnv, "audio/motor_on.raw", 0, &_refMotorOnLen);
    _refMotorOnLen >>= 1;
    _refSeekLongSnd = (short*)hataroid_getAssetDataDirect(curEnv, "audio/head_seek_long.raw", 0, &_refSeekLongLen);
    _refSeekLongLen >>= 1;
    _refSeekShortSnd = (short*)hataroid_getAssetDataDirect(curEnv, "audio/head_seek_short.raw", 0, &_refSeekShortLen);
    _refSeekShortLen >>= 1;

	return 0;
}

void FloppySnd_DeInit()
{
	if (_refMotorOnSnd != 0) {
		hataroid_freeAssetDataDirect((char*)_refMotorOnSnd);
		_refMotorOnSnd = 0;
		_refMotorOnLen = 0;
	}

	if (_refSeekLongSnd != 0) {
		hataroid_freeAssetDataDirect((char*)_refSeekLongSnd);
		_refSeekLongSnd = 0;
		_refSeekLongLen = 0;
	}

	if (_refSeekShortSnd != 0) {
		hataroid_freeAssetDataDirect((char*)_refSeekShortSnd);
		_refSeekShortSnd = 0;
		_refSeekShortLen = 0;
	}

	if (_motorOnSnd != 0) {
		free(_motorOnSnd);
		_motorOnSnd = 0;
		_motorOnLen = 0;
		_motorOnBufLen = 0;
	}

	if (_seekLongSnd != 0) {
		free(_seekLongSnd);
		_seekLongSnd = 0;
		_seekLongLen = 0;
		_seekLongBufLen = 0;
	}

	if (_seekShortSnd != 0) {
		free(_seekShortSnd);
		_seekShortSnd = 0;
		_seekShortLen = 0;
		_seekShortBufLen = 0;
	}
}

int FloppySnd_IsEnabled()
{
	return _enabled;
}

void FloppySnd_Enable(int enable)
{
	_enabled = enable;
}

void FloppySnd_SetOutVol(float outVol)
{
	_reqVol = outVol;
}

void FloppySnd_SetOutFreq(int outFreq)
{
	if (outFreq > 0)
	{
		_reqFreq = outFreq;
	}
}

int FloppySnd_Setup()
{
	float outVol = _reqVol;
	int outFreq = _reqFreq;

	if (outFreq == _curFreq && outVol == _curVol)
	{
		return 0;
	}

	_motorOnLen = _FloppySnd_Resample(_refMotorOnSnd, _refMotorOnLen, outFreq, outVol, &_motorOnSnd, &_motorOnBufLen);
	_seekLongLen = _FloppySnd_Resample(_refSeekLongSnd, _refSeekLongLen, outFreq, outVol, &_seekLongSnd, &_seekLongBufLen);
	_seekShortLen = _FloppySnd_Resample(_refSeekShortSnd, _refSeekShortLen, outFreq, outVol, &_seekShortSnd, &_seekShortBufLen);

	_curFreq = outFreq;
	_curVol = outVol;

	_setup = true;
	return 0;
}

int _FloppySnd_Resample(short* src, int srcLen, int outFreq, float outVol, short** dst, int* dstBufLen)
{
	int newLen = (int)( ((outFreq / (float)SRC_SND_FREQ) * srcLen) + 0.5f); // round up
	if (newLen > (*dstBufLen))
	{
		if ((*dst) != 0)
		{
			free(*dst);
		}
		*dst = (short*)malloc(newLen*sizeof(short));
		*dstBufLen = newLen;
	}

	{
		short* s = src;
		short* d = *dst;
		float incr = ((float)SRC_SND_FREQ) / outFreq;
		float t = 0;
		int i = 0;
		for (; i < newLen; ++i)
		{
			int t1 = _min((int)t, srcLen-1);
			int t2 = _min(t1 + 1, srcLen-1);
			float r = t - t1;

			float v = s[t1] + (r * (s[t2] - s[t1]));
			v *= outVol;

			d[i] = (int)v;

			t += incr;
		}
	}

	return newLen;
}

void FloppySnd_Reset()
{
	_onFrame = 0;
	_seekFrame = 0;

	_curOnPos = 0;
	_curSeekLongPos = 0;
	_curSeekShortPos = 0;

	_prevTrack = 0;
	_seeking = false;
	_stopDelay = 0;
}

void FloppySnd_UpdateEmuFrame()
{
//	drive_led_t driveState = Statusbar_GetFloppyLed(DRIVE_LED_A);
//	int motorOn = driveState != LED_STATE_OFF;

	bool driveActive = FDC_Get_HasActiveCommands();
	int fDT = nFrameSkips + 1;

	if (driveActive) {
		_stopDelay = MOTOR_STOP_FRAMES;
	} else {
		if (_stopDelay > 0) {
			_stopDelay -= fDT;
			if (_stopDelay < 0) {
				_stopDelay = 0;
			}
		}
	}
	int motorOn = _stopDelay > 0;

	 // post seek, but still sounds fine
	int curTrack = FDC_Get_CurTrack();
	int trackChanged = curTrack != _prevTrack;
	if (trackChanged) {
		if (_seeking < SEEK_SHORT_FRAMES) {
		    _seeking = SEEK_SHORT_FRAMES;
	    } else if (_seeking < SEEK_LONG_FRAMES) {
			_seeking += fDT;
			if (_seeking > SEEK_LONG_FRAMES) {
				_seeking = SEEK_LONG_FRAMES;
			}
		}
	} else {
		if (_seeking > 0) {
			_seeking -= fDT;
			if (_seeking <= 0) {
				_curSeekShortPos = 0;
			}
		}
	}
	_prevTrack = curTrack;


	// was going to fade in/out the motor sounds, but sounds ok without it
	/*
	{
		if (motorOn) {
			if (_onFrame < ON_FADE_FRAMES) {
				++_onFrame;
			}
		} else {
			if (_onFrame > 0) {
				--_onFrame;
			}
		}
	}
	*/
	_onFrame = motorOn;
}

void FloppySnd_GenSamples(int nMixBufIdx, int samplesToGen)
{
	if (!_enabled || !_setup)
	{
		return;
	}

	if (_onFrame > 0)
	{
		int i = 0, idx;
		for (; i < samplesToGen; ++i)
		{
			idx = (nMixBufIdx + i) % MIXBUFFER_SIZE;

			int m = _motorOnSnd[_curOnPos++];
			if (_curOnPos >= _motorOnLen) {
				_curOnPos = 0;
			}

			int s = 0;
			if (_seeking > 0) {
				if (_curSeekShortPos < _seekShortLen) {
					s = _seekShortSnd[_curSeekShortPos++];
				} else {
					s = _seekLongSnd[_curSeekLongPos++];
					if (_curSeekLongPos >= _seekLongLen) {
						_curSeekLongPos = 0;
					}
				}
			}

			int v = m + s;
			int l = MixBuffer[idx][0] + v;
			int r = MixBuffer[idx][1] + v;
			MixBuffer[idx][0] = (l>32767) ? 32767 : (l<-32767) ? -32767 : l;
			MixBuffer[idx][1] = (r>32767) ? 32767 : (r<-32767) ? -32767 : r;
		}
	}
}
