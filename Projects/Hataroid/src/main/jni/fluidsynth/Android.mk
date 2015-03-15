LOCAL_PATH:= $(call my-dir)

# fluidsynth-main
include $(CLEAR_VARS)

LOCAL_MODULE    := libfluidsynth
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/include
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/synth
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/utils
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/synth
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/sfloader
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/rvoice
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/midi
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/bindings
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/src/drivers
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/glib
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/glib/glib

#main
LOCAL_SRC_FILES := src/utils/fluid_conv.c \
	src/utils/fluid_hash.c \
	src/utils/fluid_list.c \
	src/utils/fluid_ringbuffer.c \
	src/utils/fluid_settings.c \
	src/utils/fluid_sys.c \
	src/utils/bitflags_c.c \
	src/sfloader/fluid_defsfont.c \
	src/sfloader/fluid_ramsfont.c \
	src/rvoice/fluid_adsr_env.c \
	src/rvoice/fluid_chorus.c \
	src/rvoice/fluid_iir_filter.c \
	src/rvoice/fluid_lfo.c \
	src/rvoice/fluid_rvoice.c \
	src/rvoice/fluid_rvoice_dsp.c \
	src/rvoice/fluid_rvoice_event.c \
	src/rvoice/fluid_rvoice_mixer.c \
	src/rvoice/fluid_rev.c \
	src/synth/fluid_chan.c \
	src/synth/fluid_event.c \
	src/synth/fluid_gen.c \
	src/synth/fluid_mod.c \
	src/synth/fluid_synth.c \
	src/synth/fluid_tuning.c \
	src/synth/fluid_voice.c \
	src/midi/fluid_midi.c \
	src/midi/fluid_midi_router.c \
	src/midi/fluid_seqbind.c \
	src/midi/fluid_seq.c \
	src/drivers/fluid_adriver.c \
	src/drivers/fluid_mdriver.c \
	src/bindings/fluid_cmd.c \
	src/bindings/fluid_filerenderer.c \
	src/bindings/fluid_mbufrenderer.c \
	src/drivers/fluid_aufile.c \
	src/drivers/fluid_android.c

#	src/bindings/fluid_ladspa.c \
#	src/bindings/fluid_lash.c \
#	src/drivers/fluid_alsa.c \
#	src/drivers/fluid_coreaudio.c \
#	src/drivers/fluid_coremidi.c \
#	src/drivers/fluid_jack.c \
#	src/drivers/fluid_oss.c \
#	src/drivers/fluid_portaudio.c \
#	src/drivers/fluid_pulse.c
#	src/fluid_dll.c \
#	src/drivers/fluid_dsound.c \
#	src/drivers/fluid_winmidi.c \
#	src/drivers/fluid_dart.c \
#	src/bindings/fluid_rtkit.c \

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS) -DHAVE_CONFIG_H
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

include $(BUILD_STATIC_LIBRARY)
