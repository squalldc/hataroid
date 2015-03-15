/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"
#include "SDL_thread.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_android-ogles2video.h"
#include "SDL_android-ogles2events_c.h"
#include "SDL_android-ogles2mouse_c.h"

#define ANDROID_OGLES2_DRIVER_NAME "android-ogles2"

#if defined(__ARM_ARCH_5__)
	//#define __USE_ARM_ASM__
#endif

/* globals */
volatile int g_videoFrameReady = 0;
volatile int g_curVideoFrame = 0;
volatile int g_doubleBuffer = 1;

SDL_mutex* g_videoTex_mutex = 0;
int g_videoTex_width = 0;
int g_videoTex_height = 0;
int g_videoTex_Bpp = 16;
void *g_videoTex_pixels = 0;
int g_surface_width = 0;
int g_surface_height = 0;
int g_videoModeChanged = 0;

/* Initialization/Query functions */
static int ANDROID_OGLES2_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **ANDROID_OGLES2_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *ANDROID_OGLES2_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int ANDROID_OGLES2_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void ANDROID_OGLES2_VideoQuit(_THIS);

/* Hardware surface functions */
static int ANDROID_OGLES2_AllocHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_OGLES2_LockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_OGLES2_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_OGLES2_FreeHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_OGLES2_FlipHWSurface(_THIS, SDL_Surface *surface);


/* etc. */
static void ANDROID_OGLES2_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

#if defined(__USE_ARM_ASM__)
static void updateRectASM_s32d32(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects);
#else
static void updateRectC_s32d32(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects);
static void updateRectC_s16d16(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects);
static void updateRectC_s8d16(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects);
#endif

/* ANDRROID_OGLES2 driver bootstrap functions */

static int ANDROID_OGLES2_Available(void)
{
	const char *envr = SDL_getenv("SDL_VIDEODRIVER");
	if ((envr) && (SDL_strcmp(envr, ANDROID_OGLES2_DRIVER_NAME) == 0)) {
		return(1);
	}

	return(0);
}

static void ANDROID_OGLES2_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	device->hidden = 0;
	SDL_free(device);
	device = 0;
}

static SDL_VideoDevice *ANDROID_OGLES2_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
		SDL_malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
			device = 0;
		}
		return(0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = ANDROID_OGLES2_VideoInit;
	device->ListModes = ANDROID_OGLES2_ListModes;
	device->SetVideoMode = ANDROID_OGLES2_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = ANDROID_OGLES2_SetColors;
	device->UpdateRects = ANDROID_OGLES2_UpdateRects;
	device->VideoQuit = ANDROID_OGLES2_VideoQuit;
	device->AllocHWSurface = ANDROID_OGLES2_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = ANDROID_OGLES2_LockHWSurface;
	device->UnlockHWSurface = ANDROID_OGLES2_UnlockHWSurface;
	device->FlipHWSurface = NULL;
	device->FreeHWSurface = ANDROID_OGLES2_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = ANDROID_OGLES2_InitOSKeymap;
	device->PumpEvents = ANDROID_OGLES2_PumpEvents;

	device->free = ANDROID_OGLES2_DeleteDevice;

	return device;
}

VideoBootStrap ANDROID_OGLES2_bootstrap = {
	ANDROID_OGLES2_DRIVER_NAME, "SDL Android OpenGL ES 2.0 video driver",
	ANDROID_OGLES2_Available, ANDROID_OGLES2_CreateDevice
};


int ANDROID_OGLES2_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	g_videoTex_mutex = SDL_CreateMutex();

	/* Determine the screen depth (use default 32-bit depth) */
	/* we change this during the SDL_SetVideoMode implementation... */

	vformat->BitsPerPixel = g_videoTex_Bpp;
	vformat->BytesPerPixel = g_videoTex_Bpp>>3;

	vformat->Rmask = 0xF800;
    vformat->Gmask = 0x07E0;
    vformat->Bmask = 0x001F;

	/* We're done! */
	return(0);
}

