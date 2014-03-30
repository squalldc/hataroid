LOCAL_PATH:= $(call my-dir)

# glue
include $(CLEAR_VARS)
LOCAL_MODULE    := libhataroid
LOCAL_C_INCLUDES += jni/SDL/include
LOCAL_C_INCLUDES += jni/hatari/src/gui-android
LOCAL_C_INCLUDES += jni/hatari/src/includes
LOCAL_SRC_FILES := src/hataroid.cpp src/nativeRenderer_ogles2.cpp src/uncompressGZ.cpp
LOCAL_SRC_FILES += src/virtKBDefs.c src/virtKBTex.c src/VirtKB.cpp src/ShortcutMap.cpp
LOCAL_LDLIBS    := -llog -lGLESv2 -lz

# -lOpenSLES

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS)
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

LOCAL_STATIC_LIBRARIES := libhatari libSDL

include $(BUILD_SHARED_LIBRARY)
