MY_DEBUG := false
MY_APP_MANIFEST := $(strip $(wildcard $(APP_PROJECT_PATH)/AndroidManifest.xml))
ifdef MY_APP_MANIFEST
	MY_DEBUG := $(shell $(HOST_AWK) -f $(BUILD_AWK)/extract-debuggable.awk $(APP_MANIFEST))
endif

MY_LOCAL_ARM_MODE	:= arm
ifeq ($(MY_DEBUG),false)
	MY_LOCAL_CFLAGS		:= -ffast-math -O3 -funroll-loops -marm
endif

include $(call all-subdir-makefiles)
