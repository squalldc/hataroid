//#define PROFILE_BUILD

#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <dirent.h>

#include <SDL.h>
#include <math.h>
//#include <unistd.h>

#include "hataroid.h"
#include "nativeRenderer_ogles2.h"
#include "VirtKB.h"

#include "midi/fsmidi.h"

#include <gui-android.h>

#ifdef PROFILE_BUILD
	#include <prof.h>
#endif

#define  LOG_TAG    "hataroid"

extern "C"
{
	#include <screen.h>
	#include <change.h>
	#include <dialog.h>
	#include <sound.h>
	#include <floppy.h>
	#include <gemdos.h>
	#include <hdc.h>
	#include <memorySnapShot.h>
	#include <fdc_compat.h>
	#include <main.h>

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_libExit(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationInit(JNIEnv * env, jobject obj, jobject activityInstance, jobjectArray keyarray, jobjectArray valarray);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationMain(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationDestroy(JNIEnv * env, jobject obj, jobject activityInstance);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationStartExec(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationPause(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationResume(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceCreated(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onDrawFrame(JNIEnv * env, jobject obj);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetOptions(JNIEnv * env, jobject obj, jobjectArray keyarray, jobjectArray valarray);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetCold(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetWarm(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorEjectFloppy(JNIEnv * env, jobject obj, jint floppy);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorInsertFloppy(JNIEnv * env, jobject obj, jint floppy, jstring filename, jstring zippath, jstring dispName);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateSave(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateLoad(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot);

	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveStoreOnExit(JNIEnv * env, jobject obj, jstring saveFolder);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveLoadOnStart(JNIEnv * env, jobject obj, jstring saveFolder);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleUserPaused(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetUserPaused(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetEmuPausedSoftMenu(JNIEnv * env, jobject obj, jboolean paused);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleTurboMode(JNIEnv * env, jobject obj);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetTurboMode(JNIEnv * env, jobject obj);

	JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppy(JNIEnv * env, jobject obj, jint floppy);
	JNIEXPORT jstring JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetCurFloppyZip(JNIEnv * env, jobject obj, jint floppy);

	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorIsPastiDiskResetRequired(JNIEnv * env, jobject obj, jstring filename, jstring zippath);
	JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorIsPastiDisk(JNIEnv * env, jobject obj, jstring filename, jstring zippath);

	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidDialogResult(JNIEnv * env, jobject obj, jint result);
	JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidSettingsResult(JNIEnv * env, jobject obj, jint result);

	JavaVM *g_jvm = 0;
	struct JNIAudio g_jniAudioInterface;
	struct JNIMainMethodCache g_jniMainInterface;

	extern void Ide_UnInit(void);

	extern int hatari_main_init(int argc, const char *argv[]);
	extern int hatari_main_doframe();
	extern void hatari_main_exit();
	extern void SDL_CloseAudio();
	extern bool Main_PauseEmulation(bool visualize);
	extern bool Main_UnPauseEmulation(void);
	extern void MemorySnapShot_setConfirmOnOverwriteSave(bool set);

	volatile int g_lastDialogResultValid = 0;
	volatile int g_lastDialogResult = -1;
	volatile int g_hasSettingsResult = 0;

	volatile int g_validMainActivity = 0;

	int kMaxAudioMuteFrames = 5;
	volatile int g_audioMute = 0;
	volatile int g_audioMuteFrames = 0;

	volatile int g_emuStartExec = 0;
	volatile int g_emuReady = 0;
	volatile int _runTillQuit = 0;

	int g_vsync = 0;

	extern int g_videoTex_width;
	extern int g_videoTex_height;
	extern void *g_videoTex_pixels;
	extern int g_surface_width;
	extern int g_surface_height;

	extern int gHasHataroidSaveExtra;
};

enum
{
	EMUPAUSETYPE_USER       = (1<<0),
	EMUPAUSETYPE_SOFTMENU   = (1<<1),
	//EMUPAUSETYPE_SAVE       = (1<<2),

	EMUPAUSETYPE_MASK       = 0xffffffff,
};

static volatile int emuUserPaused = 0;
static volatile bool emuInited = false;
static volatile bool videoReady = false;
static volatile bool envInited = false;

static volatile bool emuQuit = false;
static volatile bool _saveAndQuit = false;
static volatile bool _pendingQuit = false;
static volatile bool _saving = false;
static bool _crashQuit = false;

static volatile int _drawCountSinceLastUpdate = 0;

volatile int _doubleBusError = 0;

static int s_turboSpeed = 0;
static int s_turboPrevFrameSkips = 0;

static bool _altUpdate = false;
static bool _frameReady = false;

static int _shortcutAutoFire = -1;

static int _quickSaveSlot = -1;
static char _quickSavePath[FILENAME_MAX] = {0};
static char _saveDispName[FILENAME_MAX] = {0};

static int _resetSynthOnSTReset = true;

static const char* _emuTOSRomST = 0;
static int _emuTOSRomSTLen = 0;
static const char* _emuTOSRomSTE = 0;
static int _emuTOSRomSTELen = 0;


static void SetEmulatorOptions(JNIEnv * env, jobjectArray keyarray, jobjectArray valarray, bool apply, bool init, bool queueCommand);
void _storeSaveState(const char *saveMetaFilePath);
void _loadSaveState();
void _generateSaveNames(const char *savePath, int saveSlot, char* saveMetaFilePathResult);
bool _findSaveStateMetaFile(int slotID, char *resultBuf);
void _clearHataroidSaveCommands();
void _hataroidRetrieveSaveExtraData();
void _confirmSettings(JNIEnv* env);
void _resetCold();
static void _preloadEmuTOS(JNIEnv* curEnv);

static void _initAssetDataItems();
static void _deinitAssetDataItems();

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

volatile int s_loadingCommand = 0;
volatile int s_saveCommandLock = 0;
EmuCommand *s_saveCommand = 0;
void _addSaveCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data);
//---------------

static bool	_monEnabled = false;
static bool _monInited = false;
static bool _monDeInited = false;
static void _initPerfMon()
{
#ifdef PROFILE_BUILD
	if (!_monInited && _monEnabled)
	{
		Debug_Printf("init perf mon");
		#ifdef PROFILE_BUILD
			setenv("CPUPROFILE_FREQUENCY", "500", 1); // Change to 500 interrupts per second
		#endif
		monstartup("libhataroid.so");
		_monInited = true;
	}
#endif
}
static void _deinitPerfMon()
{
#ifdef PROFILE_BUILD
	if (_monInited && !_monDeInited && _monEnabled)
	{
		Debug_Printf("deinit perf mon");
		moncleanup();
		_monDeInited = true;
	}
#endif
}

//---------------

#ifdef __clang__
	clang generates slower code atm
#else
#ifdef __GNUC__
	//gcc
#endif
#endif


void Debug_Printf(const char *a_pszformat, ...)
{
	va_list args;
	va_start(args, a_pszformat);
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, a_pszformat, args);
	va_end(args);
}

void Debug_VPrintf(const char *a_pszformat, va_list args)
{
	__android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, a_pszformat, args);
}

void usleep(int usecs)
{
	struct timespec	ts;

	ts.tv_sec = usecs / 1000000;
	ts.tv_nsec = (usecs % 1000000) * 1000; // micro sec -> nano sec
	nanosleep(&ts, &ts);
}

int showGenericDialog(JNIEnv *curEnv, const char *message, int ok, int noyes, const char* noTxt, const char* yesOKTxt)
{
	Debug_Printf("Show Generic Dialog");
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	jstring str = (env)->NewStringUTF(message);
	jstring noStr = (env)->NewStringUTF(noTxt);
	jstring yesStr = (env)->NewStringUTF(yesOKTxt);
	jint dialogID = (env)->CallIntMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showGenericDialog, ok, noyes, str, noStr, yesStr);
	Debug_Printf("Show Generic Dialog Done: %d", dialogID);
	return dialogID;
}

void updateGenericDialogMessage(JNIEnv *curEnv, int dialogID, const char *message)
{
	//Debug_Printf("Update Generic Dialog Message : %d, %s", dialogID, message);
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	jstring str = (env)->NewStringUTF(message);
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.updateDialogMessage, dialogID, str);
	//Debug_Printf("Update Generic Dialog Message Done");
}

void destroyGenericDialog(JNIEnv *curEnv, int dialogID)
{
	Debug_Printf("Destroy Generic Dialog: %d", dialogID);
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.destroyGenericDialog, dialogID);
	Debug_Printf("Destroy Generic Dialog Done");
}

void showOptionsDialog(JNIEnv *curEnv)
{
	Debug_Printf("Show Options Dialog");
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showOptionsDialog);
	Debug_Printf("Show Options Dialog Done");
}

void showFloppyAInsert(JNIEnv *curEnv)
{
	Debug_Printf("Show Floppy A Dialog");
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showFloppyAInsert);
	Debug_Printf("Show Floppy A Dialog Done");
}

void showFloppyBInsert(JNIEnv *curEnv)
{
	Debug_Printf("Show Floppy B Dialog");
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showFloppyBInsert);
	Debug_Printf("Show Floppy B Dialog Done");
}

void showSoftMenu(JNIEnv *curEnv, int optionType)
{
	Debug_Printf("Show Soft Menu");
	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.showSoftMenu, optionType);
	Debug_Printf("Show Soft Menu Done");
}

static void requestQuitHataroid()
{
	Debug_Printf("Quit Hataroid");
	_deinitPerfMon();

	(g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.quitHataroid);
}

void RequestAndWaitQuit()
{
	//_saveAndQuit = false;
	//emuQuit = true;
	g_emuReady = 0;
	requestQuitHataroid();

	//exit(0);

	for (;;)
	{
		usleep(100000); // 0.1 sec
	}
}

int isUserEmuPaused()
{
	return (emuUserPaused & EMUPAUSETYPE_USER) != 0 ? 1 : 0;
}

void toggleUserEmuPaused()
{
	setUserEmuPaused(isUserEmuPaused() ? 0 : 1, EMUPAUSETYPE_USER);
}

void setUserEmuPaused(int pause, int pauseFlag)
{
	if (pause)	{ emuUserPaused |= pauseFlag; }
	else		{ emuUserPaused &= ~pauseFlag; }

	g_audioMute = 1;
	Sound_BufferIndexNeedReset = true;
}

int hasDialogResult() { return g_lastDialogResultValid; }
int getDialogResult() { return g_lastDialogResult; }
void clearDialogResult() { g_lastDialogResultValid = 0; g_lastDialogResult = -1; }

int hasSettingsResult() { return (g_hasSettingsResult != 0); }
void clearSettingsResult() { g_hasSettingsResult = 0; }

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_libExit(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> libExit");

	if (g_jniMainInterface.mainActivityGlobalRefObtained!=0)
	{
		g_validMainActivity = 0;
		g_jniMainInterface.mainActivityGlobalRefObtained = 0;
		(env)->DeleteGlobalRef(g_jniMainInterface.android_mainActivity);
	}

	exit(0);
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	g_jvm = jvm;
	g_jniMainInterface.mainActivityGlobalRefObtained = 0;
	g_validMainActivity = 0;

	return JNI_VERSION_1_6;
}

static void registerJNIcallbacks(JNIEnv * env, jobject activityInstance)
{
	jobject mainActivityRef = (env)->NewGlobalRef(activityInstance);

	jclass activityClass = (env)->GetObjectClass(activityInstance);

	{
		g_jniAudioInterface.android_env = env;
		g_jniAudioInterface.android_mainActivity = mainActivityRef;

		g_jniAudioInterface.getMinBufSize = (env)->GetMethodID(activityClass, "getMinBufSize", "(III)I");
		g_jniAudioInterface.initAudio = (env)->GetMethodID(activityClass, "initAudio", "(IIII)V");
		g_jniAudioInterface.deinitAudio = (env)->GetMethodID(activityClass, "deinitAudio", "()V");

		g_jniAudioInterface.playAudio = (env)->GetMethodID(activityClass, "playAudio", "()V");
		g_jniAudioInterface.pauseAudio = (env)->GetMethodID(activityClass, "pauseAudio", "()V");

		g_jniAudioInterface.sendAudio = (env)->GetMethodID(activityClass, "sendAudio", "([SII)V");
	}

	{
		g_jniMainInterface.android_env = env;
		g_jniMainInterface.android_mainEmuThreadEnv = NULL;
		g_jniMainInterface.android_mainActivity = mainActivityRef;
		g_jniMainInterface.showGenericDialog = (env)->GetMethodID(activityClass, "showGenericDialog", "(IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
		g_jniMainInterface.destroyGenericDialog = (env)->GetMethodID(activityClass, "destroyGenericDialog", "(I)V");
		g_jniMainInterface.updateDialogMessage = (env)->GetMethodID(activityClass, "updateDialogMessage", "(ILjava/lang/String;)V");
		g_jniMainInterface.showOptionsDialog = (env)->GetMethodID(activityClass, "showOptionsDialog", "()V");
		g_jniMainInterface.showFloppyAInsert = (env)->GetMethodID(activityClass, "showFloppyAInsert", "()V");
		g_jniMainInterface.showFloppyBInsert = (env)->GetMethodID(activityClass, "showFloppyBInsert", "()V");
		g_jniMainInterface.showSoftMenu = (env)->GetMethodID(activityClass, "showSoftMenu", "(I)V");
		g_jniMainInterface.quitHataroid = (env)->GetMethodID(activityClass, "quitHataroid", "()V");
		g_jniMainInterface.setConfigOnSaveStateLoad = (env)->GetMethodID(activityClass, "setConfigOnSaveStateLoad", "([Ljava/lang/String;)V");
		g_jniMainInterface.getAssetData = (env)->GetMethodID(activityClass, "getAssetData", "(Ljava/lang/String;)[B");

		//g_jniMainInterface.sendAndMidiAudio = (env)->GetMethodID(activityClass, "sendAndMidiAudio", "()V");
		g_jniMainInterface.sendMidiByte = (env)->GetMethodID(activityClass, "sendMidiByte", "(B)V");

		g_jniMainInterface.mainActivityGlobalRefObtained = 1;
	}

	g_validMainActivity = 1;
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

		fsMidi_init();
		_preloadEmuTOS(g_jniMainInterface.android_mainEmuThreadEnv);

		const char *argv[] = { "Hataroid" };
		hatari_main_init(1, argv);

		emuInited = true;
		_drawCountSinceLastUpdate = 0;
	}
}

static void _checkEmuReady()
{
	if (!g_emuReady)
	{
		g_emuReady = videoReady && emuInited && envInited && g_emuStartExec;// && emuUserReady;
		if (g_emuReady)
		{
			VirtKB_EnableInput(true);
			Sound_BufferIndexNeedReset = true;

			_initPerfMon();
		}
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidDialogResult(JNIEnv * env, jobject obj, jint result)
{
	g_lastDialogResultValid = 1;
	g_lastDialogResult = result;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_hataroidSettingsResult(JNIEnv * env, jobject obj, jint result)
{
	g_hasSettingsResult = 1;
}

JNIEnv *s_optionsEnv = 0;
jobjectArray s_optionsKeyArray = 0;
jobjectArray s_optionsValarray = 0;
bool s_optionsIniting = false;

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationInit(JNIEnv * env, jobject obj, jobject activityInstance, jobjectArray keyarray, jobjectArray valarray)
{
	Debug_Printf("----> emulationInit");

	_initAssetDataItems();

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
		if (g_emuReady && !emuQuit)
		{
			_drawCountSinceLastUpdate = 0;

			if (!_saveAndQuit && g_audioMute && s_loadingCommand == 0)
			{
				g_audioMuteFrames = kMaxAudioMuteFrames;
				g_audioMute = 0;
			}

			if (_altUpdate)
			{
				usleep(1);
				continue;
			}

			//if (!_altUpdate)
			if (/*!g_vsync ||*/ _frameReady || _runTillQuit)
			{
				//_frameReady = false;

				fsMidi_update();

				if (emuUserPaused == 0 || _saveAndQuit)
				{
					hatari_main_doframe();
				}

				//fsMidi_postUpdate();

				SDL_UpdateRects(sdlscrn, 0, 0);
				processEmuCommands();
				_confirmSettings(env);

				if (_pendingQuit)
				{
					Debug_Printf("----> Pending Quit...");
					_pendingQuit = false;
					g_emuReady = false;
					requestQuitHataroid();
					//RequestAndWaitQuit();
				}
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

	g_validMainActivity = 0;

	_deinitPerfMon();

	fsMidi_deinit();

	if (emuInited)
	{
		Debug_Printf("----> floppy uninit");
		Floppy_UnInit();
		Debug_Printf("----> hdc uninit");
		HDC_UnInit();
		//Debug_Printf("----> nvram uninit"); // Falcon
		//NvRam_UnInit();
		Debug_Printf("----> gemdos uninit");
		GemDOS_UnInitDrives();
		Debug_Printf("----> ide uninit");
		Ide_UnInit();
	}

	Debug_Printf("----> hataroid uninit ");

	Main_UnPauseEmulation();

	VirtKB_EnableInput(false);

	g_emuReady = false;
	videoReady = false;
	emuInited = false;
	envInited = false;
	emuQuit = true;

	Debug_Printf("----> SDL_CloseAudio");

	if (g_jniMainInterface.mainActivityGlobalRefObtained!=0)
	{
		g_jniMainInterface.mainActivityGlobalRefObtained = 0;
		(env)->DeleteGlobalRef(g_jniMainInterface.android_mainActivity);
	}

	memset(&g_jniAudioInterface, 0, sizeof(JNIAudio));
	registerJNIcallbacks(env, activityInstance);

	SDL_CloseAudio();

	_deinitAssetDataItems();

	//hatari_main_exit();

	Debug_Printf("----> emulationDestroy Done");
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationStartExec(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationStartExec");

	g_emuStartExec = true;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationPause(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationPause");

	setTurboSpeed(0);
	VirtKB_EnableInput(false);

	g_emuReady = false;
	videoReady = false;
	_frameReady = false;

	g_audioMute = 1;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleUserPaused(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulation USER Pause toggle");

	//setTurboSpeed(0);
	//VirtKB_EnableInput(false);

	toggleUserEmuPaused();

	//_checkEmuReady();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetEmuPausedSoftMenu(JNIEnv * env, jobject obj, jboolean paused)
{
	Debug_Printf("----> emulation USER Pause toggle");

	//setTurboSpeed(0);
	//VirtKB_EnableInput(false);

	setUserEmuPaused(paused?1:0, EMUPAUSETYPE_SOFTMENU);

	//_checkEmuReady();
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetUserPaused(JNIEnv * env, jobject obj)
{
	return ((emuUserPaused & EMUPAUSETYPE_USER) != 0);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulationResume(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> emulationResume");
	if (!g_emuReady)
	{
		g_audioMute = 1;
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorToggleTurboMode(JNIEnv * env, jobject obj)
{
	setTurboSpeed(1 - getTurboSpeed());
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorGetTurboMode(JNIEnv * env, jobject obj)
{
	return (getTurboSpeed() != 0);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceCreated(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> onSurfaceCreated");
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height)
{
	Debug_Printf("----> onSurfaceChanged: (%d x %d)", width, height);

	{
		setupGraphics(width, height, env);
		VirtKB_OnSurfaceChanged(width, height);
		videoReady = true;
	}

	_checkEmuReady();
}

bool _checkCrash()
{
	if (g_emuReady && _saveAndQuit && !_pendingQuit && !_doubleBusError && !_crashQuit && !_saving)
	{
		++_drawCountSinceLastUpdate;
		if (_drawCountSinceLastUpdate > 180)
		{
			Debug_Printf("*** CRASH FORCE QUITTING...");
			_crashQuit = true;
			//Main_SetCPUBrk();
			//requestQuitHataroid();
			return true;
		}
	}
	return false;
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_onDrawFrame(JNIEnv * env, jobject obj)
{
	if (g_emuReady)
	{
/*
		if (_altUpdate)
		{
			if (emuUserPaused == 0 || _saveAndQuit)
			{
				hatari_main_doframe();
			}
			if (emuQuit)
			{
				return;
			}
			SDL_UpdateRects(sdlscrn, 0, 0);
			processEmuCommands();
		}
		if (emuQuit)
		{
			return;
		}
//*/
		renderFrame(env);
		_frameReady = true;

		if (_checkCrash())
		{
			return true;
		}
	}

	return false;
}

//extern "C" void _sendAndMidiAudio()
//{
//	(g_jniMainInterface.android_mainEmuThreadEnv)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.sendAndMidiAudio);
//}

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
typedef bool (*OptionValCallback)(const OptionSetting *setting, char *dstBuf, int dstBufLen);
struct OptionSetting
{
	const char			*key;
	OptionCallback		callback;
	OptionValCallback	valCallback;
};

bool _getBoolVal(const char *val) { return !((strcasecmp(val, "false") == 0) || (strcmp(val, "0") == 0)); }

float _getOptionFloatPercent(const char *val, float fmin, float fmax)
{
	float f = atof(val);
	f *= (1.0f/100.0f);

	if (f < fmin) { f = fmin; }
	else if (f > fmax) { f = fmax; }

	return f;
}

const char *findOptionsVal(const char *optionsKey, EmuCommandSetOptions_Data *data)
{
	for (int i = 0; i < data->numKeys; ++i)
	{
		const char *curKey = data->keys[i];
		if (strcmp(curKey, optionsKey)==0)
		{
			const char *curVal = data->vals[i];
			return curVal;
		}
	}
	return 0;
}


bool _optionValMachineType(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.System.nMachineType == MACHINE_FALCON)		{ strncpy(dstBuf, "Falcon", dstBufLen); return true; }
	else if (ConfigureParams.System.nMachineType == MACHINE_STE)	{ strncpy(dstBuf, "STE", dstBufLen); return true; }
	else if (ConfigureParams.System.nMachineType == MACHINE_TT)		{ strncpy(dstBuf, "TT", dstBufLen); return true; }
	else if (ConfigureParams.System.nMachineType == MACHINE_ST)		{ strncpy(dstBuf, "ST", dstBufLen); return true; }
	return false;
}

bool _optionValTOSEmuTOS(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
    strncpy(dstBuf, ConfigureParams.Hataroid.useEmuTOS?"true":"false", dstBufLen);
    return true;
}
void _optionTOSEmuTOS(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    ConfigureParams.Hataroid.useEmuTOS = _getBoolVal(val) ? 1 : 0;
}

bool _optionValTOSST(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.System.nMachineType == MACHINE_ST) { strncpy(dstBuf, ConfigureParams.Rom.szTosImageFileName, dstBufLen); return true; }
	return false;
}
bool _optionValTOSSTE(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.System.nMachineType == MACHINE_STE) { strncpy(dstBuf, ConfigureParams.Rom.szTosImageFileName, dstBufLen); return true; }
	return false;
}

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
		const char *curVal = findOptionsVal(tospref, data);
		if (curVal != NULL && strlen(curVal) > 0)
		{
			strncpy(ConfigureParams.Rom.szTosImageFileName, curVal, FILENAME_MAX);
			ConfigureParams.Rom.szTosImageFileName[FILENAME_MAX-1]=0;
		}
	}
}

bool _optionValShowBorders(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Screen.bAllowOverscan?"true":"false", dstBufLen);
	return true;
}

void _optionShowBorders(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Screen.bAllowOverscan = _getBoolVal(val);
	Renderer_refreshDispParams();
}

bool _optionValShowIndicators(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.Screen.bShowStatusbar) { strncpy(dstBuf, "statusbar", dstBufLen); return true; }
	if (ConfigureParams.Screen.bShowDriveLed) { strncpy(dstBuf, "driveled", dstBufLen); return true; }
	return false;
}

void _optionShowIndicators(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Screen.bShowStatusbar = false;
	ConfigureParams.Screen.bShowDriveLed = false;

	if (strcmp(val, "statusbar") == 0) { ConfigureParams.Screen.bShowStatusbar = true; }
	else if (strcmp(val, "driveled") == 0) { ConfigureParams.Screen.bShowDriveLed = true; }
}

bool _optionValEnableDriveA(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.DiskImage.EnableDriveA?"true":"false", dstBufLen);
	return true;
}

void _optionEnableDriveA(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.EnableDriveA = _getBoolVal(val);
}

bool _optionValEnableDriveB(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.DiskImage.EnableDriveB?"true":"false", dstBufLen);
	return true;
}

void _optionEnableDriveB(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.EnableDriveB = _getBoolVal(val);
}

bool _optionValAutoInsertDiskB(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.DiskImage.bAutoInsertDiskB?"true":"false", dstBufLen);
	return true;
}

void _optionAutoInsertDiskB(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.bAutoInsertDiskB = _getBoolVal(val);
}

bool _optionValWriteProtectFloppy(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.DiskImage.nWriteProtection == WRITEPROT_OFF)		{ strncpy(dstBuf, "0", dstBufLen); return true; }
	else if (ConfigureParams.DiskImage.nWriteProtection == WRITEPROT_ON)	{ strncpy(dstBuf, "1", dstBufLen); return true; }
	else if (ConfigureParams.DiskImage.nWriteProtection == WRITEPROT_AUTO)	{ strncpy(dstBuf, "2", dstBufLen); return true; }
	return false;
}

void _optionWriteProtectFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.nWriteProtection = (strcmp(val, "0") == 0) ? WRITEPROT_OFF : (strcmp(val, "1") == 0) ? WRITEPROT_ON : WRITEPROT_AUTO;
}

bool _optionValFastFloppy(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.DiskImage.FastFloppy?"true":"false", dstBufLen);
	return true;
}

void _optionFastFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.DiskImage.FastFloppy = _getBoolVal(val);
}

bool _optionValLegacyFloppy(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
    strncpy(dstBuf, ConfigureParams.Hataroid.legacyFloppy?"true":"false", dstBufLen);
    return true;
}
void _optionLegacyFloppy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    ConfigureParams.Hataroid.legacyFloppy = _getBoolVal(val) ? 1 : 0;
}

bool _optionValCompatibleCPU(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.System.bCompatibleCpu?"true":"false", dstBufLen);
	return true;
}

void _optionsSetCompatibleCPU(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
//#if 0 // crashes Hatari windows as well if switch to compat while running
	ConfigureParams.System.bCompatibleCpu = _getBoolVal(val);
//#endif
}

bool _optionValSoundEnabled(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Sound.bEnableSound?"true":"false", dstBufLen);
	return true;
}

