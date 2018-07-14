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
#include <assert.h>
#include <stdbool.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "SDL_config.h"
#include "SDL_mutex.h"

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_android_opensles_audio.h"

// The tag name used by audio
#define ANDROID_OPENSLES_DRIVER_NAME         "android-opensles-audio"

static SDL_AudioDevice* ANDROID_OPENSLES_CreateDevice(int devindex);
static void             ANDROID_OPENSLES_DeleteDevice(SDL_AudioDevice *device);
static int              ANDROID_OPENSLES_Available(void);

AudioBootStrap ANDROIDAUDIO_OPENSLES_bootstrap = {
	ANDROID_OPENSLES_DRIVER_NAME, "SDL android opensles native audio driver",
	ANDROID_OPENSLES_Available, ANDROID_OPENSLES_CreateDevice
};

// Audio driver functions
static int              ANDROID_OPENSLES_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void             ANDROID_OPENSLES_WaitAudio(_THIS);
static void             ANDROID_OPENSLES_PlayAudio(_THIS);
static Uint8*           ANDROID_OPENSLES_GetAudioBuf(_THIS);
static void             ANDROID_OPENSLES_CloseAudio(_THIS);
static void             ANDROID_OPENSLES_MuteAudio(_THIS, int mute);

static void             ANDROID_OPENSLES_ThreadInit(_THIS);
static void             ANDROID_OPENSLES_ThreadDeinit(_THIS);

static int              createEngine(_THIS);
static void             shutdownEngine(_THIS);
static int              createBufferQueueAudioPlayer(_THIS, SDL_AudioSpec *spec);
static void             destroyBufferQueueAudioPlayer(_THIS);
static void             bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
static void             playStream(_THIS);
static void             muteStream(_THIS, SLboolean mute);
static void             checkMuteStream(_THIS);

// ----
extern JavaVM*          g_jvm;
extern struct JNIAudio  g_jniAudioInterface;
//extern volatile int     g_validMainActivity;

//extern volatile int     g_emuReady;
//extern int              kMaxAudioMuteFrames;
extern volatile int     g_audioMute;
extern volatile int     g_audioMuteFrames;

// Audio driver bootstrap functions
static int ANDROID_OPENSLES_Available(void)
{
	const char *envr = SDL_getenv("SDL_AUDIODRIVER");
	if (envr && (SDL_strcmp(envr, ANDROID_OPENSLES_DRIVER_NAME) == 0)) {
		return(1);
	}
	return(0);
}

static void ANDROID_OPENSLES_ThreadInit(_THIS)
{
	Debug_Printf("SDL_Audio OPENSLES Thread Init");

	struct SDL_PrivateAudioData *myData = this->hidden;

	JNIEnv *env = NULL;

	int status = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
	int attached = 0;

	if(status < 0)
	{
		status = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
		attached = 1;
	}

	myData->playThreadEnvAttached = attached;
	myData->playThreadjenv = env;

	this->spec.userdata = env; // pass our jni env in the userdata
}

static void ANDROID_OPENSLES_ThreadDeinit(_THIS)
{
	Debug_Printf("SDL_Audio OPENSLES Thread DeInit");

	struct SDL_PrivateAudioData *myData = this->hidden;

	JNIEnv *j_env = myData->playThreadjenv;

	if (myData->playThreadEnvAttached)
	{
		(*g_jvm)->DetachCurrentThread(g_jvm);
	}
	myData->playThreadEnvAttached = 0;
	myData->playThreadjenv = 0;
}

static SDL_AudioDevice *ANDROID_OPENSLES_CreateDevice(int devindex)
{
	Debug_Printf("SDL_Audio OPENSLES Create Device");

	SDL_AudioDevice *this;

	// Initialize all variables that we clean on shutdown
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	// Set the function pointers
	this->OpenAudio = ANDROID_OPENSLES_OpenAudio;
	this->WaitAudio = ANDROID_OPENSLES_WaitAudio;
	this->PlayAudio = ANDROID_OPENSLES_PlayAudio;
	this->GetAudioBuf = ANDROID_OPENSLES_GetAudioBuf;
	this->CloseAudio = ANDROID_OPENSLES_CloseAudio;
	this->MuteAudio = ANDROID_OPENSLES_MuteAudio;

	this->ThreadInit = ANDROID_OPENSLES_ThreadInit;
	this->ThreadDeinit = ANDROID_OPENSLES_ThreadDeinit;

	this->free = ANDROID_OPENSLES_DeleteDevice;

	return this;
}

