/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/* fluid_android.c
 *
 * Audio driver, outputs to android memory buffer (non real-time)
 *
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"
#include "fluid_sys.h"
#include "config.h"
#include <stdio.h>
#include "fluid_midi.h"
#include "fluid_mbufrenderer.h"


/** fluid_android_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct {
	fluid_audio_driver_t driver; // Don't move or delete this!
	//fluid_audio_func_t callback;
	void* data;
	fluid_mbuf_renderer_t* renderer;

	//int period_size;
	//double sample_rate;
	//fluid_timer_t* timer;
	//unsigned int samples;

} fluid_android_audio_driver_t;


fluid_audio_driver_t* new_fluid_android_audio_driver(fluid_settings_t* settings,
						  fluid_synth_t* synth);

int delete_fluid_android_audio_driver(fluid_audio_driver_t* p);

//static int fluid_android_audio_run_s16(void* d, unsigned int msec);

/**************************************************************
 *
 *        'file' audio driver
 *
 */

fluid_audio_driver_t*
new_fluid_android_audio_driver(fluid_settings_t* settings,
			    fluid_synth_t* synth)
{
	fluid_android_audio_driver_t* dev;
//	int msec;

	dev = FLUID_NEW(fluid_android_audio_driver_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_android_audio_driver_t));

//	fluid_settings_getint(settings, "audio.period-size", &dev->period_size);
//	fluid_settings_getnum(settings, "synth.sample-rate", &dev->sample_rate);

	dev->data = synth;
//	dev->callback = (fluid_audio_func_t) fluid_synth_process;
//	dev->samples = 0;

	dev->renderer = new_fluid_mbuf_renderer(synth);

	if (dev->renderer == NULL)
		goto error_recovery;

//	int msec = (int) (0.5 + dev->period_size / dev->sample_rate * 1000.0);
//	dev->timer = new_fluid_timer(msec, fluid_android_audio_run_s16, (void*) dev, TRUE, FALSE, TRUE);
//	if (dev->timer == NULL) {
//		FLUID_LOG(FLUID_PANIC, "Couldn't create the audio thread.");
//		goto error_recovery;
//	}

	return (fluid_audio_driver_t*) dev;

 error_recovery:
	delete_fluid_android_audio_driver((fluid_audio_driver_t*) dev);
	return NULL;
}

int delete_fluid_android_audio_driver(fluid_audio_driver_t* p)
{
	fluid_android_audio_driver_t* dev = (fluid_android_audio_driver_t*) p;

	if (dev == NULL) {
		return FLUID_OK;
	}

//	if (dev->timer != NULL) {
//		delete_fluid_timer(dev->timer);
//	}

	if (dev->renderer != NULL) {
		delete_fluid_mbuf_renderer(dev->renderer);
	}

	FLUID_FREE(dev);
	return FLUID_OK;
}

/*
static int fluid_android_audio_run_s16(void* d, unsigned int clock_time)
{
	fluid_android_audio_driver_t* dev = (fluid_android_audio_driver_t*) d;
	unsigned int sample_time;

	sample_time = (unsigned int) (dev->samples / dev->sample_rate * 1000.0);
	if (sample_time > clock_time) {
		return 1;
	}

	dev->samples += dev->period_size;

	return fluid_mbuf_renderer_process_block(dev->renderer) == FLUID_OK ? 1 : 0;
}
//*/

int fluid_android_process_frames(fluid_audio_driver_t* d, int nframes)
{
	fluid_android_audio_driver_t* dev = (fluid_android_audio_driver_t*) d;

	return fluid_mbuf_renderer_process_block_frames(dev->renderer, nframes) == FLUID_OK ? 1 : 0;
}

short* fluid_android_consume_buffer(fluid_audio_driver_t* d, int reqLenShorts, int *startOffsetShorts, int *readLenShorts, int *bufMaxShorts)
{
	fluid_android_audio_driver_t* dev = (fluid_android_audio_driver_t*) d;

	return fluid_mbuf_consume_buffer(dev->renderer, reqLenShorts, startOffsetShorts, readLenShorts, bufMaxShorts);
}

void* fluid_android_create_midi_parser()
{
	fluid_midi_parser_t* parser = new_fluid_midi_parser();
	return parser;

}
void fluid_android_destroy_midi_parser(void *parser)
{
	if (parser)
	{
		delete_fluid_midi_parser((fluid_midi_parser_t*)parser);
	}
}

fluid_midi_event_t* fluid_android_parse_midi_byte(void *parser, unsigned char b)
{
	fluid_midi_parser_t* midiParser = (fluid_midi_parser_t*)parser;

	return fluid_midi_parser_parse(midiParser, b);
}