void _optionSetSoundEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Sound.bEnableSound = _getBoolVal(val);
}

bool _optionValSoundSync(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Sound.bEnableSoundSync?"true":"false", dstBufLen);
	return true;
}

void _optionSetSoundSync(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Sound.bEnableSoundSync = _getBoolVal(val);
}

bool _optionValSoundMute(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Hataroid.muteSTSound?"true":"false", dstBufLen);
	return true;
}

void _optionSetSoundMute(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Hataroid.muteSTSound = _getBoolVal(val) ? 1 : 0;
}

bool _optionValSoundQuality(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.Sound.nPlaybackFreq);
	return true;
}

void _optionSetSoundQuality(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int freq = atoi(val);
	ConfigureParams.Sound.nPlaybackFreq = freq;

	fsMidi_setSetting(freq, -1, -1, -1, 0);
}

bool _optionValYMVoicesMixing(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.Sound.YmVolumeMixing == YM_MODEL_MIXING)		{ strncpy(dstBuf, "math", dstBufLen); return true; }
	else if (ConfigureParams.Sound.YmVolumeMixing == YM_TABLE_MIXING)	{ strncpy(dstBuf, "st", dstBufLen); return true; }
	else if (ConfigureParams.Sound.YmVolumeMixing == YM_LINEAR_MIXING)	{ strncpy(dstBuf, "linear", dstBufLen); return true; }
	return false;
}