static void ANDROID_OPENSLES_DeleteDevice(SDL_AudioDevice *device)
{
	Debug_Printf("SDL_Audio OPENSLES Delete Device");

	SDL_free(device->hidden);
	SDL_free(device);
}

static int ANDROID_OPENSLES_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	Debug_Printf("SDL_Audio OPENSLES Open Audio");

	int res = createEngine(this);
	if (res < 0) {
		Debug_Printf("SDL_Audio OPENSLES Failed to createEngine");
		return -1;
	}

	res = createBufferQueueAudioPlayer(this, spec);
	if (res < 0) {
		Debug_Printf("SDL_Audio OPENSLES Failed to createBufferQueueAudioPlayer");

		// we're still in the main thread at this point (not the audio thread)
		JNIEnv *j_env = g_jniAudioInterface.android_env;
		if (j_env)
		{
			(*j_env)->CallVoidMethod(j_env, g_jniAudioInterface.android_mainActivity, g_jniAudioInterface.showAudioErrorDialog);
		}
		return -1;
	}

	playStream(this);

	return 0;
}

static void ANDROID_OPENSLES_CloseAudio(_THIS)
{
	Debug_Printf("SDL_Audio OPENSLES Close Audio");

	destroyBufferQueueAudioPlayer(this);

	shutdownEngine(this);
}

// This function waits until it is possible to write a full sound buffer
static void ANDROID_OPENSLES_WaitAudio(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

	//Debug_Printf("ANDROID_OPENSLES_WaitAudio");

	for (;;) {

		checkMuteStream(this);

		// wait for free buf
		if (myData->freeBufs <= 0) { // don't need lock here
			SDL_Delay(8);
		} else {
			break;
		}
	}

	checkMuteStream(this);
	if (g_audioMuteFrames > 0) {
		--g_audioMuteFrames;
	}
}

static void ANDROID_OPENSLES_PlayAudio(_THIS)
{
	//Debug_Printf("ANDROID_OPENSLES_PlayAudio");

	struct SDL_PrivateAudioData *myData = this->hidden;

	Uint8* buf = myData->audioBufs[myData->nextBuf];

    // enqueue another buffer
    SLresult result = (*(myData->bqPlayerBufferQueue))->Enqueue(myData->bqPlayerBufferQueue, buf, myData->bufSizeBytes);
    if (result != SL_RESULT_SUCCESS) {
		// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
		// warn, there should be enough buffers
		Debug_Printf("failed to enqueue audio buffer: result(%d)", result);
    }
    //else
    { // always decrement buf even in error case (which shouldn't occur) as it's better for audio to drop out rather than audio thread maxing out cpu
		SDL_mutexP(myData->freeBufLock);
		{
	        --myData->freeBufs;
	        if (myData->freeBufs < 0) {
		        myData->freeBufs = 0;
	        }
	        myData->nextBuf = (myData->nextBuf+1) % myData->numBufs;
        }
		SDL_mutexV(myData->freeBufLock);
    }
}

static Uint8 *ANDROID_OPENSLES_GetAudioBuf(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;
	return myData->audioBufs[myData->nextBuf];
}

//-------------------------------------------------------------------

