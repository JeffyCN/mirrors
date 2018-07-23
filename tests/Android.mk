ifeq ($(PLATFORM_SDK_VERSION),)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	rkisp_demo.cpp

LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CPPFLAGS += -DLINUX 
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../interface \
	$(LOCAL_PATH)/../metadata/header_files/include \
	$(LOCAL_PATH)/../metadata/libcamera_client/include \
	$(LOCAL_PATH)/../metadata/libcamera_metadata/include \
	$(LOCAL_PATH)/../metadata/header_files/include/system/core/include \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../xcore \
	$(LOCAL_PATH)/../xcore/ia \
	$(LOCAL_PATH)/../ext/rkisp \
	$(LOCAL_PATH)/../plugins/3a/rkiq \
	$(LOCAL_PATH)/../modules/isp \
	$(LOCAL_PATH)/../rkisp/ia-engine \
	$(LOCAL_PATH)/../rkisp/ia-engine/include \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux/media \
	$(LOCAL_PATH)/../rkisp/isp-engine

LOCAL_SHARED_LIBRARIES += libdl

ifeq ($(IS_ANDROID_OS),true)
LOCAL_SHARED_LIBRARIES += libutils libcutils
endif

LOCAL_MODULE:= rkisp_demo

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	test_camcl.cpp

LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CPPFLAGS += -DLINUX 
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../interface \
	$(LOCAL_PATH)/../metadata/header_files/include \
	$(LOCAL_PATH)/../metadata/libcamera_client/include \
	$(LOCAL_PATH)/../metadata/libcamera_metadata/include \
	$(LOCAL_PATH)/../metadata/header_files/include/system/core/include \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../xcore \
	$(LOCAL_PATH)/../xcore/ia \
	$(LOCAL_PATH)/../ext/rkisp \
	$(LOCAL_PATH)/../plugins/3a/rkiq \
	$(LOCAL_PATH)/../modules/isp \
	$(LOCAL_PATH)/../rkisp/ia-engine \
	$(LOCAL_PATH)/../rkisp/ia-engine/include \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux/media \
	$(LOCAL_PATH)/../rkisp/isp-engine

LOCAL_STATIC_LIBRARIES := \
    librkisp_analyzer \
    librkisp_isp_engine \
    libisp_ia_engine \
    librkisp_ctrlloop \
    libstdc++

LOCAL_STATIC_LIBRARIES += \
    libisp_aaa_adpf \
    libisp_aaa_awdr \
    libisp_cam_calibdb \
    libisp_calibdb \
    libtinyxml2 \
    libisp_oslayer \
    libisp_ebase

LOCAL_SHARED_LIBRARIES += libdl librkisp libdrm

ifeq ($(IS_ANDROID_OS),false)
LOCAL_SHARED_LIBRARIES += libgobject-2.0 libglib-2.0
endif

ifeq ($(IS_ANDROID_OS),true)
LOCAL_SHARED_LIBRARIES += libutils libcutils
endif

LOCAL_MODULE:= test_ispcl

include $(BUILD_EXECUTABLE)
endif
