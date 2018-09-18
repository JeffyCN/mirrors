#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
	source/cam_calibdb.c\
	source/cam_calibdb_api.c\


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include\
	$(LOCAL_PATH)/include_priv\
	$(LOCAL_PATH)/../include/ \
	$(LOCAL_PATH)/../../../xcore

LOCAL_CFLAGS := -Wall -Wextra -std=c99 -Wno-unused -Wformat-nonliteral -g -O0 -pedantic
LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
#LOCAL_CFLAGS += -DDEBUG
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)

LOCAL_STATIC_LIBRARIES := libisp_ebase libisp_oslayer

LOCAL_MODULE:= libisp_cam_calibdb
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)
