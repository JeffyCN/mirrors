#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	source/oslayer_generic.c\
	source/oslayer_linux.c\


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include\
	$(LOCAL_PATH)/include_priv\
	$(LOCAL_PATH)/../include/

LOCAL_CFLAGS += -Wall -Wextra -std=c99   -Wformat-nonliteral -DLINUX -g -O0 -pedantic -Wno-long-long
LOCAL_CFLAGS += -DLINUX -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)   
#LOCAL_LDLIBS +=-lc
#LOCAL_LDFLAGS:=-lc -lpthread

#LOCAL_MODULE_RELATIVE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE:= libisp_oslayer
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)
