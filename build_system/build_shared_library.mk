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

SYSTEM_C_INCLUDES += $(BUILD_OUTPUT_RKISP_INC)
SYSTEM_C_INCLUDES += $(BUILD_OUTPUT_GLIB_INC)
SYSTEM_C_INCLUDES += $(BUILD_OUTPUT_GLIB_INC)/include
SYSTEM_C_INCLUDES += $(BUILD_OUTPUT_GSTREAMER_INC)

SYSTEM_FLAGS += -DHAVE_CONFIG_H -DHAVE_RK_IQ=1 -DHAVE_LIBDRM=1 -DHAVE_RK_IQ=1

LOCAL_BUILD_DIR = $(BUILD_DIR)/$(LOCAL_MODULE)

SHARED_TARGET = $(addsuffix $(SHARED_LIB_SUFFIX), $(addprefix $(BUILD_OUTPUT_STATIC_LIBS), $(LOCAL_MODULE)))
SHARED_TARGET_C_OBJ = $(patsubst %.c, $(LOCAL_BUILD_DIR)/%.o, $(filter %.c, $(LOCAL_SRC_FILES)))
SHARED_TARGET_CPP_OBJ = $(patsubst %.cpp, $(LOCAL_BUILD_DIR)/%.o, $(filter %.cpp, $(LOCAL_SRC_FILES)))
SHARED_TARGET_STATIC_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_STATIC_LIBRARIES))
SHARED_TARGET_SHARED_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_SHARED_LIBRARIES))

$(SHARED_TARGET_C_OBJ): SHARED_TARGET_CFLAG = $(LOCAL_CFLAGS) $(SYSTEM_FLAGS)
$(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_CPPFLAG := $(LOCAL_CPPFLAGS) $(SYSTEM_FLAGS)
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(LOCAL_C_INCLUDES))
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(SYSTEM_C_INCLUDES))
$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ): SHARED_TARGET_FLAG += $(addprefix -I , $(ROOT_DIR)/include)
$(SHARED_TARGET): SHARED_TARGET_LDFLAG += -L$(BUILD_OUTPUT_EXTERNAL_LIBS) -Wl,-rpath,$(BUILD_OUTPUT_EXTERNAL_LIBS) -L$(BUILD_OUTPUT_STATIC_LIBS) -Wl,--whole-archive $(SHARED_TARGET_STATIC_LIBRARIES) -Wl,-no-whole-archive $(SHARED_TARGET_SHARED_LIBRARIES) -lstdc++ -ldl -lm -lpthread -L$(BUILD_OUTPUT_GSTREAMER_LIBS) -L$(BUILD_OUTPUT_RKISP_LIBS) -Xlinker --unresolved-symbols=ignore-in-shared-libs

all: $(SHARED_TARGET)
$(SHARED_TARGET):$(SHARED_TARGET_C_OBJ) $(SHARED_TARGET_CPP_OBJ)
	@mkdir -p $(dir $@)
	$(TARGET_GCC) -shared -fpic -o $@ $^ $(SHARED_TARGET_LDFLAG) 
	$(call quiet-cmd-echo-build, GCC-SHARED, $@)
$(SHARED_TARGET_C_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.c
	@mkdir -p $(dir $@)
	@$(TARGET_GCC) $(SHARED_TARGET_CFLAG) $(SHARED_TARGET_FLAG) -c -fpic $< -o $@
	$(call quiet-cmd-echo-build, GCC, $@)
$(SHARED_TARGET_CPP_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.cpp
	@mkdir -p $(dir $@)
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
	$(patsubst %.c, $(LOCAL_BUILD_DIR)/%.o, $(LOCAL_SRC_FILES)),\
	$(LOCAL_CFLAGS)))
endef

define make-target-static-library
$1:$2
	@mkdir -p $(dir $@)
	ar r $1 $2
$2:$(LOCAL_BUILD_DIR)/%.o:%.c
	@mkdir -p $(dir $@)
	gcc $3 -c $$< -o $$@
endef

define test-cmd
target:test
	@echo "++++++++++++++++++++++++++++++++++test running..."
endef

define test-cmd1
	"----------------------------------test running..."
endef
