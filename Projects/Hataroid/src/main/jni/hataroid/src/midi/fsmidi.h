#ifndef __FSMIDI_H_
#define __FSMIDI_H_

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	MIDI_DEVICE_NONE		= 0,
	MIDI_DEVICE_ANDROID		= 1,
	MIDI_DEVICE_FLUIDSYNTH	= 2,
};

#define FSMIDI_DEFAULT_SAMPLE_RATE	44100
#define FSMIDI_DEFAULT_POLYPHONY	64
#define FSMIDI_DEFAULT_INTERP		1
#define FSMIDI_DEFAULT_VOLGAIN		50

#define FSMIDI_NUM_CHANNELS			16

typedef struct
{
	unsigned char program[FSMIDI_NUM_CHANNELS];
	unsigned char pressure[FSMIDI_NUM_CHANNELS];
	unsigned char vol[FSMIDI_NUM_CHANNELS];

} FSMIDI_SAVEDATA;

extern void fsMidi_init();
extern void fsMidi_deinit();
extern void fsMidi_update();
extern void fsMidi_postUpdate();
extern void fsMidi_reset(int resetSave);

extern void fsMidi_setMidiOutEnabled(int enable);
extern void fsMidi_setDevice(int device);
extern void fsMidi_setSoundFont(const char *sfile);
extern void fsMidi_setSetting(int sampleRate, int reverb, int chorus, int interp, int maxPoly);
extern void fsMidi_setSettingVolGain(int volGain);
extern void fsMidi_setIgnoreProgramChanges(int ignore);
extern void fsMidi_setMidiTranspose(int transpose);

extern void fsMidi_writeByte(unsigned char b);

extern const FSMIDI_SAVEDATA* fsMidi_getSaveData();
extern void fsMidi_setSaveData(FSMIDI_SAVEDATA* saveData);
extern void fsMidi_resetSaveData(FSMIDI_SAVEDATA* saveData);

extern int fsMidi_isMidiOutEnabled();
extern int fsMidi_getMidiOutDevice();
extern const char* fsMidi_getFSynthSoundFont();
extern int fsMidi_getFSynthReverb();
extern int fsMidi_getFSynthChorus();
extern int fsMidi_getFSynthInterp();
extern int fsMidi_getFSynthMaxPoly();
extern int fsMidi_getFSynthVolGain();
extern int fsMidi_getIgnoreProgramChanges();
extern int fsMidi_getMidiTranspose();

#ifdef __cplusplus
}
#endif

#endif //__FSMIDI_H_
