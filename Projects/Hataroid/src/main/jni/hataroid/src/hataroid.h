#ifndef __HATAROID_H__
#define __HATAROID_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void Debug_Printf(const char *a_pszformat, ...);
extern void Debug_VPrintf(const char *a_pszformat, va_list args);

//extern void usleep(int usecs);
extern int hasDialogResult();
extern int getDialogResult();
extern void clearDialogResult();

extern void clearSettingsResult();
extern int hasSettingsResult();

extern void hataroid_setDoubleBusError();

extern int getTurboSpeed();
extern void setTurboSpeed(int set);
extern int getShortcutAutoFire();
extern void setShortcutAutoFire(int enable, int set);

extern void quickSaveLoad(JNIEnv *curEnv);
extern void quickSaveStore(JNIEnv *curEnv);

extern void showSoftMenu(JNIEnv *curEnv, int optionType);

extern JavaVM *g_jvm;

struct JNIAudio
{
	JNIEnv *android_env;
	jobject android_mainActivity;

	jmethodID getMinBufSize;
	jmethodID initAudio;
	jmethodID deinitAudio;
	jmethodID sendAudio;
	jmethodID pauseAudio;
	jmethodID playAudio;

	jmethodID showAudioErrorDialog;
};

struct JNIMainMethodCache
{
	JNIEnv *android_env;
	jobject android_mainActivity;
	volatile int mainActivityGlobalRefObtained;

	JNIEnv *android_mainEmuThreadEnv;
	jmethodID showGenericDialog;
	jmethodID destroyGenericDialog;
	jmethodID updateDialogMessage;
	jmethodID showOptionsDialog;
	jmethodID showFloppyAInsert;
	jmethodID showFloppyBInsert;
	jmethodID showSoftMenu;
	jmethodID quitHataroid;
	jmethodID setConfigOnSaveStateLoad;
	jmethodID getAssetData;

	jmethodID dbgGetAudioBufQueuedPercent;
	jmethodID getFreeAudioBuffer;

	//jmethodID sendAndMidiAudio;
	jmethodID sendMidiByte;
};

extern struct JNIAudio	g_jniAudioInterface;

extern int showGenericDialog(JNIEnv *curEnv, const char *message, int ok, int noyes, const char* noTxt, const char* yesOKTxt);
extern void destroyGenericDialog(JNIEnv *curEnv, int dialogID);
extern void updateGenericDialogMessage(JNIEnv *curEnv, int dialogID, const char *message);
extern void showOptionsDialog(JNIEnv *curEnv);
extern void showFloppyAInsert(JNIEnv *curEnv);
extern void showFloppyBInsert(JNIEnv *curEnv);
extern int hasEmuCommands();
extern void processEmuCommands();
extern void clearEmuCommands();
extern void RequestAndWaitQuit();
extern void setUserEmuPaused(int pause, int pauseFlag);
extern void toggleUserEmuPaused();
extern int isUserEmuPaused();

extern const char* hataroid_getAssetDataDirect(JNIEnv *curEnv, const char* assetPath, int nullTerm, int *len);
extern void hataroid_freeAssetDataDirect(char* data);

extern const char* hataroid_getAssetDataRef(JNIEnv *curEnv, const char* assetPath, int persist, int *len, int *id);
extern void hataroid_releaseAssetDataRef(int assetID);

extern char* hataroid_LoadEmuTOS(int machineType, long* fileSize);

extern float dbgGetAudioBufQueuedPercent(JNIEnv *curEnv);
extern int getFreeAudioBuffer(JNIEnv *curEnv);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

#endif //__HATAROID_H__
