#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
	source/awdr.c


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include\
	$(LOCAL_PATH)/include_priv\
	$(LOCAL_PATH)/../../include/ \
	$(LOCAL_PATH)/../../../../xcore

LOCAL_CFLAGS := -std=c99  -Wformat-nonliteral -g -O0 -DDEBUG -pedantic -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
#LOCAL_STATIC_LIBRARIES := libisp_ebase libisp_oslayer libisp_common libisp_hal libisp_isi libisp_cam_calibdb libisp_cameric_reg_drv libisp_cameric_drv

#LOCAL_MODULE_RELATIVE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE:= libisp_aaa_awdr
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)
