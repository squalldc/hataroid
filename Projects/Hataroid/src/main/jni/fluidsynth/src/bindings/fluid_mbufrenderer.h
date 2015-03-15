#ifndef _FLUID_MBUFRENDERER_H
#define _FLUID_MBUFRENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fluidsynth.h"

typedef struct _fluid_mbuf_renderer_t fluid_mbuf_renderer_t;    /**< Audio file renderer instance */

FLUIDSYNTH_API fluid_mbuf_renderer_t *new_fluid_mbuf_renderer(fluid_synth_t* synth);
FLUIDSYNTH_API void delete_fluid_mbuf_renderer(fluid_mbuf_renderer_t* dev);
FLUIDSYNTH_API int fluid_mbuf_renderer_process_block(fluid_mbuf_renderer_t* dev);
FLUIDSYNTH_API int fluid_mbuf_renderer_process_block_frames(fluid_mbuf_renderer_t* dev, int nframes);
FLUIDSYNTH_API short* fluid_mbuf_get_buffer(fluid_mbuf_renderer_t* dev, int *bufLen);
FLUIDSYNTH_API short* fluid_mbuf_consume_buffer(fluid_mbuf_renderer_t* dev, int reqLen, int *startOffset, int *readLen, int *bufMax);


#ifdef __cplusplus
}
#endif

#endif //_FLUID_MBUFRENDERER_H

