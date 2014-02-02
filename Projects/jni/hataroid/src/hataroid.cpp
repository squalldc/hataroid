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

	volatile int g_lastDialogResultValid = 0;
	volatile int g_lastDialogResult = -1;
};

static volatile bool emuUserReady = true;
static volatile bool emuReady = false;
static volatile bool emuInited = false;
static volatile bool videoReady = false;
static volatile bool envInited = false;

static int s_turboSpeed = 0;
static int s_turboPrevFrameSkips = 0;

static bool _altUpdate = false;

static void SetEmulatorOptions(JNIEnv * env, jobjectArray keyarray, jobjectArray valarray, bool apply, bool init, bool queueCommand);

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
	emuUserReady = (pause==0);
}

int hasDialogResult() { return g_lastDialogResultValid; }
int getDialogResult() { return g_lastDialogResult; }
void clearDialogResult() { g_lastDialogResultValid = 0; g_lastDialogResult = -1; }

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_libExit(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> libExit");

	exit(0);
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	g_jvm = jvm;

	return JNI_VERSION_1_6;
}

static void registerJNIcallbacks(JNIEnv * env, jobject activityInstance)
{
	g_jniAudioInterface.android_env = env;
	g_jniAudioInterface.android_mainActivity = activityInstance;

	jclass activityClass = (env)->GetObjectClass(activityInstance);

	g_jniAudioInterface.initAudio = (env)->GetMethodID(activityClass, "initAudio", "(IIII)V");
	g_jniAudioInterface.deinitAudio = (env)->GetMethodID(activityClass, "deinitAudio", "()V");

	g_jniAudioInterface.playAudio = (env)->GetMethodID(activityClass, "playAudio", "()V");
	g_jniAudioInterface.pauseAudio = (env)->GetMethodID(activityClass, "pauseAudio", "()V");

	g_jniAudioInterface.sendAudio = (env)->GetMethodID(activityClass, "sendAudio", "([S)V");

	g_jniMainInterface.android_env = env;
	g_jniMainInterface.android_mainEmuThreadEnv = NULL;
	g_jniMainInterface.android_mainActivity = activityInstance;
	g_jniMainInterface.showGenericDialog = (env)->GetMethodID(activityClass, "showGenericDialog", "(IILjava/lang/String;)V");
	g_jniMainInterface.showOptionsDialog = (env)->GetMethodID(activityClass, "showOptionsDialog", "()V");
	g_jniMainInterface.quitHataroid = (env)->GetMethodID(activityClass, "quitHataroid", "()V");
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
				if (emuUserReady)
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
	setUserEmuPaused(emuUserReady);

	//_checkEmuReady();
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetUserPaused(JNIEnv * env, jobject obj)
{
	return (emuUserReady) ? JNI_FALSE : JNI_TRUE;
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
			if (emuUserReady)
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
	ConfigureParams.Screen.bAllowOverscan = (strcmp(val, "true") == 0);
	Renderer_refreshDispParams();
}
void _optionShowIndicators(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Screen.bShowStatusbar = false;
	ConfigureParams.Screen.bShowDriveLed = false;

	if (strcmp(val, "statusbar") == 0) { ConfigureParams.Screen.bShowStatusbar = true; }
	else if (strcmp(val, "driveled") == 0) { ConfigureParams.Screen.bShowDriveLed = true; }
}
void _optionAutoInsertDiskB(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.DiskImage.bAutoInsertDiskB = (strcmp(val, "1") == 0); }
void _optionWriteProtectFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.DiskImage.nWriteProtection = (strcmp(val, "0") == 0) ? WRITEPROT_OFF : (strcmp(val, "1") == 0) ? WRITEPROT_ON : WRITEPROT_AUTO; }
void _optionFastFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.DiskImage.FastFloppy = (strcmp(val, "true") == 0); }
void _optionsSetCompatibleCPU(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.System.bCompatibleCpu = (strcmp(val, "true")==0); }
void _optionSetSoundEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.Sound.bEnableSound = (strcmp(val, "true")==0); }
void _optionSetSoundSync(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.Sound.bEnableSoundSync = (strcmp(val, "true")==0); }
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
void _optionSetRTC(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.System.bRealTimeClock = (strcmp(val, "true")==0); }
void _optionPatchTimerD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.System.bPatchTimerD = (strcmp(val, "true")==0); }
void _optionFastBoot(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.System.bFastBoot = (strcmp(val, "true")==0); }
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
void _optionSetBlitter(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.System.bBlitter = (strcmp(val, "true")==0); }
void _optionSetJoystickPort(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int port = atoi(val);
	VirtKB_SetJoystickPort(port);
}
void _optionSetJoystickAutoFire(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int port = VirtKB_GetJoystickPort();
	bool autoFire = (strcmp(val, "true")==0);
	ConfigureParams.Joysticks.Joy[1].bEnableAutoFire = autoFire;

}
void _optionSetJoystickMapArrowKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool map = (strcmp(val, "true")==0);
	VirtKB_MapJoysticksToArrowKeys(map);
}
void _optionSetBilinearFilter(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool filter = (strcmp(val, "true")==0);
	Renderer_setFilterEmuScreeen(filter);
}

void _optionSetFullScreenStretch(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool fullscreen = (strcmp(val, "true")==0);
	if (fullscreen)
	{
		Renderer_setFullScreenStretch(fullscreen);
		VirtKB_setScreenZoomMode(false);
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

void _optionBootFromHD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.HardDisk.bBootFromHardDisk = (strcmp(val, "true")==0) || (strcmp(val, "1")==0); }

void _optionACSIAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.HardDisk.bUseHardDiskImage = (strcmp(val, "true")==0) || (strcmp(val, "1")==0); }
void _optionIDEMasterAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.HardDisk.bUseIdeMasterHardDiskImage = (strcmp(val, "true")==0) || (strcmp(val, "1")==0); }
void _optionIDESlaveAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.HardDisk.bUseIdeSlaveHardDiskImage = (strcmp(val, "true")==0) || (strcmp(val, "1")==0); }
void _optionGEMDOSAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data) { ConfigureParams.HardDisk.bUseHardDiskDirectories = (strcmp(val, "true")==0) || (strcmp(val, "1")==0); }

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
	bool extraKeys = (strcmp(val, "true")==0);
	VirtKB_setExtraKeys(extraKeys);
}
void _optionSetVKBObsessionKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = (strcmp(val, "true")==0);
	_altUpdate = valSet;
	VirtKB_setObsessionKeys(valSet);
}
void _optionSetVKBHideAll(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = (strcmp(val, "true")==0);
	VirtKB_setHideAll(valSet);
}
void _optionSetVKBJoystickOnly(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = (strcmp(val, "true")==0);
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
