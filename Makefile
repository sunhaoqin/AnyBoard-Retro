#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := cndroid-firmware

TARGET_ADF_VER := v2.0

ifndef ADF_PATH
$(error Not Define esp-adf path!)
endif

ifneq ("$(shell test -d $(ADF_PATH) && echo ex)","ex")
$(error Not found esp-adf!)
endif

CURRENT_ADF_VER := $(shell cd ${ADF_PATH} && git describe --always --tags)
TMP := $(info "CURRENT_ADF_VER : $(CURRENT_ADF_VER)")
ifneq ($(CURRENT_ADF_VER), $(TARGET_ADF_VER))
TMP := $(info "Start to checkout esp-adf $(TARGET_ADF_VER)")
TMP := $(shell cd $(ADF_PATH) && git fetch origin tag $(TARGET_ADF_VER) && git checkout -f $(TARGET_ADF_VER) && git clean -ffd &&  git submodule update -f --init --recursive)
endif

include $(ADF_PATH)/project.mk