SDL_Rect **ANDROID_OGLES2_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return (SDL_Rect **) -1;
}

static unsigned int roundUpPower2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	v += (v == 0);

	return v;
}

SDL_Surface *_setAndroidVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);

SDL_Surface *ANDROID_OGLES2_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	SDL_LockMutex(g_videoTex_mutex);
	SDL_Surface *s = _setAndroidVideoMode(this, current, width, height, bpp, flags);
	SDL_UnlockMutex(g_videoTex_mutex);

	return s;
}

SDL_Surface *_setAndroidVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags)
{
	int t;

	if (bpp == 0)
	{
		bpp = 16;
	}

	int texw = roundUpPower2(width);
    int texh = roundUpPower2(height);

	Debug_Printf("Setting VideoMode: %d x %d x %d", width, height, bpp);

	if ( this->hidden->buffer ) {
		SDL_free( this->hidden->buffer );
		this->hidden->buffer = 0;
	}

	g_videoFrameReady = 0;
	g_curVideoFrame = 0;

//	SDL_LockMutex(g_videoTex_mutex);
	for (t = 0; t < 2; ++t)
	{
		if ( this->hidden->texbuf[t] )
		{
			// always free (it might fragment more, but is better for rendering performance since we can't just update part of the full tex in android ogles2)
			/*if (texw > this->hidden->texw
			 || texh > this->hidden->texh)
			*/
			{
				SDL_free( this->hidden->texbuf[t] );
				this->hidden->texbuf[t] = 0;

				g_videoTex_width = 0;
				g_videoTex_height = 0;
				g_videoTex_pixels = 0;

				g_surface_width = 0;
				g_surface_height = 0;
				g_videoModeChanged = 1;
			}
		}
	}
	this->hidden->texw = 0;
	this->hidden->texh = 0;
//	SDL_UnlockMutex(g_videoTex_mutex);

    this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	if ( ! this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}
	SDL_memset(this->hidden->buffer, 0, width * height * (bpp / 8));

//	SDL_LockMutex(g_videoTex_mutex);
	for (t = 0; t < 2; ++t)
	{
		if (!this->hidden->texbuf[t])
		{
			this->hidden->texbuf[t] = SDL_malloc(texw * texh * (g_videoTex_Bpp/8));
			if ( ! this->hidden->texbuf[t] ) {
				SDL_SetError("Couldn't allocate texture buffer for requested mode");
				return(NULL);
			}

			g_videoTex_width = texw;
			g_videoTex_height = texh;
			g_videoModeChanged = 1;

			SDL_memset(this->hidden->texbuf[t], 0, texw * texh * (g_videoTex_Bpp / 8));
		}
	}
	g_videoTex_pixels = this->hidden->texbuf[g_curVideoFrame];
	this->hidden->texw = texw;
	this->hidden->texh = texh;

	g_surface_width = width;
    g_surface_height = height;
//	SDL_UnlockMutex(g_videoTex_mutex);

/* 	printf("Setting mode %dx%d\n", width, height); */

	/* Allocate the new pixel format for the screen */
    Uint32 Rmask, Gmask, Bmask;
    if (bpp == 16) {
		Rmask = 0xF800;
		Gmask = 0x07E0;
		Bmask = 0x001F;
    } else {
    	Rmask = Gmask = Bmask = 0;
    }

    if ( ! SDL_ReallocFormat(current, bpp, Rmask, Gmask, Bmask, 0) ) {
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	/* Set up the new mode framebuffer */
	current->flags = flags & SDL_FULLSCREEN;
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;

	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int ANDROID_OGLES2_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}
static void ANDROID_OGLES2_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int ANDROID_OGLES2_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void ANDROID_OGLES2_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static void ANDROID_OGLES2_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	int doubleBuffer = g_doubleBuffer;
	if (doubleBuffer==0)
	{
		SDL_LockMutex(g_videoTex_mutex); // not worth it, just double buffer it and accept the occasional tear
	}

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = g_surface_width;
	rect.h = g_surface_height;

	int srcBpp = this->screen->format->BitsPerPixel;
	#if defined(__USE_ARM_ASM__)
		updateRectASM_s32d32(this->screen, this->hidden->texbuf[g_curVideoFrame], this->hidden->texw, this->hidden->texh, 1, &rect);
	#else
		if (srcBpp == 16)
		{
			updateRectC_s16d16(this->screen, this->hidden->texbuf[g_curVideoFrame], this->hidden->texw, this->hidden->texh, 1, &rect);
		}
		else if (srcBpp == 8)
		{
			updateRectC_s8d16(this->screen, this->hidden->texbuf[g_curVideoFrame], this->hidden->texw, this->hidden->texh, 1, &rect);
		}
	#endif

	if (doubleBuffer==0)
	{
		g_curVideoFrame = 1-g_curVideoFrame;
		g_videoTex_pixels = this->hidden->texbuf[g_curVideoFrame];
		g_videoFrameReady = 1;

		SDL_UnlockMutex(g_videoTex_mutex);
	}
	else
	{
		g_videoTex_pixels = this->hidden->texbuf[g_curVideoFrame];
		g_curVideoFrame = 1-g_curVideoFrame;
		g_videoFrameReady = 1;
	}

	return;
}