void _optionSetYMVoicesMixing(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "math") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_MODEL_MIXING;
	else if (strcmp(val, "st") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_TABLE_MIXING;
	else if (strcmp(val, "linear") == 0) ConfigureParams.Sound.YmVolumeMixing = YM_LINEAR_MIXING;
}

bool _optionValSoundBufferSize(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.Hataroid.deviceSoundBufSize);
	return true;
}

void _optionSoundBufferSize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int bufSize = atoi(val);
	if (bufSize < 1)		{ bufSize = 1; }
	else if (bufSize > 50) {  bufSize = 50; }
	ConfigureParams.Hataroid.deviceSoundBufSize = bufSize;
}

void _optionSoundDownmixStereoMono(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Hataroid.downmixStereo = _getBoolVal(val) ? 1 : 0;
}

bool _optionValMonitorType(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_MONO)		{ strncpy(dstBuf, "Mono", dstBufLen); return true; }
	else if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_RGB)	{ strncpy(dstBuf, "RGB", dstBufLen); return true; }
	else if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_VGA)	{ strncpy(dstBuf, "VGA", dstBufLen); return true; }
	else if (ConfigureParams.Screen.nMonitorType == MONITOR_TYPE_TV)	{ strncpy(dstBuf, "TV", dstBufLen); return true; }
	return false;
}

void _optionSetMonitorType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "Mono") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_MONO;
	else if (strcmp(val, "RGB") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_RGB;
	else if (strcmp(val, "VGA") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_VGA;
	else if (strcmp(val, "TV") == 0) ConfigureParams.Screen.nMonitorType = MONITOR_TYPE_TV;
}

bool _optionValFrameSkip(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.Screen.nFrameSkips);
	return true;
}

void _optionSetFrameSkip(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int fskip = atoi(val); // 5 = auto (AUTO_FRAMESKIP_LIMIT)
	ConfigureParams.Screen.nFrameSkips = fskip;
}

bool _optionValSetRTC(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.System.bRealTimeClock?"true":"false", dstBufLen);
	return true;
}

void _optionSetRTC(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bRealTimeClock = _getBoolVal(val);
}

bool _optionValPatchTimerD(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.System.bPatchTimerD?"true":"false", dstBufLen);
	return true;
}

void _optionPatchTimerD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bPatchTimerD = _getBoolVal(val);
}

bool _optionValFastBoot(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.System.bFastBoot?"true":"false", dstBufLen);
	return true;
}

void _optionFastBoot(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.System.bFastBoot = _getBoolVal(val);
}

bool _optionValMemorySize(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.Memory.nMemorySize);
	return true;
}

void _optionMemorySize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int memsize = atoi(val); // Mb (0 = 512k)
	ConfigureParams.Memory.nMemorySize = memsize;
}

bool _optionValCPUType(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	const char* cpuTypeNames[] = { "68000", "68010", "68020", "68EC030FPU", "68040" };
	const int numCPUTypeNames = sizeof(cpuTypeNames)/sizeof(char*);
	if (ConfigureParams.System.nCpuLevel >= 0 && ConfigureParams.System.nCpuLevel < numCPUTypeNames)
	{
		strncpy(dstBuf, cpuTypeNames[ConfigureParams.System.nCpuLevel], dstBufLen);
		return true;
	}
	return false;
}

void _optionSetCPUType(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (strcmp(val, "68000") == 0) ConfigureParams.System.nCpuLevel = 0;
	else if (strcmp(val, "68010") == 0) ConfigureParams.System.nCpuLevel = 1;
	else if (strcmp(val, "68020") == 0) ConfigureParams.System.nCpuLevel = 2;
	else if (strcmp(val, "68EC030FPU") == 0) ConfigureParams.System.nCpuLevel = 3;
	else if (strcmp(val, "68040") == 0) ConfigureParams.System.nCpuLevel = 4;
}

bool _optionValCPUFreq(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.System.nCpuFreq);
	return true;
}

void _optionSetCPUFreq(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int freq = atoi(val);
	ConfigureParams.System.nCpuFreq = freq;
}

bool _optionValBlitter(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.System.bBlitter?"true":"false", dstBufLen);
	return true;
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

static void _setAutoFireSpeed(int joyID, int spd)
{
	ConfigureParams.Joysticks.Joy[joyID].bEnableAutoFire = (spd==8);
	ConfigureParams.Joysticks.Joy[joyID].bEnableAutoFireFast = (spd==4);
}
static int _getAutoFireSpeed(int joyID)
{
	return (ConfigureParams.Joysticks.Joy[joyID].bEnableAutoFireFast) ? 4
			 : (ConfigureParams.Joysticks.Joy[joyID].bEnableAutoFire) ? 8
			 : 0;
}
bool _optionValAutoFire(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	int autoFireSpeed = _getAutoFireSpeed(1);
	snprintf(dstBuf, dstBufLen, "%d", autoFireSpeed);
	return true;
}
void _optionSetJoystickAutoFire(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	//int port = VirtKB_GetJoystickPort();
	//bool autoFire = _getBoolVal(val);
	int autoFireSpeed = (val!=0) ? atoi(val) : 0;
	if (_shortcutAutoFire >= 0)
	{
		autoFireSpeed = (_shortcutAutoFire != 0) ? 8 : 0;
	}
	_setAutoFireSpeed(1, autoFireSpeed);
}
int getShortcutAutoFire()
{
	return _shortcutAutoFire;
}