// create the engine and output mix objects
static int createEngine(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

    SLresult result;

    // create engine
    result = slCreateEngine(&myData->engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // realize the engine
    result = (*(myData->engineObject))->Realize(myData->engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*(myData->engineObject))->GetInterface(myData->engineObject, SL_IID_ENGINE, &myData->engineEngine);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    //const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    //const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*(myData->engineEngine))->CreateOutputMix(myData->engineEngine, &myData->outputMixObject, 0, NULL, NULL);//1, ids, req);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // realize the output mix
    result = (*(myData->outputMixObject))->Realize(myData->outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    return 0;
}

// shut down the native audio system
static void shutdownEngine(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

    // destroy output mix object, and invalidate all associated interfaces
    if (myData->outputMixObject != NULL) {
        (*(myData->outputMixObject))->Destroy(myData->outputMixObject);
        myData->outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (myData->engineObject != NULL) {
        (*(myData->engineObject))->Destroy(myData->engineObject);
        myData->engineObject = NULL;
        myData->engineEngine = NULL;
    }
}

static void destroyBufferQueueAudioPlayer(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (myData->bqPlayerObject != NULL) {
        (*(myData->bqPlayerObject))->Destroy(myData->bqPlayerObject);
        myData->bqPlayerObject = NULL;
        myData->bqPlayerPlay = NULL;
        myData->bqPlayerBufferQueue = NULL;
        //bqPlayerEffectSend = NULL;
        myData->bqPlayerMuteSolo = NULL;
        //myData->bqPlayerVolume = NULL;
    }

    // destroy our buffers
	{
		if (myData->audioBufs != NULL)
		{
			int i;
			for (i = 0; i < myData->numBufs; ++i) {
				if (myData->audioBufs[i] != NULL) {
					SDL_FreeAudioMem(myData->audioBufs[i]);
					myData->audioBufs[i] = NULL;
				}
			}

			SDL_free(myData->audioBufs);
			myData->audioBufs = NULL;
		}

		myData->numBufs = 0;
		myData->freeBufs = 0;
		myData->nextBuf = 0;

		myData->bufSizeBytes = 0;
	}

	if ( myData->freeBufLock != NULL ) {
		SDL_DestroyMutex(myData->freeBufLock);
		myData->freeBufLock = NULL;
	}
}

// create buffer queue audio player
static int createBufferQueueAudioPlayer(_THIS, SDL_AudioSpec *spec)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

    SLresult result;

	int reqFormat   = spec->format;
	int reqFreq     = spec->freq;
	int reqChannels = spec->channels;
	int reqBufSize  = spec->size;
    int numBufs     = 2;

	if (reqFormat != AUDIO_S16SYS) { // we currently only support signed 16
		return -1;
	}
	int reqBits = 16;

	// we're still in the main thread at this point (not the audio thread)
	{
		jint minBufSize = 0;

		JNIEnv *j_env = g_jniAudioInterface.android_env;
		if (j_env)
		{
			minBufSize = (*j_env)->CallIntMethod(j_env,
					g_jniAudioInterface.android_mainActivity, g_jniAudioInterface.getMinBufSize,
					reqFreq, reqBits, reqChannels);
		}

		if (minBufSize <= 0) {
			// round up to power of 2
			int power2 = 1;
			while ( power2 < reqBufSize )
	            power2 *= 2;
	        reqBufSize = power2;
		} else {
		    if (reqBufSize < minBufSize) {
			    reqBufSize = minBufSize;
		    } else {
			    reqBufSize = ((reqBufSize + (minBufSize-1)) / minBufSize) * minBufSize;
		    }
	    }

		spec->fillSize = reqBufSize;
		{
			int samplesPerFrame = spec->userdata; // hack data passing
			int bytesPerFrame = samplesPerFrame * (reqChannels * (reqBits/8));
			spec->userdata = 0;

			if (bytesPerFrame > 0) {

				if (minBufSize <= 0) {

					int power2 = 1;
					while ( power2 < bytesPerFrame )
			            power2 *= 2;

			        if (power2 < reqBufSize) {
			            spec->fillSize = power2;
			        }

				} else {
					spec->fillSize = ( (bytesPerFrame + (minBufSize-1)) / minBufSize ) * minBufSize;
			    }

	            numBufs = (reqBufSize + (spec->fillSize-1)) / spec->fillSize;
	            numBufs = (numBufs <= 1) ? 2 : numBufs;
	            reqBufSize = numBufs * spec->fillSize;
			}
		}

		Debug_Printf("Audio req buf size: %d, min: %d, actual: %d, fill: %d, numbufs: %d", spec->size, minBufSize, reqBufSize, spec->fillSize, numBufs);

		// update audio spec
		spec->samples = reqBufSize / (reqChannels * (reqBits/8));
		spec->size = reqBufSize;
	}

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, numBufs};
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        reqChannels,
        reqFreq*1000,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, myData->outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, /*SL_IID_VOLUME,*/ SL_IID_MUTESOLO }; //, SL_IID_EFFECTSEND };
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_FALSE }; //, SL_BOOLEAN_TRUE}, };

    result = (*(myData->engineEngine))->CreateAudioPlayer(myData->engineEngine, &myData->bqPlayerObject, &audioSrc, &audioSnk, 2, ids, req);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // realize the player
    result = (*(myData->bqPlayerObject))->Realize(myData->bqPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // get the play interface
    result = (*(myData->bqPlayerObject))->GetInterface(myData->bqPlayerObject, SL_IID_PLAY, &myData->bqPlayerPlay);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // get the buffer queue interface
    result = (*(myData->bqPlayerObject))->GetInterface(myData->bqPlayerObject, SL_IID_BUFFERQUEUE, &myData->bqPlayerBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // register callback on the buffer queue
    result = (*(myData->bqPlayerBufferQueue))->RegisterCallback(myData->bqPlayerBufferQueue, bqPlayerCallback, myData);
    if (result != SL_RESULT_SUCCESS) {
	    return -1;
    }

    // get the mute/solo interface
    result = (*(myData->bqPlayerObject))->GetInterface(myData->bqPlayerObject, SL_IID_MUTESOLO, &myData->bqPlayerMuteSolo);
    if (result != SL_RESULT_SUCCESS) {
	    myData->bqPlayerMuteSolo = NULL; // don't fail, but don't support muting
    }

    // get the volume interface
    //result = (*(myData->bqPlayerObject))->GetInterface(myData->bqPlayerObject, SL_IID_VOLUME, &myData->bqPlayerVolume);
    //if (result != SL_RESULT_SUCCESS) {
	//    return -1;
    //}

    //result = (*(myData->bqPlayerVolume))->GetVolumeLevel(myData->bqPlayerVolume, &myData->defVolLevel);
    //if (result != SL_RESULT_SUCCESS) {
    //    return -1;
    //}

	// create our buffers
	{
		int i;

		myData->numBufs = numBufs;
		myData->freeBufs = numBufs;
		myData->nextBuf = 0;

		myData->bufSizeBytes = spec->fillSize;//reqBufSize;
		myData->audioBufs = (Uint8 **) SDL_malloc(sizeof(Uint8 *) * myData->numBufs);
		if (myData->audioBufs == NULL) {
			SDL_OutOfMemory();
			return -1;
		}
		for (i = 0; i < myData->numBufs; ++i) {
			myData->audioBufs[i] = SDL_AllocAudioMem(myData->bufSizeBytes);
			if (myData->audioBufs[i] == NULL) {
				SDL_OutOfMemory();
				return -1;
			}
			SDL_memset(myData->audioBufs[i], spec->silence, myData->bufSizeBytes);
		}
	}

	myData->freeBufLock = SDL_CreateMutex();
	if ( myData->freeBufLock == NULL ) {
		SDL_SetError("Couldn't create mixer lock");
		return -1;
	}

	return 0;
}

// this callback handler is called every time a buffer finishes playing
static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	struct SDL_PrivateAudioData *myData = (struct SDL_PrivateAudioData *)context;

	//Debug_Printf("SDL_Audio OPENSLES bqPlayerCallback");

	SDL_mutexP(myData->freeBufLock);
	{
		++myData->freeBufs;
		if (myData->freeBufs > myData->numBufs) {
			// something went wrong...
			myData->freeBufs = myData->numBufs;
		}
	}
	SDL_mutexV(myData->freeBufLock);
}

