ifeq ($(IS_ANDROID_OS),true)
include $(call all-subdir-makefiles)
else
include $(call allSubdirMakefiles)
endif