void setShortcutAutoFire(int enable, int set)
{
	_shortcutAutoFire = enable ? (set ? 1 : 0) : -1;
	bool autoFire = (_shortcutAutoFire > 0);
	_setAutoFireSpeed(1, autoFire ? 8 : 0);
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

void _optionSetVSync(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
//	bool sync = _getBoolVal(val);
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

void _optionSetDisplayShader(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	const char *shaderName = val;
	Renderer_setScreenShader(shaderName);
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

void _optionSetOnScreenJoyAlpha(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float alpha = atof(val);
	alpha *= (1.0f/100.0f);

	if (alpha < 0.0f) { alpha = 0.0f; }
	else if (alpha > 1.0f) { alpha = 1.0f; }

	VirtKB_SetJoystickAlpha(alpha);
}

void _optionSetJoystickSize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetJoystickSize(size);
}

void _optionSetJoystickFireSize(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetJoystickFireSize(size);
}

void _optionSetJoystickVJStick(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    bool useVJStick = _getBoolVal(val);
    VirtKB_VJStickEnable(useVJStick);
}

void _optionSetJoystickVJStickFloating(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    bool vjStickFloating = _getBoolVal(val);
    VirtKB_VJStickSetFloating(vjStickFloating);
}

void _optionSetJoystickVJStickDeadZone(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    float size = _getOptionFloatPercent(val, 0.0f, 0.99f);
    VirtKB_VJStickSetDeadZone(size);
}

void _optionSetJoystickVJStickDiagSensitivity(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    float sensitivity = _getOptionFloatPercent(val, 0.0f, 1.0f);
    VirtKB_VJStickSetDiagSensitivity(sensitivity);
}

bool _optionValBootFromHD(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.bBootFromHardDisk?"true":"false", dstBufLen);
	return true;
}

void _optionBootFromHD(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bBootFromHardDisk = _getBoolVal(val);
}

bool _optionValACSIAttach(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Acsi[0].bUseDevice?"true":"false", dstBufLen);
	return true;
}

void _optionACSIAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.Acsi[0].bUseDevice = _getBoolVal(val);
}

bool _optionValIDEMasterAttach(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.bUseIdeMasterHardDiskImage?"true":"false", dstBufLen);
	return true;
}

void _optionIDEMasterAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseIdeMasterHardDiskImage = _getBoolVal(val);
}

bool _optionValIDESlaveAttach(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.bUseIdeSlaveHardDiskImage?"true":"false", dstBufLen);
	return true;
}

void _optionIDESlaveAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseIdeSlaveHardDiskImage = _getBoolVal(val);
}

bool _optionValGEMDOSAttach(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.bUseHardDiskDirectories?"true":"false", dstBufLen);
	return true;
}

void _optionGEMDOSAttach(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	ConfigureParams.HardDisk.bUseHardDiskDirectories = _getBoolVal(val);
}

bool _optionValACSIImage(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.Acsi[0].sDeviceFile, dstBufLen);
	return true;
}

void _optionACSIImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.Acsi[0].sDeviceFile, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.Acsi[0].sDeviceFile[FILENAME_MAX-1]=0;
}

bool _optionValIDEMasterImage(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.szIdeMasterHardDiskImage, dstBufLen);
	return true;
}

void _optionIDEMasterImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szIdeMasterHardDiskImage, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szIdeMasterHardDiskImage[FILENAME_MAX-1]=0;
}

bool _optionValIDESlaveImage(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.szIdeSlaveHardDiskImage, dstBufLen);
	return true;
}

void _optionIDESlaveImage(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szIdeSlaveHardDiskImage, ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szIdeSlaveHardDiskImage[FILENAME_MAX-1]=0;
}

bool _optionValGEMDOSFolder(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HardDisk.szHardDiskDirectories[0], dstBufLen);
	return true;
}

void _optionGEMDOSFolder(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(ConfigureParams.HardDisk.szHardDiskDirectories[0], ((val==0)||strcmp(val,"none")==0) ? "" : val, FILENAME_MAX);
	ConfigureParams.HardDisk.szHardDiskDirectories[0][FILENAME_MAX-1]=0;
}

bool _optionValGEMDOSWriteProtection(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	if (ConfigureParams.HardDisk.nWriteProtection == WRITEPROT_OFF)			{ strncpy(dstBuf, "0", dstBufLen); return true; }
	else if (ConfigureParams.HardDisk.nWriteProtection == WRITEPROT_ON	)	{ strncpy(dstBuf, "1", dstBufLen); return true; }
	else if (ConfigureParams.HardDisk.nWriteProtection == WRITEPROT_AUTO)	{ strncpy(dstBuf, "2", dstBufLen); return true; }
	return false;
}

void _optionGEMDOSWriteProtection(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int optionVal = atoi(val);

	if (optionVal == 0) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_OFF; } // off
	else if (optionVal == 1) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_ON; } // on
	else if (optionVal == 2) { ConfigureParams.HardDisk.nWriteProtection = WRITEPROT_AUTO; } // auto
}

//void _optionSetVKBExtraKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
//{
//	bool extraKeys = _getBoolVal(val);
//	VirtKB_setExtraKeys(extraKeys);
//}

void _optionSetVKBObsessionKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (_saveAndQuit)
	{
		return;
	}
	bool valSet = _getBoolVal(val);
//	_altUpdate = valSet;
	VirtKB_setObsessionKeys(valSet);
}

void _optionSetVKBAutoHide(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    bool valSet = _getBoolVal(val);
    VirtKB_setAutoHide(valSet);
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

void _optionVKBHideJoy(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	VirtKB_setHideJoystick(valSet);
}

void _optionVKBKeySizeVX(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetKeySizeVX(size);
}

void _optionVKBKeySizeVY(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetKeySizeVY(size);
}

void _optionVKBKeySizeHX(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetKeySizeHX(size);
}

void _optionVKBKeySizeHY(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	float size = _getOptionFloatPercent(val, 0.1f, 3.0f);
	VirtKB_SetKeySizeHY(size);
}

void _optionSetVKBHideExtraJoyKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	VirtKB_setHideExtraJoyKeys(valSet);
}

void _optionSetVKBHideShortcutKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool valSet = _getBoolVal(val);
	VirtKB_setHideShortcutKeys(valSet);
}

//void _optionSetVKBHideTurboKey(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
//{
//	bool valSet = _getBoolVal(val);
//	VirtKB_setHideTurboKeys(valSet);
//}

void _optionSetSaveStateFolder(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	strncpy(_quickSavePath, val, FILENAME_MAX);
	_quickSavePath[FILENAME_MAX-1] = 0;
}

void _optionSetQuickSaveSlot(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int slot = atoi(val);
	_quickSaveSlot = slot;
}

void _optionVKBSetShortcutKeys(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	VirtKB_setShortcutKeysFromPrefs(val);
}

void _optionMidiOutEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	fsMidi_setMidiOutEnabled(_getBoolVal(val) ? 1 : 0);
}

bool _optionValMidiOutEnabled(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HataroidMidi.midiOutEnabled?"true":"false", dstBufLen);
	return true;
}

void _optionMidiResetOnSTReset(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	_resetSynthOnSTReset = _getBoolVal(val);
}

bool _optionValMidiResetOnSTReset(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HataroidMidi.midiResetOnSTReset?"true":"false", dstBufLen);
	return true;
}

/*
void _optionMidiDeviceAndroid(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (_getBoolVal(val)) { fsMidi_setDevice(MIDI_DEVICE_ANDROID); }
}

void _optionMidiDeviceFluidSynth(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (_getBoolVal(val)) { fsMidi_setDevice(MIDI_DEVICE_FLUIDSYNTH); }
}
*/

void _optionMidiFluidSynthUseCustomSF(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	bool useCustomSF = _getBoolVal(val);
	const char *sf = "asset://sf2/Florestan_Basic_GM_GS.sf2";

	if (useCustomSF)
	{
		const char *customSF = findOptionsVal("pref_midi_fluidsynth_soundfont", data);
		if (customSF != NULL && strlen(customSF) > 0)
		{
			sf = customSF;
		}
	}

	fsMidi_setSoundFont(sf);
}

void _optionMidiFluidSynthSoundFont(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	if (val != 0 && strlen(val) > 0)
	{
		fsMidi_setSoundFont(val);
	}
}

void _optionMidiFluidSynthReverb(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int reverb = (_getBoolVal(val) ? 1 : 0);
	fsMidi_setSetting(0, reverb, -1, -1, 0);
}

void _optionMidiFluidSynthChorus(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int chorus = (_getBoolVal(val) ? 1 : 0);
	fsMidi_setSetting(0, -1, chorus, -1, 0);
}

void _optionMidiFluidSynthInterp(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int interp = (val!=0) ? atoi(val) : 0;
	fsMidi_setSetting(0, -1, -1, interp, 0);
}

void _optionMidiFluidSynthMaxPoly(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int maxPoly = (val!=0) ? atoi(val) : 0;
	fsMidi_setSetting(0, -1, -1, -1, maxPoly);
}

void _optionMidiFluidSynthVolGain(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int volGain = (val!=0) ? atoi(val) : 0;
	fsMidi_setSettingVolGain(volGain);
}

void _optionMidiIgnoreProgramChanges(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int ignore = (_getBoolVal(val) ? 1 : 0);
	fsMidi_setIgnoreProgramChanges(ignore);
}

bool _optionValIgnoreProgramChanges(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	strncpy(dstBuf, ConfigureParams.HataroidMidi.midiIgnoreProgramChanges?"true":"false", dstBufLen);
	return true;
}

void _optionMidiTranspose(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
	int transpose = (val!=0) ? atoi(val) : 0;
	fsMidi_setMidiTranspose(transpose);
}

bool _optionValMidiTranspose(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
	snprintf(dstBuf, dstBufLen, "%d", ConfigureParams.HataroidMidi.midiChanTranspose);
	return true;
}

void _optionMidiHardwareOutEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    fsMidi_setMidiHardwareOutEnabled(_getBoolVal(val) ? 1 : 0);
}

bool _optionValMidiHardwareOutEnabled(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
    strncpy(dstBuf, ConfigureParams.HataroidMidi.midiHardwareOutEnabled?"true":"false", dstBufLen);
    return true;
}

void _optionMidiHardwareInEnabled(const OptionSetting *setting, const char *val, EmuCommandSetOptions_Data *data)
{
    fsMidi_setMidiHardwareInEnabled(_getBoolVal(val) ? 1 : 0);
}

bool _optionValMidiHardwareInEnabled(const OptionSetting *setting, char *dstBuf, int dstBufLen)
{
    strncpy(dstBuf, ConfigureParams.HataroidMidi.midiHardwareInEnabled?"true":"false", dstBufLen);
    return true;
}

