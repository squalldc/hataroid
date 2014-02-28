#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sdl.h>
#include <math.h>

#include "hataroid.h"
#include "nativeRenderer_ogles2.h"
#include "VirtKB.h"

#include <gui-android.h>

#define  LOG_TAG    "hataroid"

extern "C"
{
	#include <screen.h>
	#include <change.h>
	#include <dialog.h>
	#include <sound.h>
	#include <memorySnapShot.h>

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_libExit(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationInit(JNIEnv * env, jobject obj, jobject activityInstance, jobjectArray keyarray, jobjectArray valarray);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationMain(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationDestroy(JNIEnv * env, jobject obj, jobject activityInstance);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationPause(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationResume(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceCreated(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onDrawFrame(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetOptions(JNIEnv * env, jobject obj, jobjectArray keyarray, jobjectArray valarray);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetCold(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetWarm(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorEjectFloppy(JNIEnv * env, jobject obj, jint floppy);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorInsertFloppy(JNIEnv * env, jobject obj, jint floppy, jstring filename, jstring zippath);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateSave(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateLoad(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveStoreOnExit(JNIEnv * env, jobject obj, jstring saveFolder);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveLoadOnStart(JNIEnv * env, jobject obj, jstring saveFolder);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleUserPaused(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetUserPaused(JNIEnv * env, jobject obj);

	JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppy(JNIEnv * env, jobject obj, jint floppy);
	JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppyZip(JNIEnv * env, jobject obj, jint floppy);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidDialogResult(JNIEnv * env, jobject obj, jint result);

	JavaVM *g_jvm = 0;
	struct JNIAudio g_jniAudioInterface;
	struct JNIMainMethodCache g_jniMainInterface;

	extern int hatari_main_init(int argc, const char *argv[]);
	extern int hatari_main_doframe();
	extern void hatari_main_exit();
	extern void SDL_CloseAudio();
	extern bool Main_PauseEmulation(bool visualize);
	extern bool Main_UnPauseEmulation(void);
	extern void MemorySnapShot_setConfirmOnOverwriteSave(bool set);

	volatile int g_lastDialogResultValid = 0;
	volatile int g_lastDialogResult = -1;

	extern int g_videoTex_width;
	extern int g_videoTex_height;
	extern void *g_videoTex_pixels;
	extern int g_surface_width;
	extern int g_surface_height;
};

enum
{
	EMUPAUSETYPE_USER = (1<<0),
	EMUPAUSETYPE_SAVE = (1<<1),
};

static volatile int emuUserPaused = 0;
static volatile bool emuReady = false;
static volatile bool emuInited = false;
static volatile bool videoReady = false;
static volatile bool envInited = false;

static int s_turboSpeed = 0;
static int s_turboPrevFrameSkips = 0;

static bool _altUpdate = false;

static void SetEmulatorOptions(JNIEnv * env, jobjectArray keyarray, jobjectArray valarray, bool apply, bool init, bool queueCommand);
void _storeSaveState(const char *saveMetaFilePath);

//---------------
struct EmuCommand;
typedef void (*EmuCommandRunCallback)(EmuCommand *command);
typedef void (*EmuCommandCleanupCallback)(EmuCommand *command);
struct EmuCommand
{
	void *data;
	EmuCommandRunCallback runCallback;
	EmuCommandCleanupCallback cleanupCallback;
	EmuCommand *next;
};

volatile int s_EmuCommandLock = 0;
EmuCommand *s_headCommand = 0;

static EmuCommand *createEmuCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data);
static void addEmuCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data);
//---------------

void Debug_Printf(const char *a_pszformat, ...)
{
	va_list args;
	va_start(args, a_pszformat);
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, a_pszformat, args);
}

void usleep(int usecs)
{
	struct timespec	ts;

	ts.tv_sec = usecs / 1000000;
	ts.tv_nsec = (usecs % 1000000) * 1000; // micro sec -> nano sec
	nanosleep(&ts, &ts);
}

void showGenericDialog(const char *message, int ok, int noyes)
{
	Debug_Printf("Show Generic Dialog");
	jstring str = (g_jniMainInterface.android_mainEmuThreadEnv)->NewStringUTF(message);
	(g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showGenericDialog, ok, noyes, str);
	Debug_Printf("Show Generic Dialog Done");
}

void showOptionsDialog()
{
	Debug_Printf("Show Options Dialog");
	(g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showOptionsDialog);
	Debug_Printf("Show Options Dialog Done");
}

static void requestQuitHataroid()
{
	Debug_Printf("Quit Hataroid");
	(g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.quitHataroid);
}

void RequestAndWaitQuit()
{
	requestQuitHataroid();

	for (;;)
	{
		usleep(200000); // 0.2 sec
	}
}

void setUserEmuPaused(int pause)
{
	if (pause)	{ emuUserPaused |= EMUPAUSETYPE_USER; }
	else		{ emuUserPaused &= ~EMUPAUSETYPE_USER; }
}

int hasDialogResult() { return g_lastDialogResultValid; }
int getDialogResult() { return g_lastDialogResult; }
void clearDialogResult() { g_lastDialogResultValid = 0; g_lastDialogResult = -1; }

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_libExit(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> libExit");

	if (g_jniMainInterface.mainActivityGlobalRefObtained!=0)
	{
		(env)->DeleteGlobalRef(g_jniMainInterface.android_mainActivity);
		g_jniMainInterface.mainActivityGlobalRefObtained = 0;
	}

	exit(0);
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	g_jvm = jvm;
	g_jniMainInterface.mainActivityGlobalRefObtained = 0;

	return JNI_VERSION_1_6;
}

static void registerJNIcallbacks(JNIEnv * env, jobject activityInstance)
{
	jobject mainActivityRef = (env)->NewGlobalRef(activityInstance);
	g_jniAudioInterface.android_env = env;
	g_jniAudioInterface.android_mainActivity = mainActivityRef;

	jclass activityClass = (env)->GetObjectClass(activityInstance);

	g_jniAudioInterface.getMinBufSize = (env)->GetMethodID(activityClass, "getMinBufSize", "(III)I");
	g_jniAudioInterface.initAudio = (env)->GetMethodID(activityClass, "initAudio", "(IIII)V");
	g_jniAudioInterface.deinitAudio = (env)->GetMethodID(activityClass, "deinitAudio", "()V");

	g_jniAudioInterface.playAudio = (env)->GetMethodID(activityClass, "playAudio", "()V");
	g_jniAudioInterface.pauseAudio = (env)->GetMethodID(activityClass, "pauseAudio", "()V");

	g_jniAudioInterface.sendAudio = (env)->GetMethodID(activityClass, "sendAudio", "([S)V");

	g_jniMainInterface.android_env = env;
	g_jniMainInterface.android_mainEmuThreadEnv = NULL;
	g_jniMainInterface.android_mainActivity = mainActivityRef;
	g_jniMainInterface.showGenericDialog = (env)->GetMethodID(activityClass, "showGenericDialog", "(IILjava/lang/String;)V");
	g_jniMainInterface.showOptionsDialog = (env)->GetMethodID(activityClass, "showOptionsDialog", "()V");
	g_jniMainInterface.quitHataroid = (env)->GetMethodID(activityClass, "quitHataroid", "()V");

	g_jniMainInterface.mainActivityGlobalRefObtained = 1;
}

static int waitDebugger = 0;

static void _initEmulator()
{
	if (!emuInited)
	{
		while (waitDebugger)
		{
			usleep(100000);
		}

		const char *argv[] = { "Hataroid" };
		hatari_main_init(1, argv);

		emuInited = true;
	}
}

static void _checkEmuReady()
{
	if (!emuReady)
	{
		emuReady = videoReady && emuInited && envInited;// && emuUserReady;
		if (emuReady)
		{
			VirtKB_EnableInput(true);
		}
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidDialogResult(JNIEnv * env, jobject obj, jint result)
{
	g_lastDialogResultValid = 1;
	g_lastDialogResult = result;
}

JNIEnv *s_optionsEnv = 0;
jobjectArray s_optionsKeyArray = 0;
jobjectArray s_optionsValarray = 0;
bool s_optionsIniting = false;

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationInit(JNIEnv * env, jobject obj, jobject activityInstance, jobjectArray keyarray, jobjectArray valarray)
{
	Debug_Printf("----> emulationInit");

	if (!envInited)
	{
		memset(&g_jniAudioInterface, 0, sizeof(JNIAudio));
		registerJNIcallbacks(env, activityInstance);
		envInited = true;
	}

	s_optionsEnv = env;
	s_optionsKeyArray = keyarray;
	s_optionsValarray = valarray;
	s_optionsIniting = true;
	g_jniMainInterface.android_mainEmuThreadEnv = env;

	_initEmulator();

	s_optionsIniting = false;
	s_optionsEnv = 0;
	s_optionsKeyArray = 0;
	s_optionsValarray = 0;

	_checkEmuReady();
}

extern "C"	void hataroid_loadInitialConfig()
{
	if (s_optionsEnv && s_optionsKeyArray && s_optionsValarray)
	{
		SetEmulatorOptions(s_optionsEnv, s_optionsKeyArray, s_optionsValarray, false, true, false);
	}
}


JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationMain(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationMain");

	g_jniMainInterface.android_mainEmuThreadEnv = env;

	for (;;)
	{
		if (emuReady)
		{
			if (!_altUpdate)
			{
				if (emuUserPaused == 0)
				{
					hatari_main_doframe();
				}
				SDL_UpdateRects(sdlscrn, 0, 0);
				processEmuCommands();
			}
		}
		else
		{
			usleep(10000);
		}
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationDestroy(JNIEnv * env, jobject obj, jobject activityInstance)
{
	Debug_Printf("----> emulationDestroy");

	VirtKB_EnableInput(false);

	emuReady = false;
	videoReady = false;
	emuInited = false;
	envInited = false;

	memset(&g_jniAudioInterface, 0, sizeof(JNIAudio));
	registerJNIcallbacks(env, activityInstance);

	SDL_CloseAudio();
	//hatari_main_exit();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationPause(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationPause");

	setTurboSpeed(0);
	VirtKB_EnableInput(false);

	emuReady = false;
	videoReady = false;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleUserPaused(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulation USER Pause toggle");

	setTurboSpeed(0);
	//VirtKB_EnableInput(false);

	//emuReady = false;
	setUserEmuPaused((emuUserPaused & EMUPAUSETYPE_USER) == 0);

	//_checkEmuReady();
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetUserPaused(JNIEnv * env, jobject obj)
{
	return ((emuUserPaused & EMUPAUSETYPE_USER) != 0);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationResume(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationResume");
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceCreated(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> onSurfaceCreated");
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height)
{
	Debug_Printf("----> onSurfaceChanged: (%d x %d)", width, height);

	{
		setupGraphics(width, height);
		VirtKB_OnSurfaceChanged(width, height);
		videoReady = true;
	}

	_checkEmuReady();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onDrawFrame(JNIEnv * env, jobject obj)
{
	if (emuReady)
	{
		if (_altUpdate)
		{
			if (emuUserPaused == 0)
			{
				hatari_main_doframe();
			}
			SDL_UpdateRects(sdlscrn, 0, 0);
			processEmuCommands();
		}

		renderFrame();
	}
}

int getTurboSpeed() { return s_turboSpeed; }

void setTurboSpeed(int set)
{
	if (s_turboSpeed == set)
	{
		return;
	}

	s_turboSpeed = set;

	CNF_PARAMS current = ConfigureParams;
	ConfigureParams.System.bFastForward = (s_turboSpeed!=0);
	if (ConfigureParams.System.bFastForward)
	{
		s_turboPrevFrameSkips = ConfigureParams.Screen.nFrameSkips;
		ConfigureParams.Screen.nFrameSkips = 5;
	}
	else
	{
		ConfigureParams.Screen.nFrameSkips = s_turboPrevFrameSkips;

		// Reset the sound emulation variables:
		Sound_BufferIndexNeedReset = true;
	}
	Change_CopyChangedParamsToConfiguration(&current, &ConfigureParams, false);

	VirtKB_RefreshKB();
}

char * _stringArrayElemToCStr(JNIEnv* env, jobjectArray array, int elem)
{
	jstring jstr = (jstring)(env)->GetObjectArrayElement(array, elem);
	int jstrlen = (env)->GetStringUTFLength(jstr);
	const char *jcharstr = (env)->GetStringUTFChars(jstr, 0);

	char *result = new char [jstrlen+1];
	strncpy(result, jcharstr, jstrlen);
	result[jstrlen] = 0;

	(env)->ReleaseStringUTFChars(jstr, jcharstr);
	(env)->DeleteLocalRef(jstr); // explicitly releasing to assist garbage collection, though not required

	return result;
}

struct EmuCommandSetOptions_Data
{
	int numKeys;
	char **keys;
	char **vals;
	bool apply;
	bool init;
};

struct OptionSetting;
typedef void (*OptionCallback)(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data);
struct OptionSetting
{
	const char		*key;
	OptionCallback	callback;
};

bool _getBoolVal(const char *val) { return !((strcasecmp(val, "false") == 0) || (strcmp(val, "0") == 0)); }

void _optionSetMachineType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	const char *tospref = NULL;
	if (strcmp(val, "ST") == 0)
	{
		ConfigureParams.System.nMachineType = MACHINE_ST;
		tospref = "pref_system_tos";
	}
	if (strcmp(val, "STE") == 0)
	{
		ConfigureParams.System.nMachineType = MACHINE_STE;
		tospref = "pref_system_tos_ste";
	}

	if (tospref != NULL)
	{
		for (int i = 0; i < data->numKeys; ++i)
		{
			const char *curKey = data->keys[i];
			if (strcmp(curKey, tospref)==0)
			{
				const char *curVal = data->vals[i];
				if (curVal != NULL && strlen(curVal) > 0)
				{
					strncpy(ConfigureParams.Rom.szTosImageFileName, curVal, FILENAME_MAX);
					ConfigureParams.Rom.szTosImageFileName[FILENAME_MAX-1]=0;
				}
				break;
			}
		}
	}
}
void _optionShowBorders(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Screen.bAllowOverscan = _getBoolVal(val);
	Renderer_refreshDispParams();
}
void _optionShowIndicators(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Screen.bShowStatusbar = false;
	ConfigureParams.Screen.bShowDriveLed = false;

	if (strcmp(val, "statusbar") == 0) { ConfigureParams.Screen.bShowStatusbar = true; }
	else if (strcmp(val, "driveled") == 0) { ConfigureParams.Screen.bShowDriveLed = true; }
}
void _optionAutoInsertDiskB(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.bAutoInsertDiskB = _getBoolVal(val);
}
void _optionWriteProtectFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.nWriteProtection = (strcmp(val, "0") == 0) ? WRITEPROT_OFF : (strcmp(val, "1") == 0) ? WRITEPROT_ON : WRITEPROT_AUTO;
}
void _optionFastFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.FastFloppy = _getBoolVal(val);
}
void _optionsSetCompatibleCPU(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bCompatibleCpu = _getBoolVal(val);
}
void _optionSetSoundEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Sound.bEnableSound = _getBoolVal(val);
}
void _optionSetSoundSync(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Sound.bEnableSoundSync = _getBoolVal(val);
}
void _optionSetSoundQuality(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int freq = atoi(val);
	ConfigureParams.Sound.nPlaybackFreq = freq;
}
void _optionSetYMVoicesMixing(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "math") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_MODEL_MIXING;
	else if (strcmp(val, "st") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_TABLE_MIXING;
	else if (strcmp(val, "linear") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_LINEAR_MIXING;
}
void _optionSoundBufferSize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int bufSize = atoi(val);
	if (bufSize < 1)		{ bufSize = 1; }
	else if (bufSize > 50) {  bufSize = 50; }
	ConfigureParams.Hataroid.deviceSoundBufSize = bufSize;
}
void _optionSetMonitorType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "Mono") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_MONO;
	else if (strcmp(val, "RGB") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_RGB;
	else if (strcmp(val, "VGA") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_VGA;
	else if (strcmp(val, "TV") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_TV;
}
void _optionSetFrameSkip(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int fskip = atoi(val); // 5 = auto (AUTO_FRAMESKIP_LIMIT)
	ConfigureParams.Screen.nFrameSkips = fskip;
}
void _optionSetRTC(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bRealTimeClock = _getBoolVal(val);
}
void _optionPatchTimerD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bPatchTimerD = _getBoolVal(val);
}
void _optionFastBoot(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bFastBoot = _getBoolVal(val);
}
void _optionMemorySize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int memsize = atoi(val); // Mb (0 = 512k)
	ConfigureParams.Memory.nMemorySize = memsize;
}
void _optionSetCPUType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "68000") == 0) ConfigureParams.System.nCpuLevel = 0;
	else if (strcmp(val, "68010") == 0) ConfigureParams.System.nCpuLevel = 1;
	else if (strcmp(val, "68020") == 0) ConfigureParams.System.nCpuLevel = 2;
	else if (strcmp(val, "68EC030FPU") == 0) ConfigureParams.System.nCpuLevel = 3;
	else if (strcmp(val, "68040") == 0) ConfigureParams.System.nCpuLevel = 4;
}
void _optionSetCPUFreq(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int freq = atoi(val);
	ConfigureParams.System.nCpuFreq = freq;
}
void _optionSetBlitter(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bBlitter = _getBoolVal(val);
}
void _optionSetJoystickPort(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int port = atoi(val);
	VirtKB_SetJoystickPort(port);
}
void _optionSetJoystickAutoFire(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int port = VirtKB_GetJoystickPort();
	bool autoFire = _getBoolVal(val);
	ConfigureParams.Joysticks.Joy[1].bEnableAutoFire = autoFire;

}
void _optionSetJoystickMapArrowKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool map = _getBoolVal(val);
	VirtKB_MapJoysticksToArrowKeys(map);
}
void _optionSetBilinearFilter(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool filter = _getBoolVal(val);
	Renderer_setFilterEmuScreeen(filter);
}

void _optionSetFullScreenStretch(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool fullscreen = _getBoolVal(val);
	if (fullscreen)
	{
		Renderer_setFullScreenStretch(fullscreen);
		VirtKB_setScreenZoomMode(false);
	}
	else
	{
		if (Renderer_isFullScreenStretch())
		{
			Renderer_setFullScreenStretch(false);
			VirtKB_setDefaultScreenZoomPreset();
			VirtKB_setScreenZoomMode(false);
		}
	}
}
void _optionSetMouseEmuType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "direct") == 0)
	{
		VirtKB_SetMouseEmuDirect();
	}
	else if (strcmp(val, "directoffset") == 0)
	{
		VirtKB_SetMouseEmuDirect(); // TODO
	}
	else if (strcmp(val, "buttons") == 0)
	{
		VirtKB_SetMouseEmuButtons();
	}
}

void _optionSetMouseSpeed(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float speed = atof(val);
	VirtKB_SetMouseEmuSpeed(speed);
}

void _optionSetOnScreenAlpha(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float alpha = atof(val);
	alpha *= (1.0f/100.0f);

	if (alpha < 0.0f) { alpha = 0.0f; }
	else if (alpha > 1.0f) { alpha = 1.0f; }

	VirtKB_SetControlAlpha(alpha);
}

void _optionSetJoystickSize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = atof(val);
	size *= (1.0f/100.0f);

	if (size < 0.1f) { size = 0.1f; }
	else if (size > 3.0f) { size = 3.0f; }

	VirtKB_SetJoystickSize(size);
}

void _optionBootFromHD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bBootFromHardDisk = _getBoolVal(val);
}

void _optionACSIAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseHardDiskImage = _getBoolVal(val);
}
void _optionIDEMasterAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseIdeMasterHardDiskImage = _getBoolVal(val);
}
void _optionIDESlaveAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseIdeSlaveHardDiskImage = _getBoolVal(val);
}
void _optionGEMDOSAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseHardDiskDirectories = _getBoolVal(val);
}
void _optionACSIImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szHardDiskImage, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szHardDiskImage[FILENAME_MAX-1]=0;
}
void _optionIDEMasterImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szIdeMasterHardDiskImage, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szIdeMasterHardDiskImage[FILENAME_MAX-1]=0;
}
void _optionIDESlaveImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szIdeSlaveHardDiskImage, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szIdeSlaveHardDiskImage[FILENAME_MAX-1]=0;
}
void _optionGEMDOSFolder(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szHardDiskDirectories[0], ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szHardDiskDirectories[0][FILENAME_MAX-1]=0;
}
void _optionGEMDOSWriteProtection(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int optionVal = atoi(val);

	if (optionVal == 0) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_OFF; } // off
	else if (optionVal == 1) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_ON; } // on
	else if (optionVal == 2) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_AUTO; } // auto
}
void _optionSetVKBExtraKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool extraKeys = _getBoolVal(val);
	VirtKB_setExtraKeys(extraKeys);
}
void _optionSetVKBObsessionKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	_altUpdate = valSet;
	VirtKB_setObsessionKeys(valSet);
}
void _optionSetVKBHideAll(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	VirtKB_setHideAll(valSet);
}
void _optionSetVKBJoystickOnly(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	VirtKB_setJoystickOnly(valSet);
}

