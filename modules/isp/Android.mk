LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	aiq3a_utils.cpp \
	rkiq_handler.cpp \
	rkisp_device.cpp \
	hybrid_analyzer.cpp \
	hybrid_analyzer_loader.cpp \
	isp_config_translator.cpp \
	isp_controller.cpp \
	isp_image_processor.cpp \
	isp_poll_thread.cpp \
	sensor_descriptor.cpp \
	x3a_analyzer_rkiq.cpp \
	x3a_isp_config.cpp \
	x3a_statistics_queue.cpp \
	ae_state_machine.cpp \
	awb_state_machine.cpp \
	af_state_machine.cpp \
	rk_params_translate.cpp \
	Metadata2Str.cpp \
	rkaiq.cpp


LOCAL_SRC_FILES +=\
	iq/x3a_analyze_tuner.cpp \
	iq/x3a_ciq_bnr_ee_tuning_handler.cpp \
	iq/x3a_ciq_tnr_tuning_handler.cpp \
	iq/x3a_ciq_tuning_handler.cpp \
	iq/x3a_ciq_wavelet_tuning_handler.cpp

ifeq ($(IS_RKISP),true)
LOCAL_SRC_FILES += rkiq_params.cpp
endif

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds -Wno-error
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CPPFLAGS += -Wno-error -frtti -std=c++11
LOCAL_CPPFLAGS +=  -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../ext/rkisp \
	$(LOCAL_PATH)/../../xcore/ \
	$(LOCAL_PATH)/../../xcore/ia \
	$(LOCAL_PATH)/../../xcore/base \
	$(LOCAL_PATH)/../../plugins/3a/rkiq \
	$(LOCAL_PATH)/../../rkisp/isp-engine \
	$(LOCAL_PATH)/../../rkisp/ia-engine \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include/linux \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include/linux/media

ifeq ($(IS_ANDROID_OS),true)
LOCAL_C_INCLUDES += \
    system/media/camera/include \
    frameworks/av/include
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
else
LOCAL_SHARED_LIBRARIES += \
	libcamera_metadata \
	libcamera_client
endif
else
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../metadata/libcamera_client/include \
	$(LOCAL_PATH)/../../metadata/libcamera_metadata/include \
	$(LOCAL_PATH)/../../metadata/header_files/include/system/core/include

LOCAL_STATIC_LIBRARIES += \
	librkisp_metadata
endif

LOCAL_STATIC_LIBRARIES += \
	librkisp_ctrlloop \
	librkisp_isp_engine \
	libisp_ia_engine

LOCAL_MODULE:= librkisp_analyzer

include $(BUILD_STATIC_LIBRARY)
