#ifndef __HATAROID_H__
#define __HATAROID_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void Debug_Printf(const char *a_pszformat, ...);
extern void usleep(int usecs);
extern int hasDialogResult();
extern int getDialogResult();
extern void clearDialogResult();

extern int getTurboSpeed();
extern void setTurboSpeed(int set);
extern int getShortcutAutoFire();
extern void setShortcutAutoFire(int enable, int set);

extern void quickSaveLoad();
extern void quickSaveStore();

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
};

struct JNIMainMethodCache
{
	JNIEnv *android_env;
	jobject android_mainActivity;
	int mainActivityGlobalRefObtained;

	JNIEnv *android_mainEmuThreadEnv;
	jmethodID showGenericDialog;
	jmethodID showOptionsDialog;
	jmethodID quitHataroid;
	jmethodID setConfigOnSaveStateLoad;
};

extern struct JNIAudio g_jniAudioInterface;

extern void showGenericDialog(const char *message, int ok, int noyes);
extern void showOptionsDialog();
extern int hasEmuCommands();
extern void processEmuCommands();
extern void clearEmuCommands();
extern void RequestAndWaitQuit();
extern void setUserEmuPaused(int pause);


#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

#endif //__HATAROID_H__
