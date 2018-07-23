LOCAL_PATH:=$(shell pwd)

#find dependence of module libraries
DEP_FILE:=$(LOCAL_PATH)/.deps
ALLMAKEFILES:=$(wildcard $(LOCAL_PATH)/*/Android.mk) 
ifeq ($(DEP_FILE), $(wildcard $(DEP_FILE)))
include $(shell pwd)/.deps
ifneq ($(BUILD_DEPS),)
ALLMAKEFILES:=$(addprefix $(LOCAL_PATH)/,$(BUILD_DEPS))
ALLMAKEFILES:=$(addsuffix /Android.mk,$(ALLMAKEFILES))
endif
endif

all:
	$(call all-makefiles-under)

define all-makefiles-under
$(call build-subdir-makefile, $(ALLMAKEFILES))
endef


define build-subdir-makefile
	@for makefile in $1; \
	do \
		echo " "; \
		echo "[module] `dirname $$makefile | xargs basename`"; \
	   	( cd `dirname $$makefile` && $(MAKE) -f Android.mk  ) \
		|| exit 1; \
	done
endef