static void playStream(_THIS)
{
	struct SDL_PrivateAudioData *myData = this->hidden;

	if (myData->bqPlayerPlay != NULL) {
	    SLresult result = (*(myData->bqPlayerPlay))->SetPlayState(myData->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	    assert(SL_RESULT_SUCCESS == result);
    }
}

static void muteStream(_THIS, SLboolean mute)
{
	int reqMute = mute ? 1 : 0;
	if (reqMute != this->muted)
	{
		struct SDL_PrivateAudioData *myData = this->hidden;

		Debug_Printf("SDL_Audio OPENSLES Mute Audio: %d", mute);

		if (myData->bqPlayerMuteSolo != NULL) {
			const int kNumChans = 2; // TODO: use the one we passed in the spec(for now we just support 2 channels)
			int chan = 0;
			for (; chan < kNumChans; ++chan) {
			    SLresult result = (*(myData->bqPlayerMuteSolo))->SetChannelMute(myData->bqPlayerMuteSolo, chan, mute);
			    //assert(SL_RESULT_SUCCESS == result);
		    }
	    }

		{
		    //SLresult result = (*(myData->bqPlayerVolume))->SetVolumeLevel(myData->bqPlayerVolume, mute ? (-100*100) : myData->defVolLevel);
		    //assert(SL_RESULT_SUCCESS == result);
	    }

	    this->muted = reqMute;
    }
}

static void checkMuteStream(_THIS)
{
	int reqMute = (g_audioMute || g_audioMuteFrames > 0) ? 1 : 0;

	if (reqMute != this->muted)
	{
		muteStream(this, reqMute);
	}
}

static void ANDROID_OPENSLES_MuteAudio(_THIS, int mute)
{
	int reqMute = (mute != 0) ? 1 : 0;
	if (reqMute != this->muted)
	{
		muteStream(this, reqMute);
	}
}
