# build log
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	xcam_common.cpp \

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERTa
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CPPFLAGS += -std=c++11 -frtti
LOCAL_CPPFLAGS +=  -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/base \

LOCAL_MODULE:= libisp_log
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include
endif

include $(BUILD_STATIC_LIBRARY)

# build xcore
include $(CLEAR_VARS)

DRM_SRC_FILES += \
	intel_ia_ctrl.cpp \
	dma_video_buffer.cpp \
	drm_bo_buffer.cpp \
	drm_display.cpp \
	drm_v4l2_buffer.cpp

LOCAL_SRC_FILES +=\
	$(DRM_SRC_FILES)

LOCAL_SRC_FILES +=\
	analyzer_loader.cpp \
	buffer_pool.cpp \
	calibration_parser.cpp \
	device_manager.cpp \
	dynamic_analyzer.cpp \
	dynamic_analyzer_loader.cpp \
	fake_poll_thread.cpp \
	file_handle.cpp \
	handler_interface.cpp \
	image_file_handle.cpp \
	image_handler.cpp \
	image_processor.cpp \
	image_projector.cpp \
	once_map_video_buffer_priv.cpp \
	pipe_manager.cpp \
	poll_thread.cpp \
	smart_analysis_handler.cpp \
	smart_analyzer.cpp \
	smart_analyzer_loader.cpp \
	smart_buffer_priv.cpp \
	surview_fisheye_dewarp.cpp \
	swapped_buffer.cpp \
	thread_pool.cpp \
	uvc_device.cpp \
	v4l2_buffer_proxy.cpp \
	v4l2_device.cpp \
	video_buffer.cpp \
	worker.cpp \
	x3a_analyzer.cpp \
	x3a_analyzer_manager.cpp \
	x3a_analyzer_simple.cpp \
	x3a_image_process_center.cpp \
	x3a_result.cpp \
	x3a_result_factory.cpp \
	x3a_stats_pool.cpp \
	xcam_analyzer.cpp \
	xcam_buffer.cpp \
	xcam_thread.cpp \
	xcam_utils.cpp \
	dynamic_algorithms_libs_loader.cpp

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERTa
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CPPFLAGS += -std=c++11 -frtti
LOCAL_CPPFLAGS +=  -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/base \
	$(LOCAL_PATH)/ia \
	$(LOCAL_PATH)/../ext/rkisp \
	$(LOCAL_PATH)/../plugins/3a/rkiq \
    $(LOCAL_PATH)/../rkisp/isp-engine \
    $(LOCAL_PATH)/../rkisp/ia-engine \
    $(LOCAL_PATH)/../rkisp/ia-engine/include \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux/media

ifeq ($(IS_ANDROID_OS),true)
LOCAL_C_INCLUDES += \
	external/libdrm/include/drm \
	external/libdrm
endif

LOCAL_MODULE:= librkisp_ctrlloop
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include
endif

include $(BUILD_STATIC_LIBRARY)
