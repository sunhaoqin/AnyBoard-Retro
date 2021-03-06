ifndef ADF_PATH
  $(error Not define esp-adf path!)
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