static const OptionSetting s_OptionsMap[] =
{
	{ "pref_input_joystick_port", _optionSetJoystickPort },
	{ "pref_input_joysticks_autofire", _optionSetJoystickAutoFire },
	{ "pref_input_joysticks_maparrowkeys", _optionSetJoystickMapArrowKeys },
	{ "pref_input_joysticks_size", _optionSetJoystickSize },
	{ "pref_input_mouse_emutype", _optionSetMouseEmuType },
	{ "pref_input_mouse_speed", _optionSetMouseSpeed },
	{ "pref_input_onscreen_alpha", _optionSetOnScreenAlpha },
	{ "pref_system_blitteremulation", _optionSetBlitter },
	{ "pref_system_cpuclock", _optionSetCPUFreq },
	{ "pref_system_cputype", _optionSetCPUType },
	{ "pref_system_compatiblecpu", _optionsSetCompatibleCPU },
	{ "pref_system_machinetype", _optionSetMachineType },
	{ "pref_system_memory", _optionMemorySize },
	{ "pref_system_midiemulation", 0 },
	{ "pref_system_patchtimerd", _optionPatchTimerD },
	{ "pref_system_patchtosfasterboot", _optionFastBoot },
	{ "pref_system_printeremulation", 0 },
	{ "pref_system_rs232emulation", 0 },
	{ "pref_system_rtc", _optionSetRTC },
	{ "pref_system_tos", 0 },
	{ "pref_system_tos_ste", 0 },
	{ "pref_display_bilinearfilter", _optionSetBilinearFilter },
	{ "pref_display_fullscreen", _optionSetFullScreenStretch },
	{ "pref_display_keepscreenawake", 0 },
	{ "pref_display_extendedvdi", 0 },
	{ "pref_display_extendedvdi_colors", 0 },
	{ "pref_display_extendedvdi_resolution", 0 },
	{ "pref_storage_cartridge", 0 },
	{ "pref_display_frameskip", _optionSetFrameSkip },
	{ "pref_display_indicators", _optionShowIndicators },
	{ "pref_display_monitortype", _optionSetMonitorType },
	{ "pref_display_showborders", _optionShowBorders },
	{ "pref_storage_floppydisks_autoinsertb", _optionAutoInsertDiskB },
	{ "pref_storage_floppydisks_fastfloppyaccess", _optionFastFloppy },
	{ "pref_storage_floppydisks_imagedir", 0 },
	{ "pref_storage_floppydisks_writeprotection", _optionWriteProtectFloppy },
	{ "pref_storage_harddisks_acsiimage", _optionACSIImage },
	{ "pref_storage_harddisks_acsi_attach", _optionACSIAttach },
	{ "pref_storage_harddisks_bootfromharddisk", _optionBootFromHD },
	{ "pref_storage_harddisks_gemdosdrive", _optionGEMDOSFolder },
	{ "pref_storage_harddisks_gemdosdrive_attach", _optionGEMDOSAttach},
	{ "pref_storage_harddisks_gemdoswriteprotection", _optionGEMDOSWriteProtection },
	{ "pref_storage_harddisks_idemasterimage", _optionIDEMasterImage },
	{ "pref_storage_harddisks_idemaster_attach", _optionIDEMasterAttach },
	{ "pref_storage_harddisks_ideslaveimage", _optionIDESlaveImage },
	{ "pref_storage_harddisks_ideslave_attach", _optionIDESlaveAttach },
	{ "pref_sound_enabled", _optionSetSoundEnabled  },
	{ "pref_sound_quality", _optionSetSoundQuality },
	{ "pref_sound_synchronize_enabled", _optionSetSoundSync },
	{ "pref_sound_ymvoicesmixing", _optionSetYMVoicesMixing },
	{ "pref_sound_buffer_size", _optionSoundBufferSize },
	{ "pref_input_keyboard_extra_keys", _optionSetVKBExtraKeys },
	{ "pref_input_keyboard_obsession_keys", _optionSetVKBObsessionKeys },
	{ "pref_input_onscreen_hide_all", _optionSetVKBHideAll},
	{ "pref_input_onscreen_only_joy", _optionSetVKBJoystickOnly },
};
static const int s_NumOptionMaps = sizeof(s_OptionsMap)/sizeof(OptionSetting);

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetOptions(JNIEnv * env, jobject obj, jobjectArray keyarray, jobjectArray valarray)
{
	SetEmulatorOptions(env, keyarray, valarray, true, false, true);
}

