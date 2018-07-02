#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>

#include <fluidsynth.h>
#include "../../../fluidsynth/src/drivers/fluid_android.h"

#include "fsmidi.h"
//#include "andmidi.h"

#include "../hataroid.h"

extern struct JNIMainMethodCache g_jniMainInterface;

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSendMidiInstrPatch(JNIEnv * env, jobject obj, jintArray instrDefs);
JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorReceiveHardwareMidiBytes(JNIEnv * env, jobject obj, jint count, jbyteArray b);

void fsMidi_sendHardwareMidiByte(unsigned char b);
void fsMidi_writeHardwareMidiEvent(int evt, int chan, int param1, int param2);

#define MIDIEVENT_SYSTEM_RESET      0xff
#define MIDIEVENT_PROGRAM_CHANGE    0xc0
#define MIDIEVENT_CHANNEL_PRESSURE  0xd0
#define MIDIEVENT_CONTROL_CHANGE    0xb0

#define FSMIDI_MAX_INTERP_TYPES	4
static const int s_fsMidiInterpTypes[FSMIDI_MAX_INTERP_TYPES] =
{
	FLUID_INTERP_NONE,// = 0,        /**< No interpolation: Fastest, but questionable audio quality */
	FLUID_INTERP_LINEAR,// = 1,      /**< Straight-line interpolation: A bit slower, reasonable audio quality */
	FLUID_INTERP_4THORDER,// = 4,    /**< Fourth-order interpolation, good quality, the default */
	FLUID_INTERP_7THORDER,// = 7     /**< Seventh-order interpolation */
};

int _fsMidiOutEnabled = 0;
int _fsMidiOutDevice = MIDI_DEVICE_NONE;

int _fsMidiHardwareOutEnabled = 0;
int _fsMidiHardwareInEnabled = 0;

static fluid_settings_t* s_fsSettings = 0;
static fluid_synth_t* s_fsSynth = 0;
static fluid_audio_driver_t* s_fsADriver = 0;
static fluid_player_t* s_fsPlayer = 0;
static int s_fsSoundFontID = -1;
static void* s_fsMidiParser = 0;

static char _curSoundFontName[FILENAME_MAX] = "";
static int _curSampleRate = 0;
static int _curReverb = 0;
static int _curChorus = 0;
static int _curInterp = 0;
static int _curMaxPoly = 0;
static int _curVolGain = 0;
static int _ignorePgmChanges = 0;

static int _pendingDeviceReset = 0;
static int _pendingSaveStateApplyFSynth = 0;
static int _pendingSaveStateApplyHardware = 0;
static int _pendingSettingChange = 0;
static char _pendingSoundFontName[FILENAME_MAX] = "";
static int _pendingSampleRate = 0;
static int _pendingReverb = -1;
static int _pendingChorus = -1;
static int _pendingInterp = -1;
static int _pendingMaxPoly = 0;
static int _pendingVolGain = -1;

static int _transpose = 0;
static int _drumChannel = 9; // 0 offset

static FSMIDI_SAVEDATA _fsMidi_saveData;

#define kHardwareInRecvBufSize 512
static unsigned char* _hRecvBuf = 0; // TODO: save in memory snapshot?
static volatile int _hRecvBufReadIdx = 0;
static volatile int _hRecvBufWriteIdx = 0;
static volatile int _hRecvBufCount = 0;
static volatile int _hRecvBufLock = 0;

int fsMidi_isMidiOutEnabled() { return _fsMidiOutEnabled; }
int fsMidi_getMidiOutDevice() { return _fsMidiOutDevice; }
const char* fsMidi_getFSynthSoundFont() { return _curSoundFontName; }
int fsMidi_getFSynthReverb() { return _curReverb; }
int fsMidi_getFSynthChorus() { return _curChorus; }
int fsMidi_getFSynthInterp() { return _curInterp; }
int fsMidi_getFSynthMaxPoly() { return _curMaxPoly; }
int fsMidi_getFSynthVolGain() { return _curVolGain; }

