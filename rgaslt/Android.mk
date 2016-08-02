LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    RockchipRgaSlt.cpp

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include


LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libtinyalsa \
    libhardware

#LOCAL_SHARED_LIBRARIES += \
    libandroidfw \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui

LOCAL_MODULE:= rgaslt

include $(BUILD_EXECUTABLE)
