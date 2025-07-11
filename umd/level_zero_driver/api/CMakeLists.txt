#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TARGET_NAME_L0 ze_intel_npu)
add_library(${TARGET_NAME_L0} SHARED)

target_sources(${TARGET_NAME_L0} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_cmdlist.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_cmdqueue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_event.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_fence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zes_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zes_driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zes_engine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zes_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zet_metric.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zet_misc.cpp
)
target_link_libraries(${TARGET_NAME_L0} level_zero_driver)
target_link_options(${TARGET_NAME_L0} PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/ze.exports)
add_subdirectory(ext)
add_subdirectory(prv)

set_target_properties(${TARGET_NAME_L0} PROPERTIES
    FOLDER ${TARGET_NAME_L0}
    VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
)

install(TARGETS ${TARGET_NAME_L0}
        COMPONENT level-zero-npu)