void EmuCommandSetOptions_Run(EmuCommand *command)
{
	Debug_Printf("----> Set Options");

	EmuCommandSetOptions_Data *data = (EmuCommandSetOptions_Data*)command->data;

	//int changed = 0;
	//bool reset = false;

	bool bForceReset = false;
	bool bLoadedSnapshot = false;
	CNF_PARAMS current;

	if (!s_optionsIniting)//data->init)
	{
		Main_PauseEmulation(true);
	}

	// Copy details (this is so can restore if 'Cancel' dialog)
	current = ConfigureParams;

	Debug_Printf("Configuration Options:");
	// slow (doesn't need to be fast)
	for (int i = 0; i < data->numKeys; ++i)
	{
		const char *curKey = data->keys[i];
		const char *curVal = data->vals[i];

		Debug_Printf("'%s' = '%s'", curKey, curVal);
		for (int s = 0; s < s_NumOptionMaps; ++s)
		{
			const OptionSetting *curSetting = &s_OptionsMap[s];
			if (strcmp(curKey, curSetting->key) == 0)
			{
				if (curSetting->callback != 0)
				{
					/*changed |=*/ (*(curSetting->callback))(curSetting, curVal, data);
				}
				break;
			}
		}
	}

	{
		// forced settings
		ConfigureParams.Sound.SdlAudioBufferSize = 25;

		// Reset the sound emulation variables:
		Sound_BufferIndexNeedReset = true;
	}

	// If a memory snapshot has been loaded, no further changes are required
	if (!bLoadedSnapshot)
	{
		//Debug_Printf("----> Apply [%s]", (data->apply?"true":"false"));

		if (data->apply)
		{
			bool bOKDialog = true;

			// Check if reset is required and ask user if he really wants to continue then
			if (!bForceReset
				&& Change_DoNeedReset(&current, &ConfigureParams)
			)
			{
				bOKDialog = DlgAlert_Query("The emulated system must be "
										   "reset to apply these changes. "
										   "Apply changes now and reset "
										   "the emulator?");
			}

			// Copy details to configuration
			if (bOKDialog)
			{
				Change_CopyChangedParamsToConfiguration(&current, &ConfigureParams, bForceReset);
			}
//			else
//			{
//				ConfigureParams = current;
//			}
		}
	}

	if (!s_optionsIniting)//data->init)
	{
		Main_UnPauseEmulation();
	}

	Debug_Printf("----> Set Options Done");
}

