LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
				calibdb.cpp\
				xmltags.cpp\

LOCAL_C_INCLUDES := \
				bionic\
				external/stlport/stlport\
				$(LOCAL_PATH)/../include\
				$(LOCAL_PATH)/include\
				$(LOCAL_PATH)/./calib_xml\
				$(LOCAL_PATH)/../tinyxml2



LOCAL_CPPFLAGS := -fuse-cxa-atexit -Wall -Wextra -Werror -Wno-unused -Wformat-nonliteral -g -O0 -pedantic -Wno-error=unused-function -std=c++11

LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
LOCAL_CFLAGS += -DENABLE_ASSERT
LOCAL_CFLAGS += -Wno-error=unused-function -Wno-error=unused-parameter
LOCAL_CPPFLAGS += -Wno-error=unused-function -Wno-error=unused-parameter
#LOCAL_STATIC_LIBRARIES := libtinyxml2 libstlport_static libisp_cam_calibdb libisp_ebase libisp_oslayer


LOCAL_MODULE:= libisp_calibdb

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)