static void ANDROID_OGLES2_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	if (numrects == 0)
	{
		ANDROID_OGLES2_FlipHWSurface(this, 0);
	}
}

int ANDROID_OGLES2_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	/* do nothing of note. */
	return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void ANDROID_OGLES2_VideoQuit(_THIS)
{
	int t;

	SDL_LockMutex(g_videoTex_mutex);

	if (this->screen->pixels != NULL)
	{
		SDL_free(this->screen->pixels);
		this->screen->pixels = NULL;
	}

	{
		for (t = 0; t < 2; ++t)
		{
			if ( this->hidden->texbuf[t] )
			{
				SDL_free( this->hidden->texbuf[t] );
				this->hidden->texbuf[t] = 0;
				this->hidden->texw = 0;
				this->hidden->texh = 0;

				g_videoTex_width = 0;
				g_videoTex_height = 0;
				g_videoTex_pixels = 0;

				g_surface_width = 0;
				g_surface_height = 0;
			}
		}
	}
	SDL_UnlockMutex(g_videoTex_mutex);

	SDL_DestroyMutex(g_videoTex_mutex);
	g_videoTex_mutex = 0;
}

#if defined(__USE_ARM_ASM__)

#define REDBLUE_SWAP32(dst, src, initMask, tmp) \
	__asm__ __volatile__ ( \
		"bic    %[dstu], %[srcu], %[mask]\n\t" \
		"bic    %[tmpu], %[srcu], %[mask], ror #8\n\t" \
		"orr	%[dstu], %[dstu], %[tmpu], ror #16\n\t" \
		: [dstu] "+r" (dst) \
		: [srcu] "r" (src), [mask] "r" (initMask), [tmpu] "r" (tmp) \
	);

static void updateRectASM_s32d32(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects)
{
	int r, x, y;
	int bpp = surface->format->BitsPerPixel;
	if (bpp != 32)
	{
		return;
	}

	for (r = 0; r < numrects; ++r)
	{
		int rx = rects[r].x;
		int ry = rects[r].y;
		int rx2 = rects[r].w + rx;
		int ry2 = rects[r].h + ry;

		int x2 = rx2 > dstw ? dstw : rx2;
		int y2 = ry2 > dsth ? dsth : ry2;

		uint32_t *sDstPixels = ((uint32_t*)dstBuf) + rx;
		uint8_t *sSrcPixels = ((uint8_t*)surface->pixels) + rx*4;

		register uint32_t initMask = 0x00FF00FF;
		register uint32_t src;
		register uint32_t dst, tmp;
		for (y = ry; y < y2; ++y)
		{
			uint32_t *sDst = sDstPixels + dstw * y;
			uint32_t *sSrc = (uint32_t*)(sSrcPixels + surface->pitch * y);

			// temp only, should be way to flip these during screen generation in the emulator
			for (x = rx; x < x2; ++x)
			{
				src = *sSrc;
				REDBLUE_SWAP32(dst, src, initMask, tmp);
				*sDst = dst;

				++sDst;
				++sSrc;
			}
		}
	}
}