void EmuCommandSetOptions_Cleanup(EmuCommand *command)
{
	EmuCommandSetOptions_Data *data = (EmuCommandSetOptions_Data*)command->data;

	for (int i = 0; i < data->numKeys; ++i)
	{
		delete [] data->keys[i];
		delete [] data->vals[i];
	}
	delete [] data->keys;
	delete [] data->vals;

	delete data;
}

static void SetEmulatorOptions(JNIEnv * env, jobjectArray keyarray, jobjectArray valarray, bool apply, bool init, bool queueCommand)
{
	Debug_Printf("----> Set Options Command");

	EmuCommandSetOptions_Data *data = new EmuCommandSetOptions_Data;
	memset(data, 0, sizeof(EmuCommandSetOptions_Data));

	data->apply = apply;
	data->init = init;
	data->numKeys = (env)->GetArrayLength(keyarray);
	data->keys = new char * [data->numKeys];
	data->vals = new char * [data->numKeys];

	for (int i = 0; i < data->numKeys; ++i)
	{
		data->keys[i] = _stringArrayElemToCStr(env, keyarray, i);
		data->vals[i] = _stringArrayElemToCStr(env, valarray, i);
	}

	if (queueCommand)
	{
		addEmuCommand(EmuCommandSetOptions_Run, EmuCommandSetOptions_Cleanup, data);
	}
	else
	{
		EmuCommand *command = createEmuCommand(EmuCommandSetOptions_Run, EmuCommandSetOptions_Cleanup, data);
		EmuCommandSetOptions_Run(command);
		EmuCommandSetOptions_Cleanup(command);
		delete command;
	}
}

