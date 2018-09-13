LOCAL_PATH:= $(call my-dir)

# glue
include $(CLEAR_VARS)

LOCAL_MODULE    := libhataroid

LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/SDL/include
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/hatari/cmake
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/hatari/src/gui-android
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/hatari/src/includes
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/include
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/fluidsynth/include
#LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/mt32emu/src

#LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/android-ndk-profiler/src

LOCAL_SRC_FILES := src/hataroid.cpp src/nativeRenderer_ogles2.cpp src/uncompressGZ.cpp
LOCAL_SRC_FILES += src/virtKBDefs.c src/VirtKB.cpp src/ShortcutMap.cpp src/virtJoy.cpp
LOCAL_SRC_FILES += src/virtKBTex.c src/virtKBTexDE.c src/virtKBTexFR.c
LOCAL_SRC_FILES += src/RTShader.cpp src/Debug.cpp
LOCAL_SRC_FILES += src/midi/fsmidi.c
LOCAL_SRC_FILES += src/FloppySnd.c
#LOCAL_SRC_FILES += src/midi/mt32.cpp

LOCAL_LDLIBS    := -lOpenSLES -llog -lGLESv2 -lz

# for EAS midi
#LOCAL_LDLIBS	+= extlibs\armeabi-v7a\libsonivox.so

# -lOpenSLES

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS)
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

LOCAL_STATIC_LIBRARIES := libhatari libSDL libfluidsynth libglib
# libmt32emu
#LOCAL_STATIC_LIBRARIES += libandroid-ndk-profiler

include $(BUILD_SHARED_LIBRARY)
