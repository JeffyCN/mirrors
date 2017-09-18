LOCAL_PATH:= $(call my-dir)
#================================================================
ifneq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

$(info $(shell $(LOCAL_PATH)/version.sh))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

#LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += $(LOCAL_PATH)/normal

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga-normal\"

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    RockchipRga.cpp \
    GraphicBuffer.cpp \
    normal/NormalRga.cpp \
    normal/NormalRgaApi.cpp

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t720)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T72X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=0
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t760)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T76X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t860)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T86X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=1
endif
endif #android 6.0 and later

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_CFLAGS += -DRK3368
endif

LOCAL_MODULE:= librga
include $(BUILD_SHARED_LIBRARY)
endif
#############################################################################################
#############################################################################################
ifeq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

$(info $(shell $(LOCAL_PATH)/version.sh))

LOCAL_SRC_FILES += \
	RockchipRga.cpp \
	GraphicBuffer.cpp \
        normal/NormalRga.cpp \
        normal/NormalRgaApi.cpp

LOCAL_MODULE := librga

LOCAL_C_INCLUDES += external/libdrm/rockchip
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += $(LOCAL_PATH)/drm

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_SHARED_LIBRARIES += \
        libdrm_rockchip \
        liblog \
        libui \
        libcutils \
        libhardware

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga-normal\"

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_CFLAGS += -DRK3368_DRM=1
endif

LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)

include $(BUILD_SHARED_LIBRARY)
endif
#===================================================================
ifeq ($(strip $(BOARD_USE_DRM)), future)
#############################################################################################
ifeq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

$(info $(shell $(LOCAL_PATH)/version.sh))

LOCAL_SRC_FILES += \
        RockchipRga.cpp \
	GraphicBuffer.cpp \

LOCAL_MODULE := librga

LOCAL_C_INCLUDES += external/libdrm/rockchip
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += $(LOCAL_PATH)/drm

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_SHARED_LIBRARIES += \
        libdrm_rockchip \
        liblog \
        libui \
        libcutils \
        libhardware

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga-drm\"

LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)

include $(BUILD_SHARED_LIBRARY)
endif
#===================================================================
endif #it will be used when upstrean in the future