int fsMidi_isMidiHardwareOutEnabled() { return _fsMidiHardwareOutEnabled; }
int fsMidi_isMidiHardwareInEnabled() { return _fsMidiHardwareInEnabled; }

const FSMIDI_SAVEDATA* fsMidi_getSaveData() { return &_fsMidi_saveData; }
void fsMidi_setSaveData(FSMIDI_SAVEDATA* saveData)
{
	memcpy(&_fsMidi_saveData, saveData, sizeof(FSMIDI_SAVEDATA));
	_pendingSaveStateApplyFSynth = 1;
	_pendingSaveStateApplyHardware = 1;
}
void fsMidi_resetSaveData(FSMIDI_SAVEDATA* saveData)
{
	memset(saveData, 0, sizeof(FSMIDI_SAVEDATA));
}

void fsMidi_init()
{
	fsMidi_deinit();

	fsMidi_resetSaveData(&_fsMidi_saveData);

    Debug_Printf("----> alloc hardware midi buffers");
    {
        if (_hRecvBuf == 0)
        {
            _hRecvBuf = (unsigned char *) malloc(kHardwareInRecvBufSize * sizeof(unsigned char));
        }
        _hRecvBufReadIdx = 0;
        _hRecvBufWriteIdx = 0;
        _hRecvBufCount = 0;
        _hRecvBufLock = 0;
    }

	Debug_Printf("----> fsSynth - new Settings");
	{
		s_fsSettings = new_fluid_settings();
		fluid_settings_setstr(s_fsSettings, "player.timing-source", "sample");
		fluid_settings_setint(s_fsSettings, "synth.parallel-render", 1); // TODO: Fast_render should not need this, but currently do

		//fluid_settings_setint(s_fsSettings, "audio.period-size", 64);
		//fluid_settings_setint(s_fsSettings, "audio.periods", 16);
		fluid_settings_setnum(s_fsSettings, "synth.sample-rate", FSMIDI_DEFAULT_SAMPLE_RATE);
	    fluid_settings_setint(s_fsSettings, "synth.polyphony", FSMIDI_DEFAULT_POLYPHONY);

		fluid_settings_setint(s_fsSettings, "synth.reverb.active", 0);
		fluid_settings_setint(s_fsSettings, "synth.chorus.active", 0);

		fluid_settings_setnum(s_fsSettings, "synth.gain", FSMIDI_DEFAULT_VOLGAIN/100.0f);

		_curSampleRate = FSMIDI_DEFAULT_SAMPLE_RATE;
		_curMaxPoly = FSMIDI_DEFAULT_POLYPHONY;
		_curVolGain = FSMIDI_DEFAULT_VOLGAIN;
	}

	Debug_Printf("----> fsSynth - new Synth");
	{
		s_fsSynth = new_fluid_synth(s_fsSettings);

		fluid_synth_set_interp_method(s_fsSynth, -1, s_fsMidiInterpTypes[FSMIDI_DEFAULT_INTERP]);

		_curInterp = FSMIDI_DEFAULT_POLYPHONY;
	}

	Debug_Printf("----> fsSynth - new Audio Driver");
	{
		fluid_settings_setstr(s_fsSettings, "audio.driver", "android");
		//fluid_settings_setstr(s_fsSettings, "audio.file.name", "/sdcard/fstest.raw");

		s_fsADriver = new_fluid_audio_driver(s_fsSettings, s_fsSynth);
	}

	Debug_Printf("----> fsSynth - midi parser");
	{
		s_fsMidiParser = fluid_android_create_midi_parser();
		if (!s_fsMidiParser) {
			Debug_Printf("----> fsSynth - Failed to create midi parser");
		}
	}

	fsMidi_setDevice(MIDI_DEVICE_FLUIDSYNTH);

/*
	Debug_Printf("----> fsSynth - new Player");
	s_fsPlayer = new_fluid_player(s_fsSynth);

	const char *sTestMidi = "/sdcard/User/midi/impromptu.mid";
	if (fluid_is_midifile(sTestMidi)) {
		Debug_Printf("----> fsSynth - Load Midi");
		fluid_player_add(s_fsPlayer, sTestMidi);
	}

	Debug_Printf("----> fsSynth - Play Midi");
	// play the midi files, if any
	fluid_player_play(s_fsPlayer);

	//Debug_Printf("----> fsSynth - Wait Midi");
	// wait for playback termination
	//fluid_player_join(s_fsPlayer);
*/

	Debug_Printf("----> fsSynth - Midi Init Finished");
}

