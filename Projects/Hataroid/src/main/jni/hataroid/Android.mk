LOCAL_PATH:= $(call my-dir)

# glue
include $(CLEAR_VARS)
LOCAL_MODULE    := libhataroid
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/SDL/include
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/hatari/src/gui-android
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/hatari/src/includes
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/include
LOCAL_SRC_FILES := src/hataroid.cpp src/nativeRenderer_ogles2.cpp src/uncompressGZ.cpp
LOCAL_SRC_FILES += src/virtKBDefs.c src/virtKBTex.c src/VirtKB.cpp src/ShortcutMap.cpp src/virtJoy.cpp
LOCAL_SRC_FILES += src/RTShader.cpp
LOCAL_SRC_FILES += src/midi/fsmidi.c
LOCAL_LDLIBS    := -llog -lGLESv2 -lz

# for EAS midi
#LOCAL_LDLIBS	+= extlibs\armeabi-v7a\libsonivox.so

# -lOpenSLES

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS)
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

LOCAL_STATIC_LIBRARIES := libhatari libSDL libfluidsynth libglib

include $(BUILD_SHARED_LIBRARY)
