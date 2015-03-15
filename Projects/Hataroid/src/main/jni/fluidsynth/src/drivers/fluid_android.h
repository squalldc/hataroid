#ifndef _FLUID_ANDROID_H
#define _FLUID_ANDROID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fluidsynth.h"

FLUIDSYNTH_API int fluid_android_process_frames(fluid_audio_driver_t* p, int nframes);
FLUIDSYNTH_API short* fluid_android_consume_buffer(fluid_audio_driver_t* d, int reqLen, int *startOffset, int *readLen, int *bufMax);

FLUIDSYNTH_API void* fluid_android_create_midi_parser();
FLUIDSYNTH_API void fluid_android_destroy_midi_parser(void *parser);
FLUIDSYNTH_API fluid_midi_event_t* fluid_android_parse_midi_byte(void *parser, unsigned char b);

#ifdef __cplusplus
}
#endif

#endif // _FLUID_ANDROID_H