void fsMidi_deinit()
{
	if (s_fsSoundFontID >= 0)
	{
		if (s_fsSynth)
		{
			Debug_Printf("----> fsSynth - unloading soundfont");
			fluid_synth_sfunload(s_fsSynth, s_fsSoundFontID, 1);
		}
		s_fsSoundFontID = -1;
	}
	Debug_Printf("----> fsSynth - destroying");
	if (s_fsMidiParser)	{ fluid_android_destroy_midi_parser(s_fsMidiParser);	s_fsMidiParser = 0; }
    if (s_fsPlayer)		{ delete_fluid_player(s_fsPlayer);						s_fsPlayer = 0; }
    if (s_fsADriver)	{ delete_fluid_audio_driver(s_fsADriver);				s_fsADriver = 0; }
    if (s_fsSynth)		{ delete_fluid_synth(s_fsSynth);						s_fsSynth = 0; }
	if (s_fsSettings)	{ delete_fluid_settings(s_fsSettings);					s_fsSettings = 0; }
	Debug_Printf("----> fsSynth - destroyed");

    Debug_Printf("----> freeing hardware midi buffers");
    {
        //if (_hRecvBuf != 0)
        //{
        //    free(_hRecvBuf);
        //    _hRecvBuf = 0;
        //}
        _hRecvBufReadIdx = 0;
        _hRecvBufWriteIdx = 0;
        _hRecvBufCount = 0;
        _hRecvBufLock = 0;
    }

	_curSoundFontName[0] = 0;
	_curSampleRate = 0;
	_curReverb = 0;
	_curChorus = 0;
	_curInterp = 0;
	_curMaxPoly = 0;
	_curVolGain = 0;

	_pendingDeviceReset = 0;
	_pendingSaveStateApplyFSynth = 0;
	_pendingSaveStateApplyHardware = 0;
	_pendingSettingChange = 0;
	_pendingSoundFontName[0] = 0;
	_pendingSampleRate = 0;
	_pendingReverb = -1;
	_pendingChorus = -1;
	_pendingInterp = -1;
	_pendingMaxPoly = 0;
	_pendingVolGain = -1;
}

void fsMidi_unloadCurSoundFont()
{
	if (s_fsSynth && s_fsSoundFontID >= 0)
	{
		fluid_synth_sfunload(s_fsSynth, s_fsSoundFontID, 1);
		s_fsSoundFontID = -1;
		_curSoundFontName[0] = 0;
	}
}

