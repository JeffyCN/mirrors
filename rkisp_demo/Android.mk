LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
SRC_CPP := $(wildcard $(LOCAL_PATH)/demo/*.c)
SRC_C := $(wildcard $(LOCAL_PATH)/demo/*.c)
SRC_C += $(wildcard $(LOCAL_PATH)/demo/drmDsp/*.c)

ifneq ($(filter rk1126 rk356x, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_CPPFLAGS += -DISP_HW_V21
endif
ifneq ($(filter rk3588, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_CPPFLAGS += -DISP_HW_V30
endif
ifneq ($(filter rk3576, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_CPPFLAGS += -DISP_HW_V39
endif

ifneq ($(filter rk3576 rv1103b, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_CPPFLAGS += -DUSE_NEWSTRUCT=1
endif

LOCAL_SRC_FILES :=\
	demo/drmDsp.c \
	demo/drmDsp/bo.c \
	demo/drmDsp/dev.c \
	demo/drmDsp/modeset.c \
	demo/rkisp_demo.c \
    demo/ae_algo_demo/third_party_ae_algo.c \
    demo/af_algo_demo/third_party_af_algo.c \
    demo/sample/sample_a3dlut_module.c \
    demo/sample/sample_abayer2dnr_module.c \
    demo/sample/sample_abayertnr_module.c \
    demo/sample/sample_ablc_module.c \
    demo/sample/sample_accm_module.c \
    demo/sample/sample_acnr_module.c \
    demo/sample/sample_acp_module.c \
    demo/sample/sample_adebayer_module.c \
    demo/sample/sample_adehaze_module.c \
    demo/sample/sample_adpcc_module.c \
    demo/sample/sample_adrc_module.c \
    demo/sample/sample_ae_module.c \
    demo/sample/sample_af_module.c \
    demo/sample/sample_again_module.c \
    demo/sample/sample_agamma_module.c \
    demo/sample/sample_agic_module.c \
    demo/sample/sample_aie_module.c \
    demo/sample/sample_aldch_module.c \
    demo/sample/sample_aldch_v21_module.c \
    demo/sample/sample_alsc_module.c \
    demo/sample/sample_amerge_module.c \
    demo/sample/sample_asharp_module.c \
    demo/sample/sample_awb_module.c \
    demo/sample/sample_aynr_module.c \
    demo/sample/sample_cac_module.c \
    demo/sample/sample_cgc_module.c \
    demo/sample/sample_comm.c \
    demo/sample/sample_csm_module.c \
    demo/sample/sample_image_process.c \
    demo/sample/sample_misc_modules.c \
    demo/sample/sample_smartIr.c \

#TODO: have compile issue on Android now
	#demo/rga_control.c \
	#demo/rkdrm_display.c \
	#demo/display.c \
	#demo/rkRgaApi.cpp

ifneq ($(filter rk356x rk3588, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_SRC_FILES += demo/awb_algo_demo/third_party_awb_algo.c
endif
ifneq ($(filter rv1106, $(strip $(TARGET_BOARD_PLATFORM))), )
LOCAL_SRC_FILES += demo/awb_algo_demo/third_party_awbV32_algo.c
endif

LOCAL_CPPFLAGS += -std=c++11 -Wno-error -DAndroid
#LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CFLAGS += -Wno-error -Wno-return-type
#LOCAL_CPPFLAGS += -DLINUX
#LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CFLAGS += -DANDROID_OS
LOCAL_CFLAGS += -DSAMPLE_SMART_IR
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/demo/ \
	$(LOCAL_PATH)/demo/include \
	$(LOCAL_PATH)/demo/include/rga \
	$(LOCAL_PATH)/demo/include/libdrm \
	$(LOCAL_PATH)/demo/drmDsp \
	$(LOCAL_PATH)/demo/sample \
	$(LOCAL_PATH)/../../include/uAPI \
	$(LOCAL_PATH)/../../include/uAPI2 \
	$(LOCAL_PATH)/../../include/xcore \
	$(LOCAL_PATH)/../../include/algos \
	$(LOCAL_PATH)/../../include/common \
	$(LOCAL_PATH)/../../include/iq_parser \
	$(LOCAL_PATH)/../../include/isp \
	$(LOCAL_PATH)/../../smart_ir/include \
	$(LOCAL_PATH)/deps \
	$(LOCAL_PATH)/deps/include \
	$(LOCAL_PATH)/deps/include/rga \
	$(LOCAL_PATH)/deps/include/libdrm \
	$(LOCAL_PATH)/deps/include/libkms \
LOCAL_C_INCLUDES += \
	system/media/camera/include \
	frameworks/av/include \
	external/libdrm/include/drm \
	external/libdrm \
	system/core/libutils/include \
	system/core/include \
	frameworks/native/libs/binder/include \
   frameworks/native/libs/ui/include/ui
ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
LOCAL_HEADER_LIBRARIES += \
	libhardware_headers \
	libbinder_headers \
	gl_headers \
	libutils_headers
else
LOCAL_C_INCLUDES += \
	hardware/libhardware/include \
	hardware/libhardware/modules/gralloc \
	system/core/include \
	system/core/include/utils \
	frameworks/av/include \
	hardware/libhardware/include
endif
LOCAL_C_INCLUDES += \
	hardware/rockchip/librga
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X
LOCAL_SHARED_LIBRARIES += librkaiq \
						  libsmartIr \
						  libdrm \
					      librga
LOCAL_CPPFLAGS += \
	-DUSING_METADATA_NAMESPACE=using\ ::android::hardware::camera::common::V1_0::helper::CameraMetadata
ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
endif
LOCAL_32_BIT_ONLY := true
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES += libutils libcutils liblog
LOCAL_MODULE:= rkaiq_demo
include $(BUILD_EXECUTABLE)
