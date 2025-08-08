LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := rkaiq_tool_server
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := rkaiq_tool_server_32
LOCAL_MODULE_STEM_64 := rkaiq_tool_server_64
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/camera \
    $(LOCAL_PATH)/common \
    $(LOCAL_PATH)/logger \
    $(LOCAL_PATH)/mediactl \
    $(LOCAL_PATH)/mediactl/linux \
    $(LOCAL_PATH)/netserver \
    $(LOCAL_PATH)/rkaiq \
    $(LOCAL_PATH)/rkaiq/rkaiq_api \
    $(LOCAL_PATH)/rtspserver \
    $(LOCAL_PATH)/rtspserver/include \
    $(LOCAL_PATH)/rtspserver/include/UsageEnvironment/include \
    $(LOCAL_PATH)/rtspserver/include/liveMedia/ \
    $(LOCAL_PATH)/rtspserver/include/liveMedia/include \
    $(LOCAL_PATH)/rtspserver/include/BasicUsageEnvironment/include \
    $(LOCAL_PATH)/rtspserver/include/groupsock/include \
    $(LOCAL_PATH)/../xcore \
    $(LOCAL_PATH)/../include/common \
    $(LOCAL_PATH)/../xcore/base \
    $(LOCAL_PATH)/../aiq_core \
    $(LOCAL_PATH)/../algos \
    $(LOCAL_PATH)/../algos_camgroup \
    $(LOCAL_PATH)/../hwi \
    $(LOCAL_PATH)/../iq_parser \
    $(LOCAL_PATH)/../iq_parser_v2 \
    $(LOCAL_PATH)/../uAPI \
    $(LOCAL_PATH)/../uAPI/include \
    $(LOCAL_PATH)/../uAPI2 \
    $(LOCAL_PATH)/../common \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../include/iq_parser \
    $(LOCAL_PATH)/../include/iq_parser_v2 \
    $(LOCAL_PATH)/../include/iq_parser_v2/j2s \
    $(LOCAL_PATH)/../include/xcore \
    $(LOCAL_PATH)/../include/common/mediactl \
    $(LOCAL_PATH)/../include/xcore/base \
    $(LOCAL_PATH)/../include/algos \
    $(LOCAL_PATH)/../include/ipc_server \
    $(LOCAL_PATH)/../ipc_server

LOCAL_SRC_FILES := \
    rkaiq_tool_server.cpp \
    rtspserver/RKHWEncApi.cpp \
    rtspserver/CamCaptureHelper.cpp \
    rtspserver/H264LiveVideoServerMediaSubsession.cpp \
    rtspserver/H264LiveVideoSource.cpp \
    rtspserver/QMediaBuffer.cpp \
    rtspserver/RtspServer.cpp \
    camera/camera_capture.cpp \
    camera/camera_device.cpp \
    camera/camera_infohw.cpp \
    camera/camera_memory.cpp \
    logger/log.cpp \
    netserver/tcp_client.cpp \
    netserver/domain_tcp_client.cpp \
    rkaiq/rkaiq_api/rkaiq_socket.cpp \
    rkaiq/multiframe_process.cpp \
    rkaiq/rkaiq_protocol.cpp \
    rkaiq/rkaiq_online_protocol.cpp \
    rkaiq/rkaiq_raw_protocol.cpp \
    rkaiq/rkaiq_api/rkaiq_media.cpp \
    mediactl/libmediactl.c \
    mediactl/libv4l2subdev.c \
    mediactl/media_info.c \
    mediactl/options.c

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    liblog \
    libvpu \
    libssl \
    libcrypto

LOCAL_STATIC_LIBRARIES := \
    libliveMedia \
    libgroupsock \
    libBasicUsageEnvironment \
    libUsageEnvironment \
    librtspserver

LOCAL_CFLAGS := \
    -Wall \
    -Wno-error \
    -Wextra \
    -Wno-missing-field-initializers \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-non-pod-varargs \
    -DANDROID

include $(BUILD_EXECUTABLE)