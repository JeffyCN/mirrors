#CROSS_COMPILE ?= $(CURDIR)/../../../../toolschain/usr/bin/arm-linux-
#CROSS_COMPILE ?= /usr/bin/arm-linux-gnueabihf-
#CROSS_COMPILE ?= 
include $(CURDIR)/productConfigs.mk
export CROSS_COMPILE
LOCAL_PATH:= $(CURDIR)
include $(LOCAL_PATH)/build_system/Makefile.rules
#include $(LOCAL_PATH)/productConfigs.mk

export  BUILD_OUPUT_EXTERNAL_LIBS
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

define BUILD_EVERYTHING
	@make -f Android.mk
endef

define CLEAN_EVERYTHING
	@echo "clean build objects"
	@-rm -f `find ./ -name *.o`
	@echo "clean build libraries"
	@-rm -f `find ./ -path "./libs" -prune -name *.a`
	@-rm -f `find ./build -name *.so`
	@echo "clean build output directory"
	@-rm -fr ./build
endef

.PHONY:all
all:
#cp ./libs/* ./build/lib/ 
	$(BUILD_EVERYTHING)

clean:
	$(CLEAN_EVERYTHING)
	
