#ifndef FLOPPYSND_H_
#define FLOPPYSND_H_

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


extern int  FloppySnd_Init(JNIEnv* env);
extern void FloppySnd_DeInit();

extern int FloppySnd_IsEnabled();
extern void FloppySnd_Enable(int enable);
extern int  FloppySnd_Setup();
extern void FloppySnd_SetOutVol(float outVol);
extern void FloppySnd_SetOutFreq(int outFreq);
extern void FloppySnd_Reset();

extern void FloppySnd_GenSamples(int nMixBufIdx, int samplesToGen);
extern void FloppySnd_UpdateEmuFrame();

#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

#endif /* FLOPPYSND_H_ */