static const OptionSetting s_OptionsMap[] =
{
	{ "pref_input_joystick_port", _optionSetJoystickPort, 0 },
	{ "pref_input_joysticks_autofirespd", _optionSetJoystickAutoFire, _optionValAutoFire },
	{ "pref_input_joysticks_maparrowkeys", _optionSetJoystickMapArrowKeys, 0 },
	{ "pref_input_joysticks_size", _optionSetJoystickSize, 0 },
	{ "pref_input_joysticks_fire_size", _optionSetJoystickFireSize, 0 },
    { "pref_input_joysticks_vjstick", _optionSetJoystickVJStick, 0 },
    { "pref_input_joysticks_vjstick_floating", _optionSetJoystickVJStickFloating, 0 },
    { "pref_input_joysticks_vjstick_deadzone", _optionSetJoystickVJStickDeadZone, 0 },
    { "pref_input_joysticks_vjstick_diag_sensitivity", _optionSetJoystickVJStickDiagSensitivity, 0 },
	{ "pref_input_mouse_emutype", _optionSetMouseEmuType, 0 },
	{ "pref_input_mouse_speed", _optionSetMouseSpeed, 0 },
	{ "pref_input_onscreen_alpha", _optionSetOnScreenAlpha, 0 },
	{ "pref_input_onscreen_joy_alpha", _optionSetOnScreenJoyAlpha, 0 },
	{ "pref_system_blitteremulation", _optionSetBlitter, _optionValBlitter },
	{ "pref_system_cpuclock", _optionSetCPUFreq, _optionValCPUFreq },
	{ "pref_system_cputype", _optionSetCPUType, _optionValCPUType },
	{ "pref_system_compatiblecpu", _optionsSetCompatibleCPU, _optionValCompatibleCPU },
	{ "pref_system_machinetype", _optionSetMachineType, _optionValMachineType },
	{ "pref_system_memory", _optionMemorySize, _optionValMemorySize },
	{ "pref_system_patchtimerd", _optionPatchTimerD, _optionValPatchTimerD },
	{ "pref_system_patchtosfasterboot", _optionFastBoot, _optionValFastBoot },
	{ "pref_system_printeremulation", 0, 0 },
	{ "pref_system_rs232emulation", 0, 0 },
	{ "pref_system_rtc", _optionSetRTC, _optionValSetRTC },
    { "pref_system_tos_emutos", _optionTOSEmuTOS, _optionValTOSEmuTOS },
	{ "pref_system_tos", 0, _optionValTOSST },
	{ "pref_system_tos_ste", 0, _optionValTOSSTE },
	{ "pref_display_bilinearfilter", _optionSetBilinearFilter, 0 },
	{ "pref_display_vsync", _optionSetVSync, 0 },
	{ "pref_display_fullscreen", _optionSetFullScreenStretch, 0 },
	{ "pref_display_keepscreenawake", 0, 0 },
	{ "pref_display_shader", _optionSetDisplayShader, 0 },
	{ "pref_display_extendedvdi", 0, 0 },
	{ "pref_display_extendedvdi_colors", 0, 0 },
	{ "pref_display_extendedvdi_resolution", 0, 0 },
	{ "pref_storage_cartridge", 0, 0 },
	{ "pref_display_frameskip", _optionSetFrameSkip, _optionValFrameSkip },
	{ "pref_display_indicators", _optionShowIndicators, _optionValShowIndicators },
	{ "pref_display_monitortype", _optionSetMonitorType, _optionValMonitorType },
	{ "pref_display_showborders", _optionShowBorders, _optionValShowBorders },
	{ "pref_storage_floppydisks_enabledrivea", _optionEnableDriveA, _optionValEnableDriveA},
	{ "pref_storage_floppydisks_enabledriveb", _optionEnableDriveB, _optionValEnableDriveB},
	{ "pref_storage_floppydisks_autoinsertb", _optionAutoInsertDiskB, _optionValAutoInsertDiskB },
	{ "pref_storage_floppydisks_fastfloppyaccess", _optionFastFloppy, _optionValFastFloppy },
	{ "pref_storage_floppydisks_legacy", _optionLegacyFloppy, _optionValLegacyFloppy },
	{ "pref_storage_floppydisks_writeprotection", _optionWriteProtectFloppy, _optionValWriteProtectFloppy },
	{ "pref_storage_harddisks_acsiimage", _optionACSIImage, _optionValACSIImage },
	{ "pref_storage_harddisks_acsi_attach", _optionACSIAttach, _optionValACSIAttach },
	{ "pref_storage_harddisks_bootfromharddisk", _optionBootFromHD, _optionValBootFromHD },
	{ "pref_storage_harddisks_gemdosdrive", _optionGEMDOSFolder, _optionValGEMDOSFolder },
	{ "pref_storage_harddisks_gemdosdrive_attach", _optionGEMDOSAttach, _optionValGEMDOSAttach },
	{ "pref_storage_harddisks_gemdoswriteprotection", _optionGEMDOSWriteProtection, _optionValGEMDOSWriteProtection },
	{ "pref_storage_harddisks_idemasterimage", _optionIDEMasterImage, _optionValIDEMasterImage },
	{ "pref_storage_harddisks_idemaster_attach", _optionIDEMasterAttach, _optionValIDEMasterAttach },
	{ "pref_storage_harddisks_ideslaveimage", _optionIDESlaveImage, _optionValIDESlaveImage },
	{ "pref_storage_harddisks_ideslave_attach", _optionIDESlaveAttach, _optionValIDESlaveAttach },
	{ "pref_sound_enabled", _optionSetSoundEnabled, _optionValSoundEnabled },
	{ "pref_sound_quality", _optionSetSoundQuality, _optionValSoundQuality },
	{ "pref_sound_synchronize_enabled", _optionSetSoundSync, _optionValSoundSync },
	{ "pref_sound_mute", _optionSetSoundMute, _optionValSoundMute },
	{ "pref_sound_ymvoicesmixing", _optionSetYMVoicesMixing, _optionValYMVoicesMixing },
	{ "pref_sound_buffer_size", _optionSoundBufferSize, _optionValSoundBufferSize },
	{ "pref_sound_downmix_enabled", _optionSoundDownmixStereoMono, 0 },
	//{ "pref_input_keyboard_extra_keys", _optionSetVKBExtraKeys, 0 },
	{ "pref_input_keyboard_obsession_keys", _optionSetVKBObsessionKeys, 0 },
    { "pref_input_onscreen_autohide", _optionSetVKBAutoHide, 0 },
	{ "pref_input_onscreen_hide_all", _optionSetVKBHideAll, 0 },
	{ "pref_input_onscreen_only_joy", _optionSetVKBJoystickOnly, 0 },
	{ "pref_input_onscreen_hide_extra_joy_keys", _optionSetVKBHideExtraJoyKeys, 0 },
	{ "pref_input_onscreen_hide_shortcut_keys", _optionSetVKBHideShortcutKeys, 0 },
	//{ "pref_input_onscreen_hide_turbo_key", _optionSetVKBHideTurboKey, 0 },
	{ "pref_storage_savestate_folder", _optionSetSaveStateFolder, 0 },
	{ "pref_savestate_quicksaveslot", _optionSetQuickSaveSlot, 0 },
	{ "_pref_dyn_shortcut_map", _optionVKBSetShortcutKeys, 0 },
	{ "pref_input_onscreen_hide_joy", _optionVKBHideJoy, 0 },
	{ "pref_input_keyboard_sizeVX", _optionVKBKeySizeVX, 0 },
	{ "pref_input_keyboard_sizeVY", _optionVKBKeySizeVY, 0 },
	{ "pref_input_keyboard_sizeHX", _optionVKBKeySizeHX, 0 },
	{ "pref_input_keyboard_sizeHY", _optionVKBKeySizeHY, 0 },
	{ "pref_midi_out", _optionMidiOutEnabled, _optionValMidiOutEnabled },
	{ "pref_midi_st_reset", _optionMidiResetOnSTReset, _optionValMidiResetOnSTReset },
	//{ "pref_midi_device_android", _optionMidiDeviceAndroid, 0 },
	//{ "pref_midi_device_fluidsynth", _optionMidiDeviceFluidSynth, 0 },
	{ "pref_midi_fluidsynth_use_custom_sf", _optionMidiFluidSynthUseCustomSF, 0 },
	//{ "pref_midi_fluidsynth_soundfont", _optionMidiFluidSynthSoundFont, 0 },
	{ "pref_midi_fluidsynth_reverb", _optionMidiFluidSynthReverb, 0 },
	{ "pref_midi_fluidsynth_chorus", _optionMidiFluidSynthChorus, 0 },
	{ "pref_midi_fluidsynth_interp", _optionMidiFluidSynthInterp, 0 },
	{ "pref_midi_fluidsynth_maxpoly", _optionMidiFluidSynthMaxPoly, 0 },
	{ "pref_midi_fluidsynth_volgain", _optionMidiFluidSynthVolGain, 0 },
	{ "pref_midi_tweak_ignore_pgm_changes", _optionMidiIgnoreProgramChanges, _optionValIgnoreProgramChanges },
	{ "pref_midi_chan_transpose", _optionMidiTranspose, _optionValMidiTranspose },
    { "pref_midi_hardware_out", _optionMidiHardwareOutEnabled, _optionValMidiHardwareOutEnabled },
    { "pref_midi_hardware_in", _optionMidiHardwareInEnabled, _optionValMidiHardwareInEnabled },
};
static const int s_NumOptionMaps = sizeof(s_OptionsMap)/sizeof(OptionSetting);

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSetOptions(JNIEnv * env, jobject obj, jobjectArray keyarray, jobjectArray valarray)
{
	SetEmulatorOptions(env, keyarray, valarray, true, false, true);
	g_audioMute = 1;
}

static bool _firstInit = true;