#else

static void updateRectC_s16d16(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects)
{
	int r, x, y;

	for (r = 0; r < numrects; ++r)
	{
		int rx = rects[r].x;
		int ry = rects[r].y;
		int rx2 = rects[r].w + rx;
		int ry2 = rects[r].h + ry;

		int x2 = rx2 > dstw ? dstw : rx2;
		int y2 = ry2 > dsth ? dsth : ry2;

		uint16_t *sDstPixels = ((uint16_t*)dstBuf) + rx;
		uint8_t *sSrcPixels = ((uint8_t*)surface->pixels) + (rx<<1);

		for (y = ry; y < y2; ++y)
		{
			uint16_t *sDst = sDstPixels + dstw * y;
			uint16_t *sSrc = (uint16_t*)(sSrcPixels + surface->pitch * y);

			for (x = rx; x < x2; ++x)
			{
				*sDst = *sSrc;

				++sDst;
				++sSrc;
			}
		}
	}
}

static void updateRectC_s8d16(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects)
{
	int r, x, y;

	SDL_Color *colors = surface->format->palette->colors;

	for (r = 0; r < numrects; ++r)
	{
		int rx = rects[r].x;
		int ry = rects[r].y;
		int rx2 = rects[r].w + rx;
		int ry2 = rects[r].h + ry;

		int x2 = rx2 > dstw ? dstw : rx2;
		int y2 = ry2 > dsth ? dsth : ry2;

		uint16_t *sDstPixels = ((uint16_t*)dstBuf) + rx;
		uint8_t *sSrcPixels = ((uint8_t*)surface->pixels);

		for (y = ry; y < y2; ++y)
		{
			uint16_t *sDst = sDstPixels + dstw * y;
			uint8_t *sSrc = (uint8_t*)(sSrcPixels + surface->pitch * y);

			for (x = rx; x < x2; ++x)
			{
				SDL_Color *sColor = &colors[*sSrc];
				uint8_t r = sColor->r;
				uint8_t g = sColor->g;
				uint8_t b = sColor->b;

				uint16_t flipped = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
				*sDst = flipped;

				++sDst;
				++sSrc;
			}
		}
	}
}

static void updateRectC_s32d32(SDL_Surface *surface, uint8_t *dstBuf, int dstw, int dsth, int numrects, SDL_Rect *rects)
{
	int r, x, y;

	for (r = 0; r < numrects; ++r)
	{
		int rx = rects[r].x;
		int ry = rects[r].y;
		int rx2 = rects[r].w + rx;
		int ry2 = rects[r].h + ry;

		int x2 = rx2 > dstw ? dstw : rx2;
		int y2 = ry2 > dsth ? dsth : ry2;

		uint32_t *sDstPixels = ((uint32_t*)dstBuf) + rx;
		uint8_t *sSrcPixels = ((uint8_t*)surface->pixels) + (rx<<2);

		for (y = ry; y < y2; ++y)
		{
			uint32_t *sDst = sDstPixels + dstw * y;
			uint32_t *sSrc = (uint32_t*)(sSrcPixels + surface->pitch * y);

			for (x = rx; x < x2; ++x)
			{
				uint32_t col = *sSrc;
				uint32_t flipped = (col & 0xff00ff00) | ((col >> 16) & 0xff) | ((col << 16) & 0xff0000);
				*sDst = flipped;

				++sDst;
				++sSrc;
			}
		}
	}
}

#endif
