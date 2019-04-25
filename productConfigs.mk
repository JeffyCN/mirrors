ifeq ($(PLATFORM_SDK_VERSION),)
IS_ANDROID_OS = false
IS_RKISP = true
IS_RK_ISP10 = false
IS_RK_ISP11 = false
IS_NEED_SHARED_PTR = false
IS_NEED_COMPILE_STLPORT = false
IS_NEED_LINK_STLPORT = false
IS_NEED_COMPILE_TINYXML2 = true
IS_NEED_COMPILE_EXPAT = true
IS_SUPPORT_ION = false
IS_SUPPORT_DMABUF = true
IS_BUILD_GSTREAMER_PLUGIN = true
IS_BUILD_TEST_APP = false
ifeq ($(ARCH),arm)
CROSS_COMPILE ?= /home/zyc/toolchains/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
else
CROSS_COMPILE ?= $(shell pwd)/../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif
else
IS_ANDROID_OS = true
IS_RKISP = true
IS_RKISP_v12 = false
IS_RK_ISP10 = false
IS_RK_ISP11 = false
IS_NEED_SHARED_PTR = false
IS_NEED_COMPILE_STLPORT = false
IS_NEED_LINK_STLPORT = false
IS_NEED_COMPILE_TINYXML2 = false
IS_NEED_COMPILE_EXPAT = false
IS_SUPPORT_ION = false
IS_SUPPORT_DMABUF = true
IS_BUILD_GSTREAMER_PLUGIN = false
IS_BUILD_TEST_APP = false
#CROSS_COMPILE ?= /home/jacobchen/mksdk/compiler/gcc-linaro-5.5.0-2017.10-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif

ifeq ($(IS_ANDROID_OS),true)
PRJ_CPPFLAGS := -DANDROID_OS
PRJ_CPPFLAGS += -Wno-error=unused-variable
PRJ_CPPFLAGS += -Wno-error=unused-parameter
PRJ_CPPFLAGS += -Wno-error=ignored-qualifiers
PRJ_CPPFLAGS += -Wno-error=address-of-packed-member
PRJ_CPPFLAGS += -Wno-error=overloaded-virtual
PRJ_CPPFLAGS += -Wno-error=unused-private-field
PRJ_CPPFLAGS += -Wno-error=zero-length-array
PRJ_CPPFLAGS += -Wno-error=gnu-include-next
PRJ_CPPFLAGS += -Wno-error=c11-extensions
PRJ_CPPFLAGS += -Wno-error=macro-redefined
PRJ_CPPFLAGS += -Wno-error=gnu-zero-variadic-macro-arguments
PRJ_CPPFLAGS += -Wno-error=unused-function
endif

ifeq ($(IS_RKISP),true)
PRJ_CPPFLAGS += -DRKISP=1
endif

ifeq ($(IS_RKISP_v12),true)
PRJ_CPPFLAGS += -DRKISP_v12=1
endif

ifeq ($(IS_RK_ISP10),true)
PRJ_CPPFLAGS := -DRK_ISP10=1
endif

ifeq ($(IS_RK_ISP11),true)
PRJ_CPPFLAGS := -DRK_ISP11=1
endif