void EmuCommandSetOptions_Run(EmuCommand *command)
{
	Debug_Printf("----> Set Options");

	EmuCommandSetOptions_Data *data = (EmuCommandSetOptions_Data*)command->data;

	//int changed = 0;
	//bool reset = false;

	bool bForceReset = _firstInit;
	bool bLoadedSnapshot = false;
	CNF_PARAMS current;

	if (!s_optionsIniting)//data->init)
	{
		Main_PauseEmulation(false);
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
		const int minSize = 25;
		ConfigureParams.Sound.SdlAudioBufferSize = minSize + (ConfigureParams.Hataroid.deviceSoundBufSize-18);
		if (ConfigureParams.Sound.SdlAudioBufferSize < minSize)
		{
			ConfigureParams.Sound.SdlAudioBufferSize = minSize;
		}

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
				&& (s_loadingCommand == 0)
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

	_firstInit = false;
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
	if (s_loadingCommand > 0)
	{
		Debug_Printf("----> Ignored as load command pending");
		return;
	}
	if (emuQuit || _saveAndQuit)
	{
		Debug_Printf("----> Ignored as quiting");
		return;
	}

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
	Main_PauseEmulation(false);
    _resetCold();
	Main_UnPauseEmulation();
}

void _resetCold()
{
    g_audioMute = 1;
    setTurboSpeed(0);
    setUserEmuPaused(0, EMUPAUSETYPE_MASK);
    Reset_Cold(false);
    fsMidi_reset(_resetSynthOnSTReset?1:0);
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetCold(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> Cold Reset Command");
	if (emuInited)
	{
		//Main_SetCPUBrk();
		addEmuCommand(EmuCommandResetCold_Run, NULL, NULL);
	}
}

void EmuCommandResetWarm_Run(EmuCommand *command)
{
	Debug_Printf("----> Warm Reset");
	Main_PauseEmulation(false);
	g_audioMute = 1;
	setTurboSpeed(0);
	setUserEmuPaused(0, EMUPAUSETYPE_MASK);
	Reset_Warm();
	fsMidi_reset(_resetSynthOnSTReset?1:0);
	Main_UnPauseEmulation();
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorResetWarm(JNIEnv * env, jobject obj)
{
	Debug_Printf("----> Warm Reset Command");
	if (emuInited) { addEmuCommand(EmuCommandResetWarm_Run, NULL, NULL); }
}

void EmuCommandEjectFloppy_Run(EmuCommand *command)
{
	Debug_Printf("----> Eject Floppy");
	Main_PauseEmulation(false);

	int floppyID = (int)command->data;

	Floppy_SetDiskFileNameNone(floppyID);
	ConfigureParams.DiskImage.szDiskZipPath[floppyID][0] = '\0';
	Floppy_EjectDiskFromDrive(floppyID);

	Debug_Printf("----> Ejected Floppy from drive (%c)", floppyID?'B':'A');

	Main_UnPauseEmulation();
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
	char *dispName;
};

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorIsPastiDiskResetRequired(JNIEnv * env, jobject obj, jstring filename, jstring zippath)
{
	Debug_Printf("----> Check if Pasti Disk Reset required");

    bool resetReq = false;

	if (emuInited)
	{
	    if ((FDC_Compat_GetCompatMode() == FDC_CompatMode_Old))
	    {
			const char *fnameptr = env->GetStringUTFChars(filename, NULL);
			const char *zippathptr = zippath ? env->GetStringUTFChars(zippath, NULL) : NULL;

	        resetReq = Floppy_IsPasti(fnameptr, zippathptr); // ok for args to be null

			env->ReleaseStringUTFChars(filename, fnameptr);
			if (zippath)
			{
				env->ReleaseStringUTFChars(zippath, zippathptr);
			}
		}
	}

	return resetReq;
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorIsPastiDisk(JNIEnv * env, jobject obj, jstring filename, jstring zippath)
{
	Debug_Printf("----> Check if Pasti Disk");

    bool isPasti = false;

	//if (emuInited) // allow running before emu is ready
	{
		const char *fnameptr = env->GetStringUTFChars(filename, NULL);
		const char *zippathptr = zippath ? env->GetStringUTFChars(zippath, NULL) : NULL;

        isPasti = Floppy_IsPasti(fnameptr, zippathptr); // ok for args to be null

		env->ReleaseStringUTFChars(filename, fnameptr);
		if (zippath)
		{
			env->ReleaseStringUTFChars(zippath, zippathptr);
		}
	}

	return isPasti;
}

void EmuCommandInsertFloppy_Run(EmuCommand *command)
{
	Debug_Printf("----> Insert Floppy");
	Main_PauseEmulation(false);

	EmuCommandInsertFloppy_Data *data = (EmuCommandInsertFloppy_Data*)command->data;

    // check if pasti disc inserted during legacy fdc emulation, then force reset
    if ((FDC_Compat_GetCompatMode() == FDC_CompatMode_Old))
    {
        if (Floppy_IsPasti(data->floppyFileName, data->floppyZipPath))
        {
	        ConfigureParams.Hataroid.legacyFloppy = false;
            FDC_Compat_SetCompatMode(FDC_CompatMode_Default);
            _resetCold();
        }
    }

	Floppy_SetDiskFileName(data->floppyID, data->floppyFileName, data->floppyZipPath);
	Floppy_InsertDiskIntoDrive(data->floppyID);

	_saveDispName[0] = 0;
	if (data->dispName != 0)
	{
		strcpy(_saveDispName, data->dispName);
	}

	Debug_Printf("----> Inserted Floppy: (%s)(%s) into drive (%c)", data->floppyFileName, data->floppyZipPath ? data->floppyZipPath : "direct", data->floppyID?'B':'A');

	Main_UnPauseEmulation();
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
	static const int OpSaveQuickSave	= 4;
	static const int OpLoadQuickSave	= 5;

	int saveStateOp;
	int saveSlot;
	char *saveStatePath;
	char *saveStateFilePath;
};

void EmuCommandSaveState_Run(EmuCommand *command)
{
	Debug_Printf("----> Save State Operation");

	EmuCommandSaveState_Data *data = (EmuCommandSaveState_Data*)command->data;

	Main_PauseEmulation(false);

	switch (data->saveStateOp)
	{
		case EmuCommandSaveState_Data::OpSave:
		{
			_saving = true;
			Debug_Printf("Deleting: %s\n", data->saveStateFilePath);
			remove(data->saveStateFilePath); // TODO: check error codes

			char* saveMetaFilePath = new char [FILENAME_MAX];

			_generateSaveNames(data->saveStatePath, data->saveSlot, saveMetaFilePath);
			_storeSaveState(saveMetaFilePath);

			delete [] saveMetaFilePath;
			_saving = false;
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

			_loadSaveState();
			break;
		}
		case EmuCommandSaveState_Data::OpSaveAutoSave:
		{
			_saving = true;
			Debug_Printf("----> Saving auto save...");

			char* saveMetaFilePath = new char [FILENAME_MAX];
			snprintf(saveMetaFilePath, FILENAME_MAX, "%s/as.qs", data->saveStatePath);
			saveMetaFilePath[FILENAME_MAX-1] = 0;

			snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/as.sav", data->saveStatePath);
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1] = 0;

			_storeSaveState(saveMetaFilePath);

			delete [] saveMetaFilePath;

			Debug_Printf("----> Auto Save saved...");

			_pendingQuit = true;

			Main_SetCPUBrk();

			//RequestAndWaitQuit();
			_saving = false;
			break;
		}
		case EmuCommandSaveState_Data::OpLoadAutoSave:
		{
			Debug_Printf("----> Loading auto save...");

			snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/as.sav", data->saveStatePath);
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;

			_loadSaveState();
			break;
		}
		case EmuCommandSaveState_Data::OpSaveQuickSave:
		{
			_saving = true;
			Debug_Printf("----> Saving quick save...");

			if (_quickSaveSlot == -1 || _quickSavePath[0] == 0)
			{
				break;
			}
			char *metaBaseName = new char [FILENAME_MAX];
			char *saveMetaFilePath = new char [FILENAME_MAX];

			if (_findSaveStateMetaFile(_quickSaveSlot, metaBaseName))
			{
				snprintf(saveMetaFilePath, FILENAME_MAX, "%s/%s", _quickSavePath, metaBaseName);
				saveMetaFilePath[FILENAME_MAX-1] = 0;

				Debug_Printf("Deleting: %s\n", saveMetaFilePath);
				remove(saveMetaFilePath); // TODO: check error codes
			}

			_generateSaveNames(_quickSavePath, _quickSaveSlot, saveMetaFilePath);
			_storeSaveState(saveMetaFilePath);

			delete [] saveMetaFilePath;
			delete [] metaBaseName;
			_saving = false;
			break;
		}
		case EmuCommandSaveState_Data::OpLoadQuickSave:
		{
			if (_quickSaveSlot == -1 || _quickSavePath[0] == 0)
			{
				break;
			}
			Debug_Printf("----> Loading quick save...");

			snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/%03d.sav", _quickSavePath, _quickSaveSlot);
			ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;

			_loadSaveState();
			break;
		}
	}

	// Reset the sound emulation variables:
	Sound_BufferIndexNeedReset = true;

	Main_UnPauseEmulation();
}

void _generateSaveNames(const char *savePath, int saveSlot, char* saveMetaFilePathResult)
{
	// create new save name based on inserted floppy
	char* saveMetaBaseName = new char [FILENAME_MAX];

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

	if (saveSlot >= 0)
	{
		snprintf(saveMetaBaseName, FILENAME_MAX, "%03d_%s", saveSlot, sFloppyName);
		snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/%03d.sav", savePath, saveSlot);
	}
	else
	{
		snprintf(saveMetaBaseName, FILENAME_MAX, "xxx_%s", sFloppyName);
		snprintf(ConfigureParams.Memory.szMemoryCaptureFileName, FILENAME_MAX, "%s/xxx.sav", savePath);
	}
	ConfigureParams.Memory.szMemoryCaptureFileName[FILENAME_MAX-1]=0;
	saveMetaBaseName[FILENAME_MAX-1]=0;

	snprintf(saveMetaFilePathResult, FILENAME_MAX, "%s/%s.ss", savePath, saveMetaBaseName);
	saveMetaFilePathResult[FILENAME_MAX-1]=0;

	Debug_Printf("Floppy: %s, SaveName: %s, MetaName: %s", sFloppyName, ConfigureParams.Memory.szMemoryCaptureFileName, saveMetaFilePathResult);

	if (tempFloppyName != 0)
	{
		delete [] tempFloppyName;
	}
	delete [] saveMetaBaseName;
}

// TODO: to avoid this, don't bother encoding name in file, just put it in meta data instead (but requires loading meta when populating list)
bool _findSaveStateMetaFile(int slotID, char *resultBuf)
{
	const int kMaxPrefix = 12;
	char prefix[kMaxPrefix];
	snprintf(prefix, kMaxPrefix, "%03d_", slotID);
	prefix[kMaxPrefix-1] = 0;
	int prefixLen = strlen(prefix);

	DIR *dirp = opendir(_quickSavePath);

	bool found = false;
	while (dirp)
	{
	    struct dirent *entry;
	    if ((entry = readdir(dirp)) == 0)
	    {
	    	break;
	    }

		// check prefix
		if (strncmp(prefix, entry->d_name, prefixLen) == 0)
		{
		    // check ext
			const char *ext = strrchr(entry->d_name, '.');
			if (ext != 0)
			{
			    if (strcasecmp(ext, ".ss") == 0)
				{
				    strncpy(resultBuf, entry->d_name, FILENAME_MAX);
					resultBuf[FILENAME_MAX-1]=0;
					found = true;
					break;
				}
			}
		}
	}

	if (dirp)
	{
		closedir(dirp);
	}

	return found;
}

void _hataroidRetrieveSaveExtraData()
{
	ConfigureParams.Hataroid.fullScreen = Renderer_isFullScreenStretch();
	ConfigureParams.Hataroid.scrZoomX = Renderer_getScreenZoomXRaw();
	ConfigureParams.Hataroid.scrZoomY = Renderer_getScreenZoomYRaw();
	ConfigureParams.Hataroid.scrPanX = Renderer_getScreenPanXRaw();
	ConfigureParams.Hataroid.scrPanY = Renderer_getScreenPanYRaw();

	ConfigureParams.Hataroid.kbdZoom = VirtKB_getVKBZoom();
	ConfigureParams.Hataroid.kbdPanX = VirtKB_getVKBPanX();
	ConfigureParams.Hataroid.kbdPanY = VirtKB_getVKBPanY();

	ConfigureParams.Hataroid.mouseActive = VirtKB_getMouseActive();

	strcpy(ConfigureParams.Hataroid.saveDispName, _saveDispName);

	//if (gSaveVersion >= 1704)
	{
		memcpy(&ConfigureParams.HataroidMidi.midiState, fsMidi_getSaveData(), sizeof(ConfigureParams.HataroidMidi.midiState));
		ConfigureParams.HataroidMidi.midiOutEnabled = fsMidi_isMidiOutEnabled();
		ConfigureParams.HataroidMidi.midiDevice = fsMidi_getMidiOutDevice();
		memcpy(&ConfigureParams.HataroidMidi.fsSoundFont, fsMidi_getFSynthSoundFont(), sizeof(ConfigureParams.HataroidMidi.fsSoundFont));
		ConfigureParams.HataroidMidi.fsReverb = fsMidi_getFSynthReverb();
		ConfigureParams.HataroidMidi.fsChorus = fsMidi_getFSynthChorus();
		ConfigureParams.HataroidMidi.fsInterp = fsMidi_getFSynthInterp();
		ConfigureParams.HataroidMidi.fsMaxPoly = fsMidi_getFSynthMaxPoly();
		ConfigureParams.HataroidMidi.fsVolGain = fsMidi_getFSynthVolGain();
	}

	//if (gSaveVersion >= 1705)
	{
		//MemorySnapShot_Store(&ConfigureParams.Hataroid.muteSTSound, sizeof(ConfigureParams.Hataroid.muteSTSound)); // auto saved
	}

	//if (gSaveVersion >= 1706)
	{
		ConfigureParams.HataroidMidi.midiResetOnSTReset = _resetSynthOnSTReset?true:false;
		ConfigureParams.HataroidMidi.midiIgnoreProgramChanges = fsMidi_getIgnoreProgramChanges()?true:false;
		ConfigureParams.HataroidMidi.midiChanTranspose = fsMidi_getMidiTranspose();
	}

    //if (gSaveVersion >= 1707)
    {
        ConfigureParams.HataroidMidi.midiHardwareOutEnabled = fsMidi_isMidiHardwareOutEnabled();
        ConfigureParams.HataroidMidi.midiHardwareInEnabled = fsMidi_isMidiHardwareInEnabled();
    }
}

void _loadSaveState()
{
	Debug_Printf("Loading save state...");
	int restoreResult = MemorySnapShot_Restore(ConfigureParams.Memory.szMemoryCaptureFileName, false);
	if (restoreResult == 0)
	{
		VirtKB_ResetAllInputPresses();

		int numSaveExtraOptions = 0;
		bool hataroidExtraFullScreen = false;

		_saveDispName[0] = 0;
		if (gHasHataroidSaveExtra != 0)
		{
			// restore hataroid settings
			hataroidExtraFullScreen = ConfigureParams.Hataroid.fullScreen;
			++numSaveExtraOptions;
			_optionSetFullScreenStretch(0, hataroidExtraFullScreen ? "true" : "false", 0);

			if (!hataroidExtraFullScreen)
			{
				float scrZoomX = ConfigureParams.Hataroid.scrZoomX, scrZoomY = ConfigureParams.Hataroid.scrZoomY;
				float scrPanX = ConfigureParams.Hataroid.scrPanX, scrPanY = ConfigureParams.Hataroid.scrPanY;
				Renderer_setScreenPanZoomRaw(scrZoomX, scrZoomY, scrPanX, scrPanY);
			}

			float kbdZoom = ConfigureParams.Hataroid.kbdZoom;
			float kbdPanX = ConfigureParams.Hataroid.kbdPanX, kbdPanY = ConfigureParams.Hataroid.kbdPanY;
			VirtKB_SetVKBPanZoom(kbdZoom, kbdPanX, kbdPanY);

			bool mouseActive = ConfigureParams.Hataroid.mouseActive;
			VirtKB_SetMouseActive(mouseActive);

			strcpy(_saveDispName, ConfigureParams.Hataroid.saveDispName);

			fsMidi_reset(1);

			if (gSaveVersion >= 1704)
			{
				fsMidi_setSaveData(&ConfigureParams.HataroidMidi.midiState);
				fsMidi_setMidiOutEnabled(ConfigureParams.HataroidMidi.midiOutEnabled?1:0);
				//ConfigureParams.HataroidMidi.midiDevice = fsMidi_getMidiOutDevice();
				//memcpy(&ConfigureParams.HataroidMidi.fsSoundFont, fsMidi_getFSynthSoundFont(), sizeof(ConfigureParams.HataroidMidi.fsSoundFont));
				//ConfigureParams.HataroidMidi.fsReverb = fsMidi_getFSynthReverb();
				//ConfigureParams.HataroidMidi.fsChorus = fsMidi_getFSynthChorus();
				//ConfigureParams.HataroidMidi.fsInterp = fsMidi_getFSynthInterp();
				//ConfigureParams.HataroidMidi.fsMaxPoly = fsMidi_getFSynthMaxPoly();
				//ConfigureParams.HataroidMidi.fsVolGain = fsMidi_getFSynthVolGain();
			}

			//if (gSaveVersion >= 1705)
			{
				//MemorySnapShot_Store(&ConfigureParams.Hataroid.muteSTSound, sizeof(ConfigureParams.Hataroid.muteSTSound)); // auto restored
			}

			//if (gSaveVersion >= 1706)
			{
				_resetSynthOnSTReset = ConfigureParams.HataroidMidi.midiResetOnSTReset ? 1 : 0;
				fsMidi_setIgnoreProgramChanges(ConfigureParams.HataroidMidi.midiIgnoreProgramChanges ? 1 : 0);
				fsMidi_setMidiTranspose(ConfigureParams.HataroidMidi.midiChanTranspose);
			}

            //if (gSaveVersion >= 1707)
            {
                fsMidi_setMidiHardwareOutEnabled(ConfigureParams.HataroidMidi.midiHardwareOutEnabled?1:0);
                fsMidi_setMidiHardwareInEnabled(ConfigureParams.HataroidMidi.midiHardwareInEnabled?1:0);
            }
		}

		// back propagate settings back to java activity settings
		JNIEnv *env = g_jniMainInterface.android_mainEmuThreadEnv;

		int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
		if (status == JNI_OK)
		{
			int numOptions = 0;
			for (int i = 0; i < s_NumOptionMaps; ++i)
			{
				if (s_OptionsMap[i].valCallback != 0)
				{
					++numOptions;
				}
			}

			numOptions += numSaveExtraOptions;

			jobjectArray options = (jobjectArray)env->NewObjectArray(numOptions*2, env->FindClass("java/lang/String"), env->NewStringUTF(""));

			char tmpBuf[FILENAME_MAX];
			int jIdx = 0;
			for (int i = 0; i < s_NumOptionMaps; ++i)
			{
				if (s_OptionsMap[i].valCallback != 0)
				{
					const char *keyName = s_OptionsMap[i].key;
					jstring jKey = env->NewStringUTF(keyName);
					env->SetObjectArrayElement(options, jIdx++, jKey);
					env->DeleteLocalRef(jKey);

					jstring jVal = 0;
					if ((*(s_OptionsMap[i].valCallback))(&s_OptionsMap[i], tmpBuf, FILENAME_MAX))
					{
						tmpBuf[FILENAME_MAX-1] = 0;
						jVal = env->NewStringUTF(tmpBuf);
					}
					else
					{
						jVal = env->NewStringUTF("_dk_");
					}
					env->SetObjectArrayElement(options, jIdx++, jVal);
					env->DeleteLocalRef(jVal);
				}
			}

			// extra options
			if (numSaveExtraOptions > 0)
			{
				jstring jKey = env->NewStringUTF("pref_display_fullscreen");
				env->SetObjectArrayElement(options, jIdx++, jKey);
				env->DeleteLocalRef(jKey);

				jKey = env->NewStringUTF(hataroidExtraFullScreen ? "true" : "false");
				env->SetObjectArrayElement(options, jIdx++, jKey);
				env->DeleteLocalRef(jKey);
			}

			(env)->CallVoidMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.setConfigOnSaveStateLoad, options);

			env->DeleteLocalRef(options);
		}
	}

	--s_loadingCommand;
	if (s_loadingCommand < 0)
	{
		s_loadingCommand = 0;
	}

	//Main_SetCPUBrk();

	Debug_Printf("Loading done.");
}

void _storeSaveState(const char *saveMetaFilePath)
{
	Debug_Printf("Storing save state...");

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
			const int kVersion = 2;
			const int kThumbWidth = 80;
			const int kThumbHeight = 60;

			const int kHeaderSize = 4 * 4; // version, width, height, dispNameLen

			fwrite("hsst", 4, 1, metaFile);
			fwrite(&kHeaderSize, 4, 1, metaFile);

			int dispNameLen = strlen(_saveDispName);
			if (dispNameLen > 0) { ++dispNameLen; }

			fwrite(&kVersion, 4, 1, metaFile);
			fwrite(&kThumbWidth, 4, 1, metaFile);
			fwrite(&kThumbHeight, 4, 1, metaFile);
			fwrite(&dispNameLen, 4, 1, metaFile);

			// disp name
			if (dispNameLen > 0)
			{
				fwrite(_saveDispName, 1, dispNameLen, metaFile);
			}

			// generate thumbnail
			{
				unsigned short *emuScrPixels = (unsigned short*)g_videoTex_pixels;
				if (emuScrPixels)
				{
					int texW = g_videoTex_width;
					int emuScrW = g_surface_width;
					int emuScrH = g_surface_height;

					float skipX = emuScrW / (float)kThumbWidth;
					float skipY = emuScrH / (float)kThumbHeight;
					int iSkipX = (int)skipX;
					int iSkipY = (int)skipY;

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

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorInsertFloppy(JNIEnv * env, jobject obj, jint floppy, jstring filename, jstring zippath, jstring dispName)
{
	Debug_Printf("----> Insert Floppy Command");

	//if (emuInited) // allow queueing insert command before emulation is ready
	{
		EmuCommandInsertFloppy_Data *data = new EmuCommandInsertFloppy_Data;
		memset(data, 0, sizeof(EmuCommandInsertFloppy_Data));

		data->floppyID = floppy;

		const char *fnameptr = env->GetStringUTFChars(filename, NULL);
		const char *zippathptr = zippath ? env->GetStringUTFChars(zippath, NULL) : NULL;
		const char *dispNamePtr = env->GetStringUTFChars(dispName, NULL);

		data->floppyFileName = new char [strlen(fnameptr)+1];
		strcpy(data->floppyFileName, fnameptr);

		data->dispName = new char [strlen(dispNamePtr)+1];
		strcpy(data->dispName, dispNamePtr);

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
		env->ReleaseStringUTFChars(dispName, dispNamePtr);
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

		if (op == EmuCommandSaveState_Data::OpSave
		 || op == EmuCommandSaveState_Data::OpSaveAutoSave
		 || op == EmuCommandSaveState_Data::OpSaveQuickSave)
		{
			_addSaveCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);
		}
		else
		{
			addEmuCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);
		}
	}
}

void _addSaveCommand(EmuCommandRunCallback runCallback, EmuCommandCleanupCallback cleanupCallback, void *data)
{
	while (s_saveCommandLock) { usleep(5000); }
	s_saveCommandLock = 1;

	if (s_saveCommand == 0)
	{
		EmuCommand *newCommand = createEmuCommand(runCallback, cleanupCallback, data);
		s_saveCommand = newCommand;
	}

	s_saveCommandLock = 0;
}

void _processHataroidSaveCommands()
{
	while (s_saveCommandLock) { usleep(5000); }
	s_saveCommandLock = 1;

	EmuCommand *curCommand = s_saveCommand;
	if (s_saveCommand != 0)
	{
		//s_headCommand = s_headCommand->next;
		s_saveCommand = 0;
	}

	if (curCommand != 0)
	{
		(curCommand->runCallback)(curCommand);
		if (curCommand->cleanupCallback)
		{
			(curCommand->cleanupCallback)(curCommand);
		}
		delete curCommand;
	}

	s_saveCommandLock = 0;
}

void _clearHataroidSaveCommands()
{
	while (s_saveCommandLock) { usleep(5000); }
	s_saveCommandLock = 1;

	EmuCommand *curCommand = s_saveCommand;
	if (s_saveCommand != 0)
	{
		//s_headCommand = s_headCommand->next;
		s_saveCommand = 0;
	}

	if (curCommand != 0)
	{
		//(curCommand->runCallback)(curCommand);
		if (curCommand->cleanupCallback)
		{
			(curCommand->cleanupCallback)(curCommand);
		}
		delete curCommand;
	}

	s_saveCommandLock = 0;
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateSave(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot)
{
	Debug_Printf("----> Save State Save Command");
	if (emuInited && !_saveAndQuit && _doubleBusError==0)
	{
		_createNewSaveStateCommand(env, obj, path, filepath, saveSlot, EmuCommandSaveState_Data::OpSave);
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorSaveStateLoad(JNIEnv * env, jobject obj, jstring path, jstring filepath, jint saveSlot)
{
	Debug_Printf("----> Save State Load Command");
	if (emuInited && !_saveAndQuit && _doubleBusError==0)
	{
		_createNewSaveStateCommand(env, obj, path, filepath, saveSlot, EmuCommandSaveState_Data::OpLoad);
		g_audioMute = 1;
		++s_loadingCommand;

		setUserEmuPaused(0, EMUPAUSETYPE_MASK);

		//Main_SetCPUBrk();
	}
}

JNIEXPORT jboolean JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveStoreOnExit(JNIEnv * env, jobject obj, jstring saveFolder)
{
	if (emuInited && !_saveAndQuit && _doubleBusError==0)
	{
		_clearHataroidSaveCommands();
		_createNewSaveStateCommand(env, obj, saveFolder, 0, -1, EmuCommandSaveState_Data::OpSaveAutoSave);
		_saveAndQuit = true;
		_altUpdate = false;
		_runTillQuit = 1;
		Main_UnPauseEmulation();
		setUserEmuPaused(0, EMUPAUSETYPE_MASK);
		_checkEmuReady();
		return true;
	}
	//else
	//{
	//	RequestAndWaitQuit();
	//}
	return false;
}

void hataroid_setDoubleBusError()
{
	_doubleBusError = 1;
	if (_saveAndQuit)
	{
		Debug_Printf("----> Double Bus Error, Clearing Emu Commands");
		_saveAndQuit = false;
		clearEmuCommands();
		_clearHataroidSaveCommands();

		Debug_Printf("----> DBError, Request and wait quit");
		Main_SetCPUBrk();
		RequestAndWaitQuit();
	}
}

JNIEXPORT void JNICALL Java_com_RetroSoft_Hataroid_HataroidNativeLib_emulatorAutoSaveLoadOnStart(JNIEnv * env, jobject obj, jstring saveFolder)
{
	Debug_Printf("----> Save State AutoLoadOnStart Command");
	if (emuInited)
	{
		_createNewSaveStateCommand(env, obj, saveFolder, 0, -1, EmuCommandSaveState_Data::OpLoadAutoSave);
		++s_loadingCommand;
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
	if (_saveAndQuit)
	{
		return;
	}

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

void quickSaveStore(JNIEnv *curEnv)
{
	if (_doubleBusError || _saveAndQuit)
	{
		return;
	}

	if (_quickSaveSlot == -1 || _quickSavePath[0] == 0)
	{
		showGenericDialog(curEnv, "Please select a quick save slot first before using Quick Saves", 1, 0, "", "Ok");
		return;
	}

	if (emuInited)
	{
		EmuCommandSaveState_Data *data = new EmuCommandSaveState_Data;
		memset(data, 0, sizeof(EmuCommandSaveState_Data));

		data->saveStateOp = EmuCommandSaveState_Data::OpSaveQuickSave;

		_addSaveCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);
		//addEmuCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);
	}
}

void quickSaveLoad(JNIEnv *curEnv)
{
	if (_quickSaveSlot == -1 || _quickSavePath[0] == 0)
	{
		showGenericDialog(curEnv, "Please select a quick save slot first before using Quick Saves", 1, 0, "", "Ok");
		return;
	}

	if (emuInited)
	{
		EmuCommandSaveState_Data *data = new EmuCommandSaveState_Data;
		memset(data, 0, sizeof(EmuCommandSaveState_Data));

		data->saveStateOp = EmuCommandSaveState_Data::OpLoadQuickSave;

		addEmuCommand(EmuCommandSaveState_Run, EmuCommandSaveState_Cleanup, data);

		++s_loadingCommand;
	}
}

extern "C" void _checkHataroidSaveRequests()
{
	_processHataroidSaveCommands();
}

extern "C" void hataroidRetrieveSaveExtraData()
{
	_hataroidRetrieveSaveExtraData();
}

#define ASSETDATA_MAX_ITEMS		4
typedef struct
{
	int dataID;
	const char* data;
	char assetPath[FILENAME_MAX];
	int dataLen;
	int persist;
	int refCount;
} AssetData;

static AssetData* _assetDataItems = 0;
static int _assetDataNextID = 1;
static int _assetDataNumItems = 0;

void _initAssetDataItems()
{
	_deinitAssetDataItems();

	_assetDataItems = new AssetData [ASSETDATA_MAX_ITEMS];
	memset(_assetDataItems, 0, sizeof(AssetData) * ASSETDATA_MAX_ITEMS);

	_assetDataNumItems = 0;
}

void _deinitAssetDataItems()
{
	if (_assetDataItems)
	{
		for (int i = 0; i < ASSETDATA_MAX_ITEMS; ++i)
		{
			if (_assetDataItems[i].data)
			{
				delete[] _assetDataItems[i].data;
				_assetDataItems[i].data = 0;
			}
		}
		delete [] _assetDataItems;
		_assetDataItems = 0;

		_assetDataNumItems = 0;
	}
}

void _deleteCachedAssetDataRef(AssetData *d)
{
	if (d != 0 && d->refCount <= 0)
	{
		Debug_Printf("Releasing asset: %s", d->assetPath);

		d->dataID = 0;
		delete [] d->data;
		d->data = 0;
		d->assetPath[0] = 0;
		d->dataLen = 0;
		d->persist = 0;
		d->refCount = 0;

		--_assetDataNumItems;
	}
}

extern "C" void hataroid_releaseAssetDataRef(int assetID)
{
	if (_assetDataItems != 0)
	{
		for (int i = 0; i < ASSETDATA_MAX_ITEMS; ++i)
		{
			AssetData *d = &_assetDataItems[i];
			if (d->dataID == assetID)
			{
				--d->refCount;

				if (!d->persist && d->refCount <= 0)
				{
					_deleteCachedAssetDataRef(d);
				}
				return;
			}
		}
	}
}

extern "C" const char* hataroid_getAssetDataDirect(JNIEnv *curEnv, const char* assetPath, int nullTerm, int *len)
{
	Debug_Printf("hataroid_getAssetDataDirect: %s", assetPath);

	if (!envInited)
	{
		Debug_Printf("env not inited yet: %s", assetPath);
		return 0;
	}

	(*len) = 0;

	JNIEnv* env = (curEnv==0) ? g_jniMainInterface.android_mainEmuThreadEnv : curEnv;
	jstring str = (env)->NewStringUTF(assetPath);
	jbyteArray assetData = (jbyteArray)((env)->CallObjectMethod(g_jniMainInterface.android_mainActivity, g_jniMainInterface.getAssetData, str));

	char* buf = 0;

	if (assetData != 0)
	{
		int dataLen = (env)->GetArrayLength(assetData);
		if (dataLen > 0)
		{
			jbyte* dataBytes = (env)->GetByteArrayElements(assetData, 0);

			buf = new char [dataLen + (nullTerm?1:0)];
			memcpy(buf, dataBytes, dataLen);
			if (nullTerm)
			{
				buf[dataLen] = 0;
			}

			(env)->ReleaseByteArrayElements(assetData, dataBytes, JNI_ABORT);

			(*len) = dataLen;

			Debug_Printf("retrieved asset (direct): %s, %d", assetPath, dataLen);
		}

		//(env)->DeleteLocalRef(assetData); // explicitly releasing to assist garbage collection, though not required
	}

	return buf;
}

extern "C" const char* hataroid_getAssetDataRef(JNIEnv *curEnv, const char* assetPath, int persist, int *len, int *id)
{
	Debug_Printf("hataroid_getAssetDataRef: %s", assetPath);

	(*len) = 0;
	(*id) = 0;

	if (_assetDataItems == 0)
	{
		return 0; // not initialized yet
	}

	// return cached asset
	AssetData *freeSlot = 0;
	AssetData *unrefSlot = 0;
	for (int i = 0; i < ASSETDATA_MAX_ITEMS; ++i)
	{
		AssetData *d = &_assetDataItems[i];
		if (d->dataID == 0)
		{
			if (freeSlot == 0) { freeSlot = d; }
		}
		else if (strcmp(d->assetPath, assetPath) == 0)
		{
			Debug_Printf("retrieved asset (cached): %s, %d", assetPath, d->dataLen);

			++d->refCount;
			(*len) = d->dataLen;
			(*id) = d->dataID;
			return d->data;
		}
		else if (d->refCount == 0)
		{
			if (unrefSlot == 0) { unrefSlot = d; }
		}
	}

	if (freeSlot == 0 && unrefSlot != 0)
	{
		_deleteCachedAssetDataRef(unrefSlot);
		freeSlot = unrefSlot;
		unrefSlot = 0;
	}

	if (freeSlot == 0 || _assetDataNumItems >= ASSETDATA_MAX_ITEMS)
	{
		return 0; // no more slots
	}

	const char* buf = hataroid_getAssetDataDirect(curEnv, assetPath, 0, len);
	if (buf != 0)
	{
		freeSlot->dataID = _assetDataNextID++;
		freeSlot->data = buf;
		freeSlot->dataLen = (*len);
		freeSlot->persist = persist;
		freeSlot->refCount = 1;
		strcpy(freeSlot->assetPath, assetPath);
		++_assetDataNumItems;

		(*id) = freeSlot->dataID;
	}

	return buf;
}

void _confirmSettings(JNIEnv* curEnv)
{
}

static void _preloadEmuTOS(JNIEnv* curEnv)
{
    _emuTOSRomST = hataroid_getAssetDataDirect(curEnv, "tos/etos192uk.img", 0, &_emuTOSRomSTLen);
    _emuTOSRomSTE = hataroid_getAssetDataDirect(curEnv, "tos/etos256uk.img", 0, &_emuTOSRomSTELen);
}

extern "C" char* hataroid_LoadEmuTOS(int machineType, long* fileSize)
{
    (*fileSize) = 0;

    if (machineType == MACHINE_STE)
    {
        if (_emuTOSRomSTE != 0 && _emuTOSRomSTELen > 0)
        {
            (*fileSize) = _emuTOSRomSTELen;

            char* buf = (char*)malloc(_emuTOSRomSTELen);
            memcpy(buf, _emuTOSRomSTE, _emuTOSRomSTELen);
            return buf;
        }
        return 0;
    }

    // default to MACHINE_ST
    if (_emuTOSRomST != 0 && _emuTOSRomSTLen > 0)
    {
        (*fileSize) = _emuTOSRomSTLen;

        char* buf = (char*)malloc(_emuTOSRomSTLen);
        memcpy(buf, _emuTOSRomST, _emuTOSRomSTLen);
        return buf;
    }
    return 0;
}
