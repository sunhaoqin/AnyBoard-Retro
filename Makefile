#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
PROJECT_NAME := AnyBoard

TARGET_ADF_VER := v2.0-78-gcf28897

TARGET_IDF_VER := v4.0.1

ADF_PATCH_DIR := $(realpath ./esp_adf_patch)

IDF_PATCH_DIR := $(realpath ./esp_idf_patch)

include $(realpath ./make/check_adf.mk)	

include $(realpath ./make/apply_patch.mk)

include $(ADF_PATH)/project.mk