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

EXECUTABLE_TARGET = $(addprefix $(BUILD_OUTPUT_BIN), $(LOCAL_MODULE))
EXECUTABLE_TARGET_C_OBJ = $(patsubst %.c, $(LOCAL_BUILD_DIR)/%.o, $(filter %.c, $(LOCAL_SRC_FILES)))
EXECUTABLE_TARGET_CPP_OBJ = $(patsubst %.cpp, $(LOCAL_BUILD_DIR)/%.o, $(filter %.cpp, $(LOCAL_SRC_FILES)))

EXECUTABLE_TARGET_STATIC_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_STATIC_LIBRARIES))
EXECUTABLE_TARGET_SHARED_LIBRARIES = $(patsubst lib%, -l%, $(LOCAL_SHARED_LIBRARIES))

$(EXECUTABLE_TARGET_C_OBJ): EXECUTABLE_TARGET_CFLAG = $(LOCAL_CFLAGS) $(SYSTEM_FLAGS)
$(EXECUTABLE_TARGET_CPP_OBJ): EXECUTABLE_TARGET_CPPFLAG = $(LOCAL_CPPFLAGS) $(SYSTEM_FLAGS)
$(EXECUTABLE_TARGET_C_OBJ) $(EXECUTABLE_TARGET_CPP_OBJ): EXECUTABLE_TARGET_FLAG += $(addprefix -I, $(LOCAL_C_INCLUDES))
$(EXECUTABLE_TARGET_C_OBJ) $(EXECUTABLE_TARGET_CPP_OBJ): EXECUTABLE_TARGET_FLAG += $(addprefix -I , $(SYSTEM_C_INCLUDES))
$(EXECUTABLE_TARGET_C_OBJ) $(EXECUTABLE_TARGET_CPP_OBJ): EXECUTABLE_TARGET_FLAG += $(addprefix -I, $(ROOT_DIR)/include)
$(EXECUTABLE_TARGET_C_OBJ) $(EXECUTABLE_TARGET_CPP_OBJ): EXECUTABLE_TARGET_FLAG += -L$(BUILD_OUTPUT_STATIC_LIBS)
$(EXECUTABLE_TARGET): EXECUTABLE_TARGET_LDFLAG += -L$(BUILD_OUTPUT_EXTERNAL_LIBS) -Wl,-rpath,$(BUILD_OUTPUT_EXTERNAL_LIBS) -L$(BUILD_OUTPUT_STATIC_LIBS) $(EXECUTABLE_TARGET_SHARED_LIBRARIES) $(EXECUTABLE_TARGET_STATIC_LIBRARIES) -L$(BUILD_OUTPUT_GSTREAMER_LIBS) -Wl,-rpath,$(BUILD_OUTPUT_GSTREAMER_LIBS) -L$(BUILD_OUTPUT_RKISP_LIBS) -lstdc++ -ldl -lm -lpthread -Xlinker --unresolved-symbols=ignore-in-shared-libs

all: $(EXECUTABLE_TARGET)
$(EXECUTABLE_TARGET):$(EXECUTABLE_TARGET_C_OBJ) $(EXECUTABLE_TARGET_CPP_OBJ)
	@mkdir -p $(dir $@)
	@$(TARGET_GPP) -o  $@ $^ $(EXECUTABLE_TARGET_LDFLAG)
	$(call quiet-cmd-echo-build, G++EXE, $@)
$(EXECUTABLE_TARGET_C_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.c
	@mkdir -p $(dir $@)
	@$(TARGET_GCC) $(EXECUTABLE_TARGET_CFLAG) $(EXECUTABLE_TARGET_FLAG) -c $< -o $@
	$(call quiet-cmd-echo-build, GCC, $@)
$(EXECUTABLE_TARGET_CPP_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.cpp
	@mkdir -p $(dir $@)
	@$(TARGET_GPP) $(EXECUTABLE_TARGET_CPPFLAG) $(EXECUTABLE_TARGET_FLAG) -c -fpic $< -o $@
	$(call quiet-cmd-echo-build, G++, $@)

#$(eval $(call make-target-static-library, "test"))
#$(call addsuffix, EXECUTABLE_LIB_SUFFIX, $(LOCAL_MODULE))
#$(addsuffix , EXECUTABLE_LIB_SUFFIX, $(LOCAL_MODULE))
#LOCAL_CFLAGS += $(addprefix -I , $(LOCAL_C_INCLUDES))

define quiet-cmd-echo-build
    @echo "  [$1]  $2"
endef

define build-static-library
	@echo "enter build-static-library"$1
	@echo $(LOCAL_CFLAGS)

$(eval $(call make-target-static-library,\
	$(addsuffix , EXECUTABLE_LIB_SUFFIX, $(LOCAL_MODULE)),\
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