void fsMidi_applyPendingChanges()
{
	int allDone = 1;

	if (!_pendingSettingChange)
	{
		return;
	}

	if (_pendingSoundFontName[0] != 0)
	{
		if (strcmp(_pendingSoundFontName, _curSoundFontName) != 0)
		{
			if (s_fsSynth)
			{
				if (s_fsSoundFontID >= 0)
				{
					fluid_synth_sfunload(s_fsSynth, s_fsSoundFontID, 1);
					s_fsSoundFontID = -1;
					_curSoundFontName[0] = 0;
				}

				Debug_Printf("----> fsSynth - Load SoundFont");
				int dialogID = showGenericDialog(0, "Loading MIDI SoundFont...", 0, 0, "", "");

				if (strncmp(_pendingSoundFontName, "asset://", 8) == 0 || fluid_is_soundfont(_pendingSoundFontName))
				{
					s_fsSoundFontID = fluid_synth_sfload(s_fsSynth, _pendingSoundFontName, 1);
					if (s_fsSoundFontID >= 0)
					{
						Debug_Printf("----> fsSynth - Loaded %s (%d)", _pendingSoundFontName, s_fsSoundFontID);
					}
					else
					{
						Debug_Printf("----> fsSynth - Failed to load %s (%d)", _pendingSoundFontName, s_fsSoundFontID);
					}
				}
				else
				{
					Debug_Printf("----> fsSynth - file not a sound font");
				}

				destroyGenericDialog(0, dialogID);

				strcpy(_curSoundFontName, _pendingSoundFontName);
				_pendingSoundFontName[0] = 0;
			}
			else { allDone = 0; }
		}
		else { _pendingSoundFontName[0] = 0; }
	}

	if (_pendingSampleRate > 0)
	{
		if (_curSampleRate != _pendingSampleRate)
		{
			if (s_fsSettings && s_fsSynth)
			{
				fluid_settings_setnum(s_fsSettings, "synth.sample-rate", _pendingSampleRate);
				_curSampleRate = _pendingSampleRate;
				_pendingSampleRate = 0;
			}
			else { allDone = 0; }
		}
		else { _pendingSampleRate = 0; }
	}

	if (_pendingReverb >= 0)
	{
		if (_curReverb != _pendingReverb)
		{
			if (s_fsSynth)
			{
				fluid_synth_set_reverb_on(s_fsSynth, _pendingReverb);
				_curReverb = _pendingReverb;
				_pendingReverb = -1;
			}
			else { allDone = 0; }
		}
		else { _pendingReverb = -1; }
	}

	if (_pendingChorus >= 0)
	{
		if (_curChorus != _pendingChorus)
		{
			if (s_fsSynth)
			{
				fluid_synth_set_chorus_on(s_fsSynth, _pendingChorus);
				_curChorus = _pendingChorus;
				_pendingChorus = -1;
			}
			else { allDone = 0; }
		}
		else { _pendingChorus = -1; }
	}

	if (_pendingInterp >= 0)
	{
		if (_curInterp != _pendingInterp)
		{
			if (s_fsSynth)
			{
				fluid_synth_set_interp_method(s_fsSynth, -1, s_fsMidiInterpTypes[_pendingInterp]);
				_curInterp = _pendingInterp;
				_pendingInterp = -1;
			}
			else { allDone = 0; }
		}
		else { _pendingInterp = -1; }
	}

	if (_pendingMaxPoly > 0)
	{
		if (_curMaxPoly != _pendingMaxPoly)
		{
			if (s_fsSettings && s_fsSynth)
			{
			    fluid_settings_setint(s_fsSettings, "synth.polyphony", _pendingMaxPoly);
				_curMaxPoly = _pendingMaxPoly;
				_pendingMaxPoly = 0;
			}
			else { allDone = 0; }
		}
		else { _pendingMaxPoly = 0; }
	}

	if (_pendingVolGain >= 0)
	{
		if (_curVolGain != _pendingVolGain)
		{
			if (s_fsSettings && s_fsSynth)
			{
			    fluid_settings_setnum(s_fsSettings, "synth.gain", _pendingVolGain/100.0f);
			    _curVolGain = _pendingVolGain;
			    _pendingVolGain = -1;
			}
			else { allDone = 0; }
		}
		else { _pendingVolGain = -1; }
	}

	if (allDone)
	{
		_pendingSettingChange = 0;
	}
}