void EmuCommandResetCold_Run(EmuCommand *command)
{
	Debug_Printf("----> Cold Reset");
	setTurboSpeed(0);
	setUserEmuPaused(0);
	Reset_Cold();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetCold(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> Cold Reset Command");
	if (emuInited) { addEmuCommand(EmuCommandResetCold_Run, NULL, NULL); }
}

void EmuCommandResetWarm_Run(EmuCommand *command)
{
	Debug_Printf("----> Warm Reset");
	setTurboSpeed(0);
	setUserEmuPaused(0);
	Reset_Warm();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetWarm(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> Warm Reset Command");
	if (emuInited) { addEmuCommand(EmuCommandResetWarm_Run, NULL, NULL); }
}

void EmuCommandEjectFloppy_Run(EmuCommand *command)
{
	Debug_Printf("----> Eject Floppy");

	int floppyID = (int)command->data;

	Floppy_SetDiskFileNameNone(floppyID);
	ConfigureParams.DiskImage.szDiskZipPath[floppyID][0] = '\0';
	Floppy_EjectDiskFromDrive(floppyID);

	Debug_Printf("----> Ejected Floppy from drive (%c)", floppyID?'B':'A');
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorEjectFloppy(JNIEnv * env, jobject obj, jint floppy)
{
	Debug_Printf("----> Eject Floppy Command");
	if (emuInited) { addEmuCommand(EmuCommandEjectFloppy_Run, NULL, (void*)((int)floppy)); }
}

struct EmuCommandInsertFloppy_Data
{
	int floppyID;
	char *floppyFileName;
	char *floppyZipPath;
};

void EmuCommandInsertFloppy_Run(EmuCommand *command)
{
	Debug_Printf("----> Insert Floppy");

	EmuCommandInsertFloppy_Data *data = (EmuCommandInsertFloppy_Data*)command->data;

	Floppy_SetDiskFileName(data->floppyID, data->floppyFileName, data->floppyZipPath);
	Floppy_InsertDiskIntoDrive(data->floppyID);

	Debug_Printf("----> Inserted Floppy: (%s)(%s) into drive (%c)", data->floppyFileName, data->floppyZipPath ? data->floppyZipPath : "direct", data->floppyID?'B':'A');
}

void EmuCommandInsertFloppy_Cleanup(EmuCommand *command)
{
	EmuCommandInsertFloppy_Data *data = (EmuCommandInsertFloppy_Data*)command->data;
	if (data->floppyFileName) delete [] data->floppyFileName;
	if (data->floppyZipPath) delete [] data->floppyZipPath;
	delete data;
}

struct EmuCommandSaveState_Data
{
	static const int OpSave				= 0;
	static const int OpLoad				= 1;
	static const int OpSaveAutoSave		= 2;
	static const int OpLoadAutoSave		= 3;

	int saveStateOp;
	int saveSlot;
	char *saveStatePath;
	char *saveStateFilePath;
};

void EmuCommandSaveState_Run(EmuCommand *command)
{
	Debug_Printf("----> Save State Operation");

	EmuCommandSaveState_Data *data = (EmuCommandSaveState_Data*)command->data;

	switch (data->saveStateOp)
	{
		case EmuCommandSaveState_Data::OpSave:
		{
			remove(data->saveStateFilePath); // TODO: check error codes

			// create new save name based on inserted floppy
			char* saveMetaBaseName = new char [FILENAME_MAX];
			char* saveMetaFilePath = new char [FILENAME_MAX];

			char* tempFloppyName = 0;

			const char *sFloppyName = 0;
			for (int d = 0; d < MAX_FLOPPYDRIVES; ++d)
			{
				if (ConfigureParams.DiskImage.szDiskFileName[d][0] != 0)
				{
					sFloppyName = ConfigureParams.DiskImage.szDiskFileName[d];
					break;
				}
				else if (ConfigureParams.DiskImage.szDiskZipPath[d][0] != 0)
				{
					sFloppyName = ConfigureParams.DiskImage.szDiskZipPath[d];
					break;
				}
			}
			if (sFloppyName == 0)
			{
				sFloppyName = "unknown";
			}
			else
			{
				const char *floppyBase = strrchr(sFloppyName, '/');
				if (floppyBase != 0)
				{
					sFloppyName = &floppyBase[1];
				}

				tempFloppyName = new char [FILENAME_MAX];
				strncpy(tempFloppyName, sFloppyName, FILENAME_MAX);

				int nameLen = strlen(tempFloppyName);
				for (int c = nameLen-1; c >= 0; --c)
				{
					if (tempFloppyName[c] == '.')
					{
						tempFloppyName[c] = 0;
						break;
					}
				}

				sFloppyName = tempFloppyName;
			}

			if (data->saveSlot >= 0)
			{
				snprintf(saveMetaBaseName, FILENAME_MAX, "%03d_%s", data->saveSlot, sFloppyName);
				snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/%03d.sav", data->saveStatePath, data->saveSlot);
			}
			else
			{
				snprintf(saveMetaBaseName, FILENAME_MAX, "xxx_%s", data->saveSlot, sFloppyName);
				snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/xxx.sav", data->saveStatePath);
			}
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;
			saveMetaBaseName[FILENAME_MAX-1]=0;

			snprintf(saveMetaFilePath, FILENAME_MAX, "%s/%s.ss", data->saveStatePath, saveMetaBaseName);
			saveMetaFilePath[FILENAME_MAX-1]=0;

			Debug_Printf("Floppy: %s, SaveName: %s, MetaName: %s", sFloppyName, ConfigureParams.Memory.szMemoryCaptureFileName, saveMetaFilePath);

			_storeSaveState(saveMetaFilePath);

			if (tempFloppyName != 0)
			{
				delete [] tempFloppyName;
			}
			delete [] saveMetaFilePath;
			delete [] saveMetaBaseName;

			emuUserPaused &= ~EMUPAUSETYPE_SAVE;
			break;
		}
		case EmuCommandSaveState_Data::OpLoad:
		{
			if (data->saveSlot >= 0)
			{
				snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/%03d.sav", data->saveStatePath, data->saveSlot);
			}
			else
			{
				snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/xxx.sav", data->saveStatePath);
			}
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;

			MemorySnapShot_Restore(ConfigureParams.Memory.szMemoryCaptureFileName, false);
			break;
		}
		case EmuCommandSaveState_Data::OpSaveAutoSave:
		{
			Debug_Printf("----> Saving auto save...");

			char* saveMetaFilePath = new char [FILENAME_MAX];
			snprintf(saveMetaFilePath, FILENAME_MAX, "%s/as.qs", data->saveStatePath);
			saveMetaFilePath[FILENAME_MAX-1] = 0;

			snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/as.sav", data->saveStatePath);
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1] = 0;

			_storeSaveState(saveMetaFilePath);

			delete [] saveMetaFilePath;
			emuUserPaused &= ~EMUPAUSETYPE_SAVE;

			Debug_Printf("----> Auto Save saved...");

			RequestAndWaitQuit();
			return;
		}
		case EmuCommandSaveState_Data::OpLoadAutoSave:
		{
			Debug_Printf("----> Loading auto save...");

			snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/as.sav", data->saveStatePath);
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;

			MemorySnapShot_Restore(ConfigureParams.Memory.szMemoryCaptureFileName, false);
			break;
		}
	}

	// Reset the sound emulation variables:
	Sound_BufferIndexNeedReset = true;
}

void _storeSaveState(const char *saveMetaFilePath)
{
	// save state
	{
		MemorySnapShot_setConfirmOnOverwriteSave(false);
		MemorySnapShot_Capture(ConfigureParams.Memory.szMemoryCaptureFileName, false);
	}

	// save meta info
	{
		FILE *metaFile = fopen(saveMetaFilePath, "w");
		if (metaFile != 0)
		{
			const int kVersion = 1;
			const int kHeaderSize = 3 * 4;

			fwrite("hsst", 4, 1, metaFile);
			fwrite(&kHeaderSize, 4, 1, metaFile);
			fwrite(&kVersion, 4, 1, metaFile);

			// generate thumbnail
			{
				unsigned short *emuScrPixels = (unsigned short*)g_videoTex_pixels;
				if (emuScrPixels)
				{
					int texW = g_videoTex_width;
					int emuScrW = g_surface_width;
					int emuScrH = g_surface_height;

					const int kThumbWidth = 80;
					const int kThumbHeight = 60;
					float skipX = emuScrW / (float)kThumbWidth;
					float skipY = emuScrH / (float)kThumbHeight;
					int iSkipX = (int)skipX;
					int iSkipY = (int)skipY;

					fwrite(&kThumbWidth, 4, 1, metaFile);
					fwrite(&kThumbHeight, 4, 1, metaFile);

					int maxSrcW = emuScrW-1;
					int maxSrcH = emuScrH-2; // not correct, but I don't have to deal with edge cases due to fp error for now

					int* rt = new int [kThumbWidth];
					int* gt = new int [kThumbWidth];
					int* bt = new int [kThumbWidth];

					float y = 0;
					for (int savedH = 0; savedH < kThumbHeight; ++savedH)
					{
						int ny = (int)y;
						y += skipY;

						memset(rt, 0, sizeof(int)*kThumbWidth);
						memset(gt, 0, sizeof(int)*kThumbWidth);
						memset(bt, 0, sizeof(int)*kThumbWidth);

						int yc = 0;
						for (int ay = 0; ay < iSkipY; ++ay)
						{
							int iy = ny + ay;
							iy = (iy >= maxSrcH) ? maxSrcH : iy;

							unsigned short *curPixs = emuScrPixels + (iy * texW);
							float x = 0;
							for (int savedW = 0; savedW < kThumbWidth; ++savedW)
							{
								int nx = (int)x;
								x += skipX;

								int r = 0, g = 0, b = 0;
								if (iSkipX > 0)
								{
									for (int ax = 0; ax < iSkipX; ++ax)
									{
										int ix = nx + ax;
										ix = (ix >= maxSrcW) ? maxSrcW : ix;
										unsigned short c = *(curPixs + ix);
										r += c&31;
										g += (c>>5)&63;
										b += (c>>11)&31;
									}
									r /= iSkipX;
									g /= iSkipX;
									b /= iSkipX;
								}

								rt[savedW] += r;
								gt[savedW] += g;
								bt[savedW] += b;
							}
							++yc;
						}

						{
							if (yc == 0)
							{
								yc = 1;
							}
							for (int x = 0; x < kThumbWidth; ++x)
							{
								int r = rt[x] / yc;
								int g = gt[x] / yc;
								int b = bt[x] / yc;

								unsigned short nc = r | (g<<5) | (b<<11);
								fwrite(&nc, 2, 1, metaFile); // (fwrite should already buffer internally, so should be ok to write small values like this)
							}
						}
					}

					delete [] rt;
					delete [] gt;
					delete [] bt;
				}
			}

			fclose(metaFile);
		}
	}
}

void EmuCommandSaveState_Cleanup(EmuCommand *command)
{
	EmuCommandSaveState_Data *data = (EmuCommandSaveState_Data*)command->data;
	if (data->saveStateFilePath) delete [] data->saveStateFilePath;
	if (data->saveStatePath) delete [] data->saveStatePath;
	delete data;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorInsertFloppy(JNIEnv * env, jobject obj, jint floppy, jstring filename, jstring zippath)
{
	Debug_Printf("----> Insert Floppy Command");

	if (emuInited)
	{
		EmuCommandInsertFloppy_Data *data = new EmuCommandInsertFloppy_Data;
		memset(data, 0, sizeof(EmuCommandInsertFloppy_Data));

		data->floppyID = floppy;

		const char *fnameptr = env->GetStringUTFChars(filename, NULL);
		const char *zippathptr = zippath ? env->GetStringUTFChars(zippath, NULL) : NULL;

		data->floppyFileName = new char [strlen(fnameptr)+1];
		strcpy(data->floppyFileName, fnameptr);

		data->floppyZipPath = NULL;
		if (zippathptr)
		{
			data->floppyZipPath = new char [strlen(zippathptr)+1];
			strcpy(data->floppyZipPath, zippathptr);
		}
		else
		{
			data->floppyZipPath = new char [1];
			data->floppyZipPath[0] = 0;
		}

		env->ReleaseStringUTFChars(filename, fnameptr);
		if (zippath)
		{
			env->ReleaseStringUTFChars(zippath, zippathptr);
		}

		addEmuCommand(EmuCommandInsertFloppy_Run, EmuCommandInsertFloppy_Cleanup, data);
	}
}

void _createNewSaveStateCommand(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot, int op)
{
	if (emuInited)
	{
		EmuCommandSaveState_Data *data = new EmuCommandSaveState_Data;
		memset(data, 0, sizeof(EmuCommandSaveState_Data));

		data->saveStateOp = op;
		data->saveSlot = saveSlot;

		if (path != 0)
		{
			const char *fnameptr = env->GetStringUTFChars(path, NULL);
			data->saveStatePath = new char [strlen(fnameptr)+1];
			strcpy(data->saveStatePath, fnameptr);
			env->ReleaseStringUTFChars(path, fnameptr);
		}

		if (filepath != 0)
		{
			const char *fnameptr = env->GetStringUTFChars(filepath, NULL);
			data->saveStateFilePath = new char [strlen(fnameptr)+1];
			strcpy(data->saveStateFilePath, fnameptr);
			env->ReleaseStringUTFChars(filepath, fnameptr);
		}

		addEmuCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateSave(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot)
{
	Debug_Printf("----> Save State Save Command");
	_createNewSaveStateCommand(env, obj, path, filepath, saveSlot, EmuCommandSaveState_Data::OpSave);

	emuUserPaused |= EMUPAUSETYPE_SAVE;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateLoad(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot)
{
	Debug_Printf("----> Save State Load Command");
	_createNewSaveStateCommand(env, obj, path, filepath, saveSlot, EmuCommandSaveState_Data::OpLoad);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveStoreOnExit(JNIEnv * env, jobject obj, jstring saveFolder)
{
	_createNewSaveStateCommand(env, obj, saveFolder, 0, -1, EmuCommandSaveState_Data::OpSaveAutoSave);

	emuUserPaused |= EMUPAUSETYPE_SAVE;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveLoadOnStart(JNIEnv * env, jobject obj, jstring saveFolder)
{
	Debug_Printf("----> Save State AutoLoadOnStart Command");
	_createNewSaveStateCommand(env, obj, saveFolder, 0, -1, EmuCommandSaveState_Data::OpLoadAutoSave);
}

static EmuCommand *createEmuCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data)
{
	EmuCommand *newCommand = new EmuCommand;
	memset(newCommand, 0, sizeof(EmuCommand));

	newCommand->data = data;
	newCommand->runCallback = runCallback;
	newCommand->cleanupCallback = cleanupCallback;

	return newCommand;
}

static void addEmuCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data)
{
	while (s_EmuCommandLock) { usleep(5000); }
	s_EmuCommandLock = 1;

	EmuCommand *newCommand = createEmuCommand(runCallback, cleanupCallback, data);

	if (s_headCommand == 0)
	{
		s_headCommand = newCommand;
	}
	else
	{
		EmuCommand *curCommand = s_headCommand;
		while (curCommand->next != 0)
		{
			curCommand = curCommand->next;
		}
		curCommand->next = newCommand;
	}

	s_EmuCommandLock = 0;
}

int hasEmuCommands()
{
	while (s_EmuCommandLock) { usleep(5000); }
	s_EmuCommandLock = 1;

	int hasCommands = (s_headCommand != 0) ? 1 : 0;

	s_EmuCommandLock = 0;

	return hasCommands;
}

void clearEmuCommands()
{
	while (s_EmuCommandLock) { usleep(5000); }
	s_EmuCommandLock = 1;

	while (s_headCommand != 0)
	{
		EmuCommand *curCommand = s_headCommand;
		s_headCommand = s_headCommand->next;

		if (curCommand->cleanupCallback)
		{
			(curCommand->cleanupCallback)(curCommand);
		}
		delete curCommand;
	}

	s_EmuCommandLock = 0;
}

void processEmuCommands()
{
	while (s_EmuCommandLock) { usleep(5000); }
	s_EmuCommandLock = 1;

	EmuCommand *curCommand = s_headCommand;
	if (s_headCommand != 0)
	{
		s_headCommand = s_headCommand->next;
	}

	s_EmuCommandLock = 0;

	if (curCommand != 0)
	{
		(curCommand->runCallback)(curCommand);
		if (curCommand->cleanupCallback)
		{
			(curCommand->cleanupCallback)(curCommand);
		}
		delete curCommand;
	}
}

JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppy(JNIEnv * env, jobject obj, jint floppy)
{
	const char *filename = ConfigureParams.DiskImage.szDiskFileName[floppy];
	jstring str = (env)->NewStringUTF(filename);
	return str;
}

JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppyZip(JNIEnv * env, jobject obj, jint floppy)
{
	const char *zipname = ConfigureParams.DiskImage.szDiskZipPath[floppy];
	jstring str = (env)->NewStringUTF(zipname);
	return str;
}
