LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE		:= libSDL

LOCAL_C_INCLUDES	:= jni/SDL/include
LOCAL_C_INCLUDES	+= jni/hataroid/src

#--- audio
LOCAL_SRC_FILES		+= src/audio/SDL_audio.c src/audio/SDL_audiocvt.c src/audio/SDL_audiodev.c src/audio/SDL_wave.c
LOCAL_SRC_FILES		+= src/audio/dummy/SDL_dummyaudio.c
LOCAL_SRC_FILES		+= src/audio/android_audiotrack/SDL_android_audiotrackaudio.c

#--- cdrom
LOCAL_SRC_FILES		+= src/cdrom/SDL_cdrom.c
LOCAL_SRC_FILES		+= src/cdrom/dummy/SDL_syscdrom.c

#--- cpuinfo
LOCAL_SRC_FILES		+= src/cpuinfo/SDL_cpuinfo.c

#--- events
LOCAL_SRC_FILES		+= src/events/SDL_active.c src/events/SDL_events.c src/events/SDL_expose.c \
						src/events/SDL_keyboard.c src/events/SDL_mouse.c src/events/SDL_quit.c src/events/SDL_resize.c

#--- file
LOCAL_SRC_FILES		+= src/file/SDL_rwops.c

#--- joystick
LOCAL_SRC_FILES		+= src/joystick/SDL_joystick.c
LOCAL_SRC_FILES		+= src/joystick/dummy/SDL_sysjoystick.c

#--- loadso
LOCAL_SRC_FILES		+= src/loadso/dummy/SDL_sysloadso.c

#--- stdlib
LOCAL_SRC_FILES		+= src/stdlib/SDL_getenv.c src/stdlib/SDL_iconv.c src/stdlib/SDL_malloc.c \
						src/stdlib/SDL_qsort.c src/stdlib/SDL_stdlib.c src/stdlib/SDL_string.c
#--- thread
LOCAL_SRC_FILES		+= src/thread/SDL_thread.c
LOCAL_SRC_FILES		+= src/thread/pthread/SDL_syscond.c src/thread/pthread/SDL_sysmutex.c \
						src/thread/pthread/SDL_syssem.c src/thread/pthread/SDL_systhread.c
#--- video
LOCAL_SRC_FILES		+= src/video/SDL_blit.c src/video/SDL_blit_0.c src/video/SDL_blit_1.c \
						src/video/SDL_blit_A.c src/video/SDL_blit_N.c src/video/SDL_bmp.c \
						src/video/SDL_cursor.c src/video/SDL_gamma.c src/video/SDL_pixels.c \
						src/video/SDL_RLEaccel.c src/video/SDL_stretch.c src/video/SDL_surface.c \
						src/video/SDL_video.c src/video/SDL_yuv.c src/video/SDL_yuv_sw.c
LOCAL_SRC_FILES		+= src/video/dummy/SDL_nullevents.c src/video/dummy/SDL_nullmouse.c src/video/dummy/SDL_nullvideo.c
LOCAL_SRC_FILES		+= src/video/android-ogles2/SDL_android-ogles2events.c src/video/android-ogles2/SDL_android-ogles2mouse.c src/video/android-ogles2/SDL_android-ogles2video.c

#--- timer
LOCAL_SRC_FILES		+= src/timer/SDL_timer.c
LOCAL_SRC_FILES		+= src/timer/unix/SDL_systimer.c

#--- main
LOCAL_SRC_FILES		+= src/SDL.c src/SDL_error.c src/SDL_fatal.c

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS)
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

include $(BUILD_STATIC_LIBRARY)
