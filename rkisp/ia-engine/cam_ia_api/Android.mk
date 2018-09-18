#ifeq ($(IS_CAM_IA10_API),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(IS_RK_ISP10),true)
LOCAL_CPPFLAGS += -D RK_ISP10
endif
ifeq ($(IS_RK_ISP11),true)
LOCAL_CPPFLAGS += -D RK_ISP11
endif

LOCAL_SRC_FILES:= \
	cam_ia10_engine.cpp \
	cam_ia10_engine_isp_modules.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/linux \
    $(LOCAL_PATH)/../include/linux/media \
    $(LOCAL_PATH)/../../isp-engine \
	$(LOCAL_PATH)/../../../xcore \
	$(LOCAL_PATH)/../../../xcore/ia \
	$(LOCAL_PATH)/../../../plugins/3a/rkiq \

ifeq ($(IS_NEED_COMPILE_TINYXML2), true)
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../../../ext/tinyxml2 \

else
LOCAL_C_INCLUDES += \
    external/tinyxml2 \

endif
LOCAL_CFLAGS += -std=c99 -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CPPFLAGS += -D_GLIBCXX_USE_C99=1 -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += -std=c++11
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_STATIC_LIBRARIES += libisp_aaa_adpf libisp_aaa_awdr libtinyxml2 libisp_cam_calibdb libisp_calibdb libisp_oslayer libisp_ebase

LOCAL_MODULE:= libisp_ia_engine
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)

#endif
