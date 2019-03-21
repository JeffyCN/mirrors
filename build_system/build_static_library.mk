COMMAND = $(eval $(call test-cmd1))
XCAM_VERSION = 1.0

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

WORK_DIR=$(addprefix $(BUILD_OUTPUT_STATIC_LIBS), $(LOCAL_MODULE))
STATIC_TARGET = $(addsuffix $(STATIC_LIB_SUFFIX), $(addprefix $(BUILD_OUTPUT_STATIC_LIBS), $(LOCAL_MODULE)))
STATIC_TARGET_C_OBJ = $(patsubst %.c, $(LOCAL_BUILD_DIR)/%.o, $(filter %.c, $(LOCAL_SRC_FILES)))
STATIC_TARGET_CPP_OBJ = $(patsubst %.cpp, $(LOCAL_BUILD_DIR)/%.o, $(filter %.cpp, $(LOCAL_SRC_FILES)))
STATIC_TARGET_STATIC_LIBRARIES = $(addprefix $(BUILD_OUTPUT_STATIC_LIBS), $(addsuffix $(STATIC_LIB_SUFFIX), $(LOCAL_STATIC_LIBRARIES)))

$(STATIC_TARGET_C_OBJ): STATIC_TARGET_CFLAG = $(LOCAL_CFLAGS) -std=c99 $(SYSTEM_FLAGS)
$(STATIC_TARGET_CPP_OBJ): STATIC_TARGET_CPPFLAG = $(LOCAL_CPPFLAGS) $(SYSTEM_FLAGS)
$(STATIC_TARGET_C_OBJ) $(STATIC_TARGET_CPP_OBJ): STATIC_TARGET_FLAG += $(addprefix -I, $(LOCAL_C_INCLUDES))
$(STATIC_TARGET_C_OBJ) $(STATIC_TARGET_CPP_OBJ): STATIC_TARGET_FLAG += $(addprefix -I, $(SYSTEM_C_INCLUDES))
$(STATIC_TARGET_C_OBJ) $(STATIC_TARGET_CPP_OBJ): STATIC_TARGET_FLAG += $(addprefix -I, $(ROOT_DIR)/include)
$(STATIC_TARGET_C_OBJ) $(STATIC_TARGET_CPP_OBJ): STATIC_TARGET_FLAG += -L$(BUILD_OUTPUT_STATIC_LIBS)

all: $(STATIC_TARGET)
$(STATIC_TARGET):$(STATIC_TARGET_C_OBJ) $(STATIC_TARGET_CPP_OBJ)
	@mkdir -p $(dir $@)
	@$(TARGET_AR) rcs $@ $^
	$(call quiet-cmd-echo-build, AR, $@)
$(STATIC_TARGET_C_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.c
	@mkdir -p $(dir $@)
	@$(TARGET_GCC) $(STATIC_TARGET_CFLAG) $(STATIC_TARGET_FLAG) -c -fpic $< -o $@
	$(call quiet-cmd-echo-build, GCC, $@)
$(STATIC_TARGET_CPP_OBJ):$(LOCAL_BUILD_DIR)/%.o:%.cpp
	@mkdir -p $(dir $@)
	@$(TARGET_GPP) $(STATIC_TARGET_CPPFLAG) $(STATIC_TARGET_FLAG) -c -fpic $< -o $@
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

	@-mkdir $(WORK_DIR)
	@for i in $(LOCAL_STATIC_LIBRARIES); \
	do \
		cd $(WORK_DIR); \
		$(TARGET_AR) x $(BUILD_OUTPUT_STATIC_LIBS)/$$i$(STATIC_LIB_SUFFIX); \
	done
	@$(TARGET_AR) rcs $@ $^ `ls $(WORK_DIR)/*.o`
	@rm -fr $(WORK_DIR)


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
