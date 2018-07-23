COMMAND = $(eval $(call test-cmd1))

SYSTEM_C_INCLUDES += $(ROOT_DIR)
SYSTEM_C_INCLUDES += $(ROOT_DIR)/xcore
SYSTEM_C_INCLUDES += $(ROOT_DIR)/modules
SYSTEM_C_INCLUDES += $(ROOT_DIR)/modules/rkisp
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp/usr/include
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp/usr/include/drm
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp/usr/include/gstreamer-1.0
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp/usr/include/glib-2.0
SYSTEM_C_INCLUDES += $(ROOT_DIR)/ext/rkisp/usr/include/glib-2.0/include

SYSTEM_FLAGS += -DHAVE_CONFIG_H -DHAVE_RK_IQ=1 -DHAVE_LIBDRM=1 -DHAVE_RK_IQ=1

SHARED_TARGET = $(addsuffix $(SHARED_LIB_SUFFIX), $(addprefix $(BUILD_OUTPUT_STATIC_LIBS), $(LOCAL_MODULE)))
SHARED_TARGET_C_OBJ = $(patsubst %.c, %.o, $(filter %.c, $(LOCAL_SRC_FILES)))
SHARED_TARGET_CPP_OBJ = $(patsubst %.cpp, %.o, $(filter %.cpp, $(LOCAL_SRC_FILES)))
SHARED_TARGET_STATIC_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_STATIC_LIBRARIES))
SHARED_TARGET_SHARED_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_SHARED_LIBRARIES))

$(SHARED_TARGET_C_OBJ): SHARED_TARGET_CFLAG = $(LOCAL_CFLAGS) $(SYSTEM_FLAGS)
$(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_CPPFLAG := $(LOCAL_CPPFLAGS) $(SYSTEM_FLAGS)
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(LOCAL_C_INCLUDES))
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(SYSTEM_C_INCLUDES))
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(ROOT_DIR)/include)
$(SHARED_TARGET): SHARED_TARGET_LDFLAG += -L$(BUILD_OUPUT_EXTERNAL_LIBS) -Wl,-rpath,$(BUILD_OUPUT_EXTERNAL_LIBS) -L$(BUILD_OUTPUT_STATIC_LIBS) -Wl,--whole-archive $(SHARED_TARGET_STATIC_LIBRARIES) -Wl,-no-whole-archive $(SHARED_TARGET_SHARED_LIBRARIES) -lstdc++ -ldl -lm -lpthread -L$(BUILD_OUPUT_GSTREAMER_LIBS) -L$(BUILD_OUPUT_RKISP_LIBS) -Xlinker --unresolved-symbols=ignore-in-shared-libs

all: $(SHARED_TARGET)
$(SHARED_TARGET):$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ)
	$(TARGET_GCC) -shared -fpic -o $@ $^ $(SHARED_TARGET_LDFLAG) 
	$(call quiet-cmd-echo-build, GCC-SHARED, $@)
$(SHARED_TARGET_C_OBJ):%.o:%.c
	@$(TARGET_GCC) $(SHARED_TARGET_CFLAG) $(SHARED_TARGET_FLAG) -c -fpic $< -o $@
	$(call quiet-cmd-echo-build, GCC, $@)
$(SHARED_TARGET_CPP_OBJ):%.o:%.cpp
	@$(TARGET_GPP) $(SHARED_TARGET_CPPFLAG) $(SHARED_TARGET_FLAG) -c -fpic $< -o $@
	$(call quiet-cmd-echo-build, G++, $@)

#$(eval $(call make-target-static-library, "test"))
#$(call addsuffix, STATIC_LIB_SUFFIX, $(LOCAL_MODULE))
#$(addsuffix , STATIC_LIB_SUFFIX, $(LOCAL_MODULE))
#LOCAL_CFLAGS += $(addprefix -I , $(LOCAL_C_INCLUDES))

define quiet-cmd-echo-build
    @echo "  [$1]  $2"
endef

define build-static-library
	@echo "enter build-static-library"$1
	@echo $(LOCAL_CFLAGS)

$(eval $(call make-target-static-library,\
	$(addsuffix , STATIC_LIB_SUFFIX, $(LOCAL_MODULE)),\
	$(patsubst %.c, %.o, $(LOCAL_SRC_FILES)),\
	$(LOCAL_CFLAGS)))
endef

define make-target-static-library
$1:$2
	ar r $1 $2
$2:%.o:%.c
	gcc $3 -c $$< -o $$@
endef

define test-cmd
target:test
	@echo "++++++++++++++++++++++++++++++++++test running..."
endef

define test-cmd1
	"----------------------------------test running..."
endef
