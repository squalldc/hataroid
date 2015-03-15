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

 /*
  * Low-level routines for memory buffer output.
  */

#include <stdio.h>
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_sys.h"
#include "fluid_settings.h"
#include "fluid_mbufrenderer.h"

struct _fluid_mbuf_renderer_t {
	fluid_synth_t* synth;

	short* buf;

	//int period_size;
	//int buf_size;
	//int buf_period_count;

	//int cur_buf_read_len;
	//int cur_buf_read_offset;
	//int cur_buf_write_period_offset;

	int max_buf_frames;
	int max_buf_shorts;
	int cur_buf_shorts;
};

void
fluid_mbuf_renderer_settings (fluid_settings_t* settings)
{
  fluid_settings_register_str(settings, "audio.mbuf.format", "s16", 0, NULL, NULL);
  fluid_settings_add_option (settings, "audio.mbuf.format", "s16");
  fluid_settings_register_str(settings, "audio.mbuf.endian", "cpu", 0, NULL, NULL);
  fluid_settings_add_option (settings, "audio.mbuf.endian", "cpu");
}

/**
 * Create a new memory buffer renderer
 * @param synth The synth that creates audio data.
 * @return the new object, or NULL on failure
 * @since 1.1.0
 *
 * Uses the following settings from the synth object:
 *   - audio.mbuf.format: Audio format
 *   - audio.mbuf.endian: Endian byte order, "auto" for file type's default byte order
 *   - audio.period-size: Size of audio blocks to process
 *   - synth.sample-rate: Sample rate to use
 */
fluid_mbuf_renderer_t *
new_fluid_mbuf_renderer(fluid_synth_t* synth)
{
	fluid_mbuf_renderer_t* dev;

	fluid_return_val_if_fail (synth != NULL, NULL);
	fluid_return_val_if_fail (synth->settings != NULL, NULL);

	dev = FLUID_NEW(fluid_mbuf_renderer_t);
	if (dev == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(dev, 0, sizeof(fluid_mbuf_renderer_t));

	dev->synth = synth;

	/*
	fluid_settings_getint (synth->settings, "audio.period-size", &dev->period_size);

	dev->buf_period_count = 1;

	dev->buf_size = 2 * dev->period_size * sizeof (short) * dev->buf_period_count;
	dev->buf = FLUID_ARRAY(short, 2 * dev->period_size * dev->buf_period_count);
	*/

	dev->max_buf_frames = 4096;
	dev->max_buf_shorts = dev->max_buf_frames * 2;
	dev->cur_buf_shorts = 0;
	dev->buf = FLUID_ARRAY(short, dev->max_buf_shorts);

	if (dev->buf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		goto error_recovery;
	}

	return dev;

 error_recovery:
	delete_fluid_mbuf_renderer(dev);
	return NULL;
}

/**
 * Destroy a memory buffer renderer object.
 * @param dev Memory Buffer renderer object.
 * @since 1.1.0
 */
void delete_fluid_mbuf_renderer(fluid_mbuf_renderer_t* dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->buf != NULL) {
		FLUID_FREE(dev->buf);
	}

	FLUID_FREE(dev);
	return;
}

/**
 * Write period_size samples to buffer.
 * @param dev Memory Buffer renderer instance
 * @return #FLUID_OK or #FLUID_FAILED if an error occurred
 * @since 1.1.0
 */
/*
int
fluid_mbuf_renderer_process_block(fluid_mbuf_renderer_t* dev)
{
	int bufOffset = dev->cur_buf_write_period_offset * (dev->period_size<<1);

	fluid_synth_write_s16(dev->synth, dev->period_size, dev->buf, bufOffset, 2, dev->buf, bufOffset+1, 2);

	dev->cur_buf_read_len += (dev->period_size<<1);

	++dev->cur_buf_write_period_offset;
	if (dev->cur_buf_write_period_offset >= dev->buf_period_count)
	{
		dev->cur_buf_write_period_offset = 0; // wrap around
	}

	return FLUID_OK;
}
//*/

int
fluid_mbuf_renderer_process_block_frames(fluid_mbuf_renderer_t* dev, int nframes)
{
	if (nframes > dev->max_buf_frames)
	{
		nframes = dev->max_buf_frames;
	}

	fluid_synth_write_s16(dev->synth, nframes, dev->buf, 0, 2, dev->buf, 1, 2);

	dev->cur_buf_shorts = nframes << 1;

	return FLUID_OK;
}

short* fluid_mbuf_consume_buffer(fluid_mbuf_renderer_t* dev, int reqLenShorts, int *startOffsetShorts, int *readLenShorts, int *bufMaxShorts)
{
	(*bufMaxShorts) = dev->max_buf_shorts;

	(*startOffsetShorts) = 0;

	int consumeLenShorts = reqLenShorts;
	if (consumeLenShorts > dev->cur_buf_shorts)
	{
		consumeLenShorts = dev->cur_buf_shorts;
	}
	(*readLenShorts) = consumeLenShorts;

	dev->cur_buf_shorts = 0;

	return dev->buf;
}

/*
short* fluid_mbuf_consume_buffer(fluid_mbuf_renderer_t* dev, int reqLenShorts, int *startOffsetShorts, int *readLenShorts, int *bufMaxShorts)
{
	int bufSizeShorts = (dev->period_size<<1) * dev->buf_period_count;
	(*bufMax) = bufSizeShorts;

	(*startOffset) = dev->cur_buf_read_offset;

	int consumeLen = dev->cur_buf_read_len;
	if (consumeLen > reqLen)
	{
		consumeLen = reqLen;
	}
	(*readLen) = consumeLen;

	dev->cur_buf_read_len -= consumeLen;
	dev->cur_buf_read_offset += consumeLen;
	if (dev->cur_buf_read_offset >= bufSizeShorts)
	{
		dev->cur_buf_read_offset = dev->cur_buf_read_offset - bufSizeShorts;
	}

	return dev->buf;
}
//*/
