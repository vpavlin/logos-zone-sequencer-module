cmake_minimum_required(VERSION 3.20)
project(logos-zone-sequencer-module LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Qt ----
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
set(CMAKE_AUTOMOC ON)

# ---- OS Specifics ----
if(APPLE)
    set(DYLIB_EXT ".dylib")
else()
    set(DYLIB_EXT ".so")
endif()

# ---- Rust FFI library ----
if(NOT ZONE_SEQUENCER_RS_LIB_DIR)
  set(ZONE_SEQUENCER_RS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../zone-sequencer-rs")
  set(ZONE_SEQUENCER_RS_LIB_DIR "${ZONE_SEQUENCER_RS_ROOT}/target/release")
endif()
set(ZONE_SEQUENCER_RS_LIB "${ZONE_SEQUENCER_RS_LIB_DIR}/libzone_sequencer_rs${DYLIB_EXT}")

# ---- Plugin ----
set(PLUGIN_TARGET logos_zone_sequencer_module)

qt_add_plugin(${PLUGIN_TARGET} CLASS_NAME LogosZoneSequencerModule)

target_sources(${PLUGIN_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logos_zone_sequencer_module.cpp
)

set_property(TARGET ${PLUGIN_TARGET} PROPERTY PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i_logos_zone_sequencer_module.h
)

if(LOGOS_CPP_SDK_ROOT)
  target_include_directories(${PLUGIN_TARGET} PRIVATE
      ${LOGOS_CPP_SDK_ROOT}/include
      ${LOGOS_CPP_SDK_ROOT}/include/cpp
      ${LOGOS_CPP_SDK_ROOT}/include/core
      ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
else()
  target_include_directories(${PLUGIN_TARGET} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/../logos-cpp-sdk
      ${CMAKE_CURRENT_SOURCE_DIR}/../logos-cpp-sdk/cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/../logos-cpp-sdk/core
      ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
endif()

# Link by name (-lzone_sequencer_rs) so the NEEDED entry uses the soname, not an absolute path.
# This lets the runtime linker find the library via RUNPATH ($ORIGIN).
target_link_directories(${PLUGIN_TARGET} PRIVATE "${ZONE_SEQUENCER_RS_LIB_DIR}")

if(LOGOS_CPP_SDK_ROOT)
    target_link_libraries(${PLUGIN_TARGET} PRIVATE
        Qt6::Core
        Qt6::Concurrent
        zone_sequencer_rs
        ${LOGOS_CPP_SDK_ROOT}/lib/liblogos_sdk.a
    )
else()
    target_link_libraries(${PLUGIN_TARGET} PRIVATE
        Qt6::Core
        Qt6::Concurrent
        zone_sequencer_rs
    )
endif()

target_compile_definitions(${PLUGIN_TARGET} PRIVATE
    LOGOS_ZONE_SEQUENCER_MODULE_METADATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
)

if(APPLE)
    set_target_properties(${PLUGIN_TARGET} PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX)
    set_target_properties(${PLUGIN_TARGET} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# ---- Install ----
install(TARGETS ${PLUGIN_TARGET}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)
install(FILES "${ZONE_SEQUENCER_RS_LIB}" DESTINATION lib)
