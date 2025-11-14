cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "anywp_engine")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "anywp_engine_plugin")

# ==========================================
# Precompiled DLL Mode
# ==========================================
# This CMakeLists.txt is for precompiled package distribution
# It uses IMPORTED library instead of compiling from source

# Paths to precompiled binaries (relative to this CMakeLists.txt)
set(PRECOMPILED_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../bin/anywp_engine_plugin.dll")
set(PRECOMPILED_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/anywp_engine_plugin.lib")

if(NOT EXISTS "${PRECOMPILED_DLL}")
  message(FATAL_ERROR "Precompiled DLL not found at ${PRECOMPILED_DLL}")
endif()

if(NOT EXISTS "${PRECOMPILED_LIB}")
  message(FATAL_ERROR "Precompiled LIB not found at ${PRECOMPILED_LIB}")
endif()

# Create imported library
add_library(${PLUGIN_NAME} SHARED IMPORTED)
set_target_properties(${PLUGIN_NAME} PROPERTIES
  IMPORTED_LOCATION "${PRECOMPILED_DLL}"
  IMPORTED_IMPLIB "${PRECOMPILED_LIB}"
)

# Include directories (for C API header)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

# Link Flutter libraries
target_link_libraries(${PLUGIN_NAME} INTERFACE flutter flutter_wrapper_plugin)

# Windows SDK libraries (still needed for linking)
target_link_libraries(${PLUGIN_NAME} INTERFACE
  shlwapi
  version
)

# List of absolute paths to libraries that should be bundled with the plugin
set(anywp_engine_bundled_libraries
  "${PRECOMPILED_DLL}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../bin/WebView2Loader.dll"
  PARENT_SCOPE
)

message(STATUS "Using precompiled AnyWP Engine DLL: ${PRECOMPILED_DLL}")

