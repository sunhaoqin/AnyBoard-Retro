cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED ENV{ADF_PATH})
  message(FATAL_ERROR "Not defined ADF_PATH")
endif(NOT DEFINED ENV{ADF_PATH})

if(NOT DEFINED ENV{IDF_PATH})
  message(FATAL_ERROR "Not defined IDF_PATH")
endif(NOT DEFINED ENV{IDF_PATH})

function(check_repo_version REPO_DIR TARGET_VERSION)
  execute_process(COMMAND git describe --always --tags
                  WORKING_DIRECTORY ${REPO_DIR}
                  OUTPUT_VARIABLE CURRENT_VER
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT ${CURRENT_VER} STREQUAL ${TARGET_VERSION})
    execute_process(COMMAND git fetch origin
          COMMAND git clean -ffd
          COMMAND git checkout -f ${TARGET_VERSION}
          COMMAND git clean -ffd
          COMMAND git submodule update -f --init
          WORKING_DIRECTORY ${REPO_DIR})
  endif(NOT ${CURRENT_VER} STREQUAL ${TARGET_VERSION})
endfunction(check_repo_version REPO_DIR TARGET_VERSION)

function(check_patch REPO_DIR TARGET_VERSION PATCH_DIR)
  file(TO_CMAKE_PATH ${PATCH_DIR}/*.patch PATCH_GLOB_EXPR)
  file(GLOB ALL_PATCH_FIEL LIST_DIRECTORIES false ${PATCH_GLOB_EXPR})
  list(LENGTH ALL_PATCH_FIEL PATCH_NUM)
  if(${PATCH_NUM} GREATER 0)

    # Get current hashs
    file(TO_NATIVE_PATH ${REPO_DIR}/temp_content TEMP_CONTENT_FILE)
    foreach(PATCH_FILE IN LISTS ALL_PATCH_FIEL)
      file(READ ${PATCH_FILE} TEMP_CONTENT)
      file(APPEND ${TEMP_CONTENT_FILE} ${TEMP_CONTENT})
    endforeach(PATCH_FILE)

    file(SHA256 ${TEMP_CONTENT_FILE} CURR_HASH)
    file(REMOVE ${TEMP_CONTENT_FILE})

    file(TO_NATIVE_PATH ${REPO_DIR}/patch_flag PATCH_FLAG_FILE)
    if(NOT EXISTS ${PATCH_FLAG_FILE})
      set(APPLY_PATCHES TRUE)
    else(NOT EXISTS ${PATCH_FLAG_FILE})
      # Get previous hash of applyied patches
      file(READ ${PATCH_FLAG_FILE} PREV_HASH)

      if(NOT ${PREV_HASH} STREQUAL ${CURR_HASH})
        set(APPLY_PATCHES TRUE)
      else(NOT ${PREV_HASH} STREQUAL ${CURR_HASH})
        set(APPLY_PATCHES FALSE)
      endif(NOT ${PREV_HASH} STREQUAL ${CURR_HASH})
    endif(NOT EXISTS ${PATCH_FLAG_FILE})

    # Apply patches 
    if(${APPLY_PATCHES})
      message("Reverting previous patches ...")
      execute_process(
          COMMAND git checkout -f ${TARGET_VERSION}
          COMMAND git clean -ffd
          COMMAND git submodule update -f --init
          WORKING_DIRECTORY ${REPO_DIR})

      message("Applying patches ...")

      foreach(PATCH_FILE IN LISTS ALL_PATCH_FIEL)
        file(TO_NATIVE_PATH ${PATCH_FILE} NATIVE_PATCH_FILE)
        message("Applying patch ${NATIVE_PATCH_FILE} ...")
          execute_process(
            COMMAND git apply --whitespace=warn ${NATIVE_PATCH_FILE}
            WORKING_DIRECTORY ${REPO_DIR})
      endforeach(PATCH_FILE)

      #save new hash
      file(WRITE ${PATCH_FLAG_FILE} ${CURR_HASH})

      message("info Patches applied!")
    endif(${APPLY_PATCHES})
  endif(${PATCH_NUM} GREATER 0)
endfunction(check_patch REPO_DIR TARGET_VERSION PATCH_DIR)

set(TARGET_ADF_VER v2.0-78-gcf28897)
check_repo_version($ENV{ADF_PATH} ${TARGET_ADF_VER})

set(TARGET_IDF_VER v4.0.1)
check_repo_version($ENV{IDF_PATH} ${TARGET_IDF_VER})

file(TO_CMAKE_PATH  ${CMAKE_SOURCE_DIR}/esp_adf_patch ADF_PATCH_DIR)
check_patch($ENV{ADF_PATH} ${TARGET_ADF_VER} ${ADF_PATCH_DIR})

file(TO_CMAKE_PATH  ${CMAKE_SOURCE_DIR}/esp_idf_patch IDF_PATCH_DIR)
check_patch($ENV{IDF_PATH} ${TARGET_IDF_VER} ${IDF_PATCH_DIR})

execute_process(COMMAND git describe --always --tags --dirty
                OUTPUT_VARIABLE APP_VER
                OUTPUT_STRIP_TRAILING_WHITESPACE)
add_definitions(-DAPP_VER=\"${APP_VER}\")

include($ENV{ADF_PATH}/CMakeLists.txt)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(AnyBoard)