void fsMidi_applySaveState()
{
	if (_fsMidiOutEnabled)
	{
		if (_fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
		{
			if (_pendingSaveStateApplyFSynth && s_fsSynth && !_fsMidiHardwareOutEnabled)
			{
				int chan = 0;
				for (; chan < FSMIDI_NUM_CHANNELS; ++chan)
				{
					int pgm = _fsMidi_saveData.program[chan];
					if (pgm > 0 && pgm <= 128) { fluid_synth_program_change(s_fsSynth, chan, pgm-1); }

					int pressure = _fsMidi_saveData.pressure[chan];
					if (pressure > 0 && pressure <= 128) { fluid_synth_channel_pressure(s_fsSynth, chan, pressure-1); }

					int vol = _fsMidi_saveData.vol[chan];
					if (vol > 0 && vol <= 128) { fluid_synth_cc(s_fsSynth, chan, 0x7, vol-1); }
				}

				_pendingSaveStateApplyFSynth = 0;
			}
		}

        if (_fsMidiHardwareOutEnabled && _pendingSaveStateApplyHardware)
        {
            int chan = 0;
            for (; chan < FSMIDI_NUM_CHANNELS; ++chan)
            {
                int pgm = _fsMidi_saveData.program[chan];
                if (pgm > 0 && pgm <= 128) { fsMidi_writeHardwareMidiEvent(MIDIEVENT_PROGRAM_CHANGE, chan, pgm-1, 0); }

                int pressure = _fsMidi_saveData.pressure[chan];
                if (pressure > 0 && pressure <= 128) { fsMidi_writeHardwareMidiEvent(MIDIEVENT_CHANNEL_PRESSURE, chan, pressure-1, 0); }

                int vol = _fsMidi_saveData.vol[chan];
                if (vol > 0 && vol <= 128) { fsMidi_writeHardwareMidiEvent(MIDIEVENT_CONTROL_CHANGE, chan, 0x7, vol-1); }
            }

            _pendingSaveStateApplyHardware = 0;
        }
    }
}

void fsMidi_postUpdate()
{
	//if (_fsMidiOutEnabled && _fsMidiOutDevice == MIDI_DEVICE_ANDROID)
	//{
	//	_sendAndMidiAudio();
	//}
}

void fsMidi_update()
{
	if (_fsMidiOutEnabled)
	{
		if (_fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
		{
			if (_pendingDeviceReset)
			{
				if (s_fsSynth)
				{
					fsMidi_reset(0);
					_pendingDeviceReset = 0;
				}
            }
			if (_pendingSettingChange)
			{
				fsMidi_applyPendingChanges();
			}
			if (_pendingSaveStateApplyFSynth)
			{
				fsMidi_applySaveState();
			}
		}

        if (_pendingSaveStateApplyHardware)
        {
            fsMidi_applySaveState();
        }
    }
}

void fsMidi_reset(int resetSave)
{
	if (_fsMidiOutEnabled)
	{
		if (_fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
		{
			if (s_fsSynth)
			{
				fluid_synth_system_reset(s_fsSynth);
			}
		}

        if (_fsMidiHardwareOutEnabled)
        {
            fsMidi_writeHardwareMidiEvent(MIDIEVENT_SYSTEM_RESET, 0, 0, 0);
        }
    }

	if (resetSave)
	{
		fsMidi_resetSaveData(&_fsMidi_saveData);
	}

	_pendingSaveStateApplyFSynth = 1;
	_pendingSaveStateApplyHardware = 1;
}

void fsMidi_setMidiOutEnabled(int enable)
{
	_fsMidiOutEnabled = enable;
}

void fsMidi_setMidiHardwareOutEnabled(int enable)
{
    if (_fsMidiHardwareOutEnabled != enable)
    {
        _pendingDeviceReset = 1;
        _pendingSaveStateApplyFSynth = 1;
        _pendingSaveStateApplyHardware = 1;

        _fsMidiHardwareOutEnabled = enable;
    }
}

void fsMidi_setMidiHardwareInEnabled(int enable)
{
    _fsMidiHardwareInEnabled = enable;
}

void fsMidi_setDevice(int device)
{
	if (_fsMidiOutDevice != device)
	{
		_fsMidiOutDevice = device;

		_pendingDeviceReset = 1;
        _pendingSaveStateApplyFSynth = 1;
        _pendingSaveStateApplyHardware = 1;
    }
}

void fsMidi_setSoundFont(const char *sfile)
{
	strcpy(_pendingSoundFontName, sfile);
	_pendingSettingChange = 1;
}

void fsMidi_setSettingVolGain(int volGain)
{
	if (volGain >= 0)
	{
		_pendingVolGain = volGain;
		_pendingSettingChange = 1;
	}
}

void fsMidi_setIgnoreProgramChanges(int ignore)
{
	_ignorePgmChanges = ignore;
}

void fsMidi_setMidiTranspose(int transpose)
{
    _transpose = transpose;
}

int fsMidi_getIgnoreProgramChanges()
{
	return _ignorePgmChanges;
}

int fsMidi_getMidiTranspose()
{
	return _transpose;
}

void fsMidi_setSetting(int sampleRate, int reverb, int chorus, int interp, int maxPoly)
{
	if (sampleRate > 0)
	{
		_pendingSampleRate = sampleRate;
		_pendingSettingChange = 1;
	}
	if (reverb >= 0)
	{
		_pendingReverb = reverb;
		_pendingSettingChange = 1;
	}
	if (chorus >= 0)
	{
		_pendingChorus = chorus;
		_pendingSettingChange = 1;
	}
	if (interp >= 0)
	{
		if (interp >= FSMIDI_MAX_INTERP_TYPES) { interp = FSMIDI_MAX_INTERP_TYPES-1; }

		_pendingInterp = interp;
		_pendingSettingChange = 1;
	}
	if (maxPoly > 0)
	{
		_pendingMaxPoly = maxPoly;
		_pendingSettingChange = 1;
	}
}

void fsMidi_generateFrames(int nframes)
{
	if (_fsMidiOutEnabled && _fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
	{
		if (s_fsADriver)
		{
			fluid_android_process_frames(s_fsADriver, nframes);
		}
	}
}

short *fsMidi_consumeBuffer(int bufLenReqShorts, int *startOffsetShorts, int *readLenShorts, int *bufMaxShorts)
{
	if (_fsMidiOutEnabled && _fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
	{
		return fluid_android_consume_buffer(s_fsADriver, bufLenReqShorts, startOffsetShorts, readLenShorts, bufMaxShorts);
	}

	(*readLenShorts) = 0;
	return 0;
}

void fsMidi_writeByte(unsigned char b)
{
    //Debug_Printf("midi out: %x", b);

    if (_fsMidiHardwareOutEnabled)
    {
        // TODO: buffer these up before sending to hardware device
        // TODO: support transposing
        fsMidi_sendHardwareMidiByte(b);
	}

	if (s_fsMidiParser)
	{
		fluid_midi_event_t* evt = fluid_android_parse_midi_byte(s_fsMidiParser, b);
		if (evt)
		{
			int dirty = 0;
			switch (fluid_midi_event_get_type(evt))
			{
                case 0x90://NOTE_ON:
                    if (_fsMidiHardwareOutEnabled) { return; }

                    if (_transpose != 0)
                    {
    					int channel = fluid_midi_event_get_channel(evt);
    					if (channel != _drumChannel)
    					{
                            int k = fluid_midi_event_get_key(evt);
                            k += _transpose;
                            k = (k < 0) ? 0 : (k > 127) ? 127 : k;
                            fluid_midi_event_set_key(evt, k);
                        }
                    }
                    break;

                case 0x80://NOTE_OFF:
                    if (_fsMidiHardwareOutEnabled) { return; }

                    if (_transpose != 0)
                    {
    					int channel = fluid_midi_event_get_channel(evt);
    					if (channel != _drumChannel)
    					{
                            int k = fluid_midi_event_get_key(evt);
                            k += _transpose;
                            k = (k < 0) ? 0 : (k > 127) ? 127 : k;
                            fluid_midi_event_set_key(evt, k);
                        }
                    }
                    break;

				case 0xc0://PROGRAM_CHANGE:
				{
					if (_ignorePgmChanges)
					{
						return;
					}

					int channel = fluid_midi_event_get_channel(evt);
					int pgm = fluid_midi_event_get_program(evt);
					if (channel >= 0 && channel < FSMIDI_NUM_CHANNELS
					 && pgm >= 0 && pgm <= 127)
					{
						_fsMidi_saveData.program[channel] = (unsigned char)(pgm+1);
						//Debug_Printf("----> fsSynth - chan: %d, program change: %d", channel, pgm);
						dirty = 1;
					}
					break;
				}
				case 0xd0://CHANNEL_PRESSURE:
				{
					int channel = fluid_midi_event_get_channel(evt);
					int pressure = fluid_midi_event_get_program(evt);
					if (channel >= 0 && channel < FSMIDI_NUM_CHANNELS
					 && pressure >= 0 && pressure <= 127)
					{
						_fsMidi_saveData.pressure[channel] = (unsigned char)(pressure+1);
						dirty = 1;
					}
					break;
				}
				case 0xb0://CONTROL_CHANGE:
				{
					int channel = fluid_midi_event_get_channel(evt);
					if (channel >= 0 && channel < FSMIDI_NUM_CHANNELS)
					{
						int control = fluid_midi_event_get_control(evt);
						int val = fluid_midi_event_get_value(evt);
						if (control == 0x7)
						{
							if (val >= 0 && val <= 127)
							{
								_fsMidi_saveData.vol[channel] = (unsigned char)(val+1);
								dirty = 1;
							}
						}
						else if (control == 0x79) // Reset All Controllers (program and vol aren't reset according to specs)
						{
							_fsMidi_saveData.pressure[channel] = 0;
							dirty = 1;
						}
						/*
						else if (control == 0x6)
						{
							Debug_Printf("----> fsSynth CZ-101 Tune: %x", val);
						}
						*/
					}
					break;
				}
				case 0xff://MIDI_SYSTEM_RESET:
				{
					//Debug_Printf("----> fsSynth - midi reset");
					fsMidi_resetSaveData(&_fsMidi_saveData);
					dirty = 1;
					break;
				}
				/*
				case 0xf0://MIDI_SYSEX
				{
					unsigned char *data = ((unsigned char *)(evt))+4;
					int len = *((unsigned int*)(((unsigned char *)(evt))+12));
					char s[1024];
					s[0] = 0;
					int i = 0;
					for (;i < len; ++i)
					{
						sprintf(&s[strlen(s)], "%02x ", data[i]);
					}
					s[strlen(s)] = 0;
					Debug_Printf("----> fsSynth SYSEX: %s", s);
					break;
				}
				*/
				default:
				{
					break;
				}
			}

			if (_fsMidiOutEnabled)
			{
				if (_fsMidiOutDevice == MIDI_DEVICE_FLUIDSYNTH)
				{
					if (s_fsSynth && !_fsMidiHardwareOutEnabled)
					{
                        fluid_synth_handle_midi_event(s_fsSynth, evt);
					}
					else if (dirty)
					{
						_pendingSaveStateApplyFSynth = 1;
					}
				}
			}
			else if (dirty)
			{
				_pendingSaveStateApplyFSynth = 1;
			}

            if (dirty && !_fsMidiHardwareOutEnabled)
            {
                _pendingSaveStateApplyHardware = 1;
            }
        }

		//if (_fsMidiOutEnabled && _fsMidiOutDevice == MIDI_DEVICE_ANDROID)
		//{
		//	andMidiWriteByte(b);
		//}

	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSendMidiInstrPatch(JNIEnv * env, jobject obj, jintArray instrDefs)
{
	int numInstrs = (*env)->GetArrayLength(env, instrDefs);
	jint* instrVals = (*env)->GetIntArrayElements(env, instrDefs, 0);

	int numChans = (numInstrs > FSMIDI_NUM_CHANNELS) ? FSMIDI_NUM_CHANNELS : numInstrs;
	int chan = 0;
	for (chan = 0; chan < FSMIDI_NUM_CHANNELS; ++chan)
	{
		_fsMidi_saveData.program[chan] = 1;
	}
	for (chan = 0; chan < numChans; ++chan)
	{
		int instr = instrVals[chan];
		if (instr < 0 || instr > 127)
		{
			instr = 0;
		}
		_fsMidi_saveData.program[chan] = (unsigned char)(instr+1);
	}

	_pendingSaveStateApplyFSynth = 1;
	_pendingSaveStateApplyHardware = 1;

	(*env)->ReleaseIntArrayElements(env, instrDefs, instrVals, JNI_ABORT);
	//(*env)->DeleteLocalRef(env, instrDefs); // explicitly releasing to assist garbage collection, though not required
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorReceiveHardwareMidiBytes(JNIEnv * env, jobject obj, jint count, jbyteArray b)
{
    if (_fsMidiHardwareInEnabled == 0)
    {
        return;
    }

	jbyte* data = (*env)->GetByteArrayElements(env, b, 0);
	int i;

	while (_hRecvBufLock) { usleep(2); }
    _hRecvBufLock = 1; // TODO: proper atomic lock

	for (i = 0; i < count; ++i)
	{
		if (_hRecvBufCount >= kHardwareInRecvBufSize)
		{
			break;
		}
        _hRecvBuf[_hRecvBufWriteIdx] = data[i];
        _hRecvBufWriteIdx = (_hRecvBufWriteIdx+1) % kHardwareInRecvBufSize; // TODO: optimize
		++_hRecvBufCount;
	}

    _hRecvBufLock = 0;

	(*env)->ReleaseByteArrayElements(env, b, data, JNI_ABORT);
	//(*env)->DeleteLocalRef(env, b); // explicitly releasing to assist garbage collection, though not required
}

int fsMidi_hasRecvData()
{
	return (_hRecvBufCount > 0) ? 1 : 0;
}

unsigned char fsMidi_readRecvData()
{
	if (_hRecvBufCount > 0)
	{
		unsigned char b = _hRecvBuf[_hRecvBufReadIdx];
        _hRecvBufReadIdx = (_hRecvBufReadIdx+1) % kHardwareInRecvBufSize; // TODO: optimize

		while (_hRecvBufLock) { usleep(2); }
        _hRecvBufLock = 1; // TODO: proper atomic lock
		{
			--_hRecvBufCount;
		}
        _hRecvBufLock = 0;

		return b;
	}
	return 0;
}

void fsMidi_writeHardwareMidiEvent(int evt, int chan, int param1, int param2)
{
    if (_fsMidiHardwareOutEnabled)
    {
        switch (evt)
        {
            case MIDIEVENT_SYSTEM_RESET: // 0xff
            {
                fsMidi_sendHardwareMidiByte(MIDIEVENT_SYSTEM_RESET);
                break;
            }
            case MIDIEVENT_PROGRAM_CHANGE: //0xc0
            {
                unsigned char b1 = (unsigned char)(MIDIEVENT_PROGRAM_CHANGE | chan);
                unsigned char b2 = (unsigned char)param1; // prg

                fsMidi_sendHardwareMidiByte(b1);
                fsMidi_sendHardwareMidiByte(b2);
                break;
            }
            case MIDIEVENT_CHANNEL_PRESSURE: //0xd0
            {
                unsigned char b1 = (unsigned char)(MIDIEVENT_CHANNEL_PRESSURE | chan);
                unsigned char b2 = (unsigned char)param1; // pressure

                fsMidi_sendHardwareMidiByte(b1);
                fsMidi_sendHardwareMidiByte(b2);
                break;
            }
            case MIDIEVENT_CONTROL_CHANGE: //0xb0
            {
                unsigned char b1 = (unsigned char)(MIDIEVENT_CONTROL_CHANGE | chan);
                unsigned char b2 = (unsigned char)param1; // control
                unsigned char b3 = (unsigned char)param2; // val

                fsMidi_sendHardwareMidiByte(b1);
                fsMidi_sendHardwareMidiByte(b2);
                fsMidi_sendHardwareMidiByte(b3);
                break;
            }
        }
    }
}

void fsMidi_sendHardwareMidiByte(unsigned char b)
{
    if (_fsMidiHardwareOutEnabled)
    {
        // TODO: buffer these up before sending to hardware device
        if (g_jniMainInterface.android_mainEmuThreadEnv != 0)
        {
            (*g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainEmuThreadEnv, g_jniMainInterface.android_mainActivity,
                                                                           g_jniMainInterface.sendMidiByte, (jbyte)b);
        }
    }
}
