cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "anywp_engine")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must not be changed
set(PLUGIN_NAME "anywp_engine_plugin")

# Use precompiled library
set(PRECOMPILED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Create an imported library target
add_library(${PLUGIN_NAME} SHARED IMPORTED GLOBAL)

# Set the location of the precompiled DLL
set_target_properties(${PLUGIN_NAME} PROPERTIES
  IMPORTED_LOCATION "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
  IMPORTED_IMPLIB "${PRECOMPILED_DIR}/lib/anywp_engine_plugin.lib"
)

# Include directories
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${PRECOMPILED_DIR}/include"
)

# List of absolute paths to libraries that should be bundled with the plugin
set(anywp_engine_bundled_libraries
  "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
  "${PRECOMPILED_DIR}/bin/WebView2Loader.dll"
  PARENT_SCOPE
)

