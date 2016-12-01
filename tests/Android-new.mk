LOCAL_PATH:= $(call my-dir)

#======================================================================
#
#rgacopy
#
#======================================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include

LOCAL_C_INCLUDES += hardware/rockchip/librga
LOCAL_C_INCLUDES += hardware/rk29/librga

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware \
    librga

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    RockchipRgaCopy.cpp

LOCAL_MODULE:= rgacopy

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

#======================================================================
#
#rgacolorfill
#
#======================================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include

LOCAL_C_INCLUDES += hardware/rockchip/librga
LOCAL_C_INCLUDES += hardware/rk29/librga

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware \
    librga

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    RockchipRgaColorFill.cpp

LOCAL_MODULE:= rgacolorfill

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)


#======================================================================
#
#rgayuvtentoyuv
#
#======================================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include

LOCAL_C_INCLUDES += hardware/rockchip/librga
LOCAL_C_INCLUDES += hardware/rk29/librga

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware \
    librga

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    RockchipRgaCovertYuvTenBitToNormalYuv.cpp

LOCAL_MODULE:= rgayuvtentoyuv

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)


#======================================================================
#
#rgablit
#
#======================================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include

LOCAL_C_INCLUDES += hardware/rockchip/librga
LOCAL_C_INCLUDES += hardware/rk29/librga

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libskia \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware \
    librga

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    RockchipRgaBlit.cpp

LOCAL_MODULE:= rgablit

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif
include $(BUILD_EXECUTABLE)

