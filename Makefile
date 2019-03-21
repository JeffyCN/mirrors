#CROSS_COMPILE ?= $(CURDIR)/../../../../toolschain/usr/bin/arm-linux-
#CROSS_COMPILE ?= /usr/bin/arm-linux-gnueabihf-
#CROSS_COMPILE ?=

include $(CURDIR)/productConfigs.mk
export CROSS_COMPILE
LOCAL_PATH:= $(CURDIR)
include $(LOCAL_PATH)/build_system/Makefile.rules
#include $(LOCAL_PATH)/productConfigs.mk

export  BUILD_OUTPUT_EXTERNAL_LIBS
export  IS_ANDROID_OS
export  IS_RKISP
export  IS_RK_ISP10
export  IS_RK_ISP11
export  IS_NEED_SHARED_PTR
export  IS_NEED_COMPILE_STLPORT
export  IS_NEED_LINK_STLPORT
export  IS_NEED_COMPILE_TINYXML2
export  IS_NEED_COMPILE_EXPAT
export  IS_SUPPORT_ION
export  IS_SUPPORT_DMABUF
export  IS_BUILD_GSTREAMER_PLUGIN
export  IS_BUILD_TEST_APP

#System level flags
export  PRJ_CPPFLAGS

export BUILD_EVERYTHING
export CLEAN_EVERYTHING

ifeq ($(IS_RKISP_v12),true)
	PRJ_CPPFLAGS += -DRKISP_v12=1
endif

ifeq ($(ARCH),arm)
define SET_EVERYTHING
	@cp -rf $(CURDIR)/ext/rkisp/usr/lib32/* $(BUILD_OUTPUT_EXTERNAL_LIBS)/
	@cp -rf $(CURDIR)/ext/rkisp/usr/include/glib-2.0-32/* $(BUILD_OUTPUT_GLIB_INC)/
endef
else
define SET_EVERYTHING
	@cp -rf $(CURDIR)/ext/rkisp/usr/lib64/* $(BUILD_OUTPUT_EXTERNAL_LIBS)/
	@cp -rf $(CURDIR)/ext/rkisp/usr/include/glib-2.0-64/* $(BUILD_OUTPUT_GLIB_INC)/
endef
endif

define BUILD_EVERYTHING
	@make -f Android.mk
endef

define CLEAN_EVERYTHING
	@echo "clean build output directory"
	@-rm -rf $(BUILD_DIR)
endef

.PHONY:all
all:
	@mkdir -p $(BUILD_OUTPUT_EXTERNAL_LIBS)
	@mkdir -p $(BUILD_OUTPUT_GLIB_INC)
	$(SET_EVERYTHING)
	$(BUILD_EVERYTHING)

clean:
	$(CLEAN_EVERYTHING)
