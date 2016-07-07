LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    RockchipRgaTest_main.cpp \
    AudioPlayer.cpp \
    RockchipRgaTest.cpp

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include


LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libandroidfw \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libtinyalsa \
    libhardware

LOCAL_MODULE:= drmrgatest

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
