#
# RockChip Camera HAL 
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	cam_types.cpp \
	cam_thread.cpp \
	V4l2Isp10Ioctl.cpp \
	isp_engine.cpp \
	isp10_engine.cpp \
	isp_ctrl.cpp

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CPPFLAGS += -D_GLIBCXX_USE_C99=1 -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += -std=c++11
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

ifeq ($(IS_ANDROID_OS),true)
#LOCAL_CPPFLAGS += -DANDROID_OS
#LOCAL_C_INCLUDES += external/stlport/stlport bionic/ bionic/libstdc++/include system/core/libion/include/ \
#		    system/core/include
LOCAL_SHARED_LIBRARIES += libcutils liblog
#LOCAL_MODULE_RELATIVE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

ifeq ($(IS_NEED_SHARED_PTR),true)
LOCAL_CPPFLAGS += -D ANDROID_SHARED_PTR
endif

ifeq ($(IS_USE_RK_V4L2_HEAD),true)
LOCAL_CPPFLAGS += -D USE_RK_V4L2_HEAD_FILES
endif

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../xcore \
	$(LOCAL_PATH)/../../xcore/ia \
	$(LOCAL_PATH)/../../plugins/3a/rkiq \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/linux \
	$(LOCAL_PATH)/../ia-engine \
    $(LOCAL_PATH)/../ia-engine/include \
    $(LOCAL_PATH)/../ia-engine/include/linux \
    $(LOCAL_PATH)/../ia-engine/include/linux/media \

ifeq ($(IS_NEED_COMPILE_TINYXML2), true)
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../../ext/tinyxml2 \

else
LOCAL_C_INCLUDES += \
    external/tinyxml2 \

endif

LOCAL_STATIC_LIBRARIES += \
	librkisp_ctrlloop \
	libisp_ia_engine


LOCAL_STATIC_LIBRARIES += libisp_ebase libisp_oslayer

ifeq ($(IS_NEED_LINK_STLPORT),true)
LOCAL_SHARED_LIBRARIES += libstlport
endif

ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
endif

LOCAL_MODULE:= librkisp_isp_engine
include $(BUILD_STATIC_LIBRARY)
