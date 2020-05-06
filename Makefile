#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
PROJECT_NAME := AnyBoard

UNAME := $(shell uname)

TARGET_ADF_VER := v2.0

TARGET_IDF_VER := v3.3.1-211-g6f9a7264c

ADF_PATCHE_DIR := $(realpath ./esp_adf_patch)
IDF_PATCHE_DIR := $(realpath ./esp_idf_patch)

ifndef ADF_PATH
  $(error Not Define esp-adf path!)
endif

ifneq ("$(shell test -d $(ADF_PATH) && echo ex)","ex")
  $(error Not found esp-adf!)
endif

IDF_PATH := $(ADF_PATH)/esp-idf

CURRENT_ADF_VER := $(shell cd ${ADF_PATH} && git describe --always --tags)
TMP := $(info "CURRENT_ADF_VER : $(CURRENT_ADF_VER)")

ifneq ($(CURRENT_ADF_VER), $(TARGET_ADF_VER))
	$(info "Start to checkout esp-adf $(TARGET_ADF_VER)")
	$(shell cd $(ADF_PATH) && git fetch origin tag $(TARGET_ADF_VER) && git checkout -f $(TARGET_ADF_VER) && git clean -ffd &&  git submodule update -f --init --recursive)
endif

# Check ADF patches
ifneq ("$(shell test -e $(ADF_PATH)/patch_flag && echo ex)","ex")
  APPLY_PATCHES := 1
else
  # Get previous hash of applyied patches
  PREV_HASH := $(shell cat $(ADF_PATH)/patch_flag)

  # Get current hash
  ifeq ("$(UNAME)", "Linux")
    CURR_HASH := $(shell cat $(ADF_PATCHE_DIR)/*.patch | sha256sum)
  else
    CURR_HASH := $(shell cat $(ADF_PATCHE_DIR)/*.patch | shasum -a 256 -p)
  endif

  ifneq ("$(PREV_HASH)","$(CURR_HASH)")
    APPLY_PATCHES := 1
  else
    APPLY_PATCHES := 0
  endif
endif
	
# Apply ADF patches
ifeq ("$(APPLY_PATCHES)","1")
  $(info Reverting previous esp-adf patches ...)
  $(shell cd $(ADF_PATH) && git checkout -f $(TARGET_ADF_VER))
  $(shell cd $(ADF_PATH) && git clean -ffd)
  TMP := $(shell cd $(ADF_PATH) && git submodule update -f --init --recursive)
  $(info Applying esp-adf patches ...)
  $(foreach PATCH,$(abspath $(wildcard $(ADF_PATCHE_DIR)/*.patch)), \
    $(info Applying patch $(PATCH)...); 							\
    $(shell cd $(ADF_PATH) && git apply --whitespace=warn $(PATCH)) \
  )

  $(info Patches applied)

  #Compute and save new hash
  ifeq ("$(UNAME)", "Linux")
    $(shell cat $(ADF_PATCHE_DIR)/*.patch | sha256sum > $(ADF_PATH)/patch_flag)
  else
    $(shell cat $(ADF_PATCHE_DIR)/*.patch | shasum -a 256 -p > $(ADF_PATH)/patch_flag)
  endif
endif

# Check IDF patches
ifneq ("$(shell test -e $(IDF_PATH)/patch_flag && echo ex)","ex")
  APPLY_PATCHES := 1
else
  # Get previous hash of applyied patches
  PREV_HASH := $(shell cat $(IDF_PATH)/patch_flag)

  # Get current hash
  ifeq ("$(UNAME)", "Linux")
    CURR_HASH := $(shell cat $(IDF_PATCHE_DIR)/*.patch | sha256sum)
  else
    CURR_HASH := $(shell cat $(IDF_PATCHE_DIR)/*.patch | shasum -a 256 -p)
  endif

  ifneq ("$(PREV_HASH)","$(CURR_HASH)")
    APPLY_PATCHES := 1
  else
    APPLY_PATCHES := 0
  endif
 endif

# endif # Check IDF patches

# Apply IDF patches
ifeq ("$(APPLY_PATCHES)","1")
  $(info Reverting previous esp-idf patches ...)
  $(shell cd $(IDF_PATH) && git checkout -f $(TARGET_IDF_VER))
  $(shell cd $(IDF_PATH) && git clean -ffd)
  TMP := $(shell cd $(IDF_PATH) && git submodule update -f --init --recursive)
  $(info Applying esp-idf patches ...)
  $(foreach PATCH,$(abspath $(wildcard $(IDF_PATCHE_DIR)/*.patch)), \
    $(info Applying patch $(PATCH)...); \
    $(shell cd $(IDF_PATH) && git apply --whitespace=warn $(PATCH)) \
  )

  $(info Patches applied)

  # Compute and save new hash
  ifeq ("$(UNAME)", "Linux")
    $(shell cat $(IDF_PATCHE_DIR)/*.patch | sha256sum > $(IDF_PATH)/patch_flag)
  else
    $(shell cat $(IDF_PATCHE_DIR)/*.patch | shasum -a 256 -p > $(IDF_PATH)/patch_flag)
  endif
endif # Apply IDF patches

include $(ADF_PATH)/project.mk