/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#ifndef _SDL_android_opensles_audio_h
#define _SDL_android_opensles_audio_h

#include "../SDL_sysaudio.h"
#include <hataroid.h>

// Hidden "this" pointer for the audio functions
#define _THIS	SDL_AudioDevice *this

// private audio thread data
struct SDL_PrivateAudioData {

	// JNI vars
	JNIEnv *        playThreadjenv;
	int			    playThreadEnvAttached;

	// engine interfaces
	SLObjectItf     engineObject;
	SLEngineItf     engineEngine;

	// output mix interfaces
	SLObjectItf     outputMixObject;

	// buffer queue player interfaces
	SLObjectItf     bqPlayerObject;
	SLPlayItf       bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf   bqPlayerBufferQueue;
	SLMuteSoloItf   bqPlayerMuteSolo;
	//SLVolumeItf     bqPlayerVolume;

	//SLmillibel      defVolLevel;


	SDL_mutex *     freeBufLock;

	// buffers
	int             numBufs;
	volatile int    freeBufs;
	int             nextBuf;

	Uint8 **        audioBufs;
	int             bufSizeBytes;
};

#endif /* _SDL_android_opensles_audio_h */
