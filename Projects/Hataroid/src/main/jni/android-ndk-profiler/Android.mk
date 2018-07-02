LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -fvisibility=hidden -DDEBUG
LOCAL_MODULE    := libandroid-ndk-profiler
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/android-ndk-profiler/src
LOCAL_SRC_FILES := src/gnu_mcount.S src/prof.c src/read_maps.c

include $(BUILD_STATIC_LIBRARY)
