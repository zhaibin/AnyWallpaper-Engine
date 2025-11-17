cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "anywp_engine")
project(${PROJECT_NAME} LANGUAGES CXX RC)  # Add RC for Windows Resource Compiler

# ==========================================
# VC Runtime Linking (v2.3.0+)
# Note: Must use dynamic linking (/MD) to match Flutter framework
# ==========================================
# Flutter Windows plugins must use dynamic CRT (/MD) because:
# 1. flutter_wrapper_plugin.lib is built with /MD
# 2. Static linking (/MT) causes LNK2038 runtime library mismatch
# 3. Windows 10/11 systems typically have VC Runtime 2015-2022 pre-installed
#
# Dependencies (usually pre-installed on Windows 10/11):
#   - VC Runtime 2015-2022 (via Windows Update)
#   - WebView2 Runtime (included with Microsoft Edge)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "anywp_engine_plugin")

add_library(${PLUGIN_NAME} SHARED
  "anywp_engine_plugin.cpp"
  "sdk_loader.cpp"
  "sdk_resource.rc"
  "utils/state_persistence.cpp"
  "utils/logger.cpp"
  "utils/url_validator.cpp"
  "utils/desktop_wallpaper_helper.cpp"
  "utils/resource_tracker.cpp"
  "utils/conflict_detector.cpp"
  "utils/input_validator.cpp"
  "utils/memory_profiler.cpp"
  "utils/cpu_profiler.cpp"
  "utils/startup_optimizer.cpp"
  "utils/error_handler.cpp"
  "utils/performance_benchmark.cpp"
  "utils/permission_manager.cpp"
  "utils/event_bus.cpp"
  "utils/config_manager.cpp"
  "utils/service_locator.cpp"
  "utils/mime_type_detector.cpp"
  "modules/iframe_detector.cpp"
  "modules/sdk_bridge.cpp"
  "modules/mouse_hook_manager.cpp"
  "modules/monitor_manager.cpp"
  "modules/power_manager.cpp"
  "modules/webview_manager.cpp"
  "modules/flutter_bridge.cpp"
  "modules/display_change_coordinator.cpp"
  "modules/instance_manager.cpp"
  "modules/window_manager.cpp"
  "modules/initialization_coordinator.cpp"
  "modules/webview_configurator.cpp"
  "modules/event_dispatcher.cpp"
  "modules/memory_optimizer.cpp"
  "modules/custom_scheme_handler.cpp"
)

apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_compile_options(${PLUGIN_NAME} PRIVATE 
  /wd4819  # Disable C4819 encoding warning
  /wd4244  # Disable C4244 wchar_t to char conversion warning
)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

# ==========================================
# Precompiled DLL Detection
# ==========================================
# Check if we're using precompiled DLL (for precompiled package distribution)
set(PRECOMPILED_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../bin/anywp_engine_plugin.dll")
set(PRECOMPILED_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/anywp_engine_plugin.lib")
set(USING_PRECOMPILED FALSE)

if(EXISTS "${PRECOMPILED_DLL}" AND EXISTS "${PRECOMPILED_LIB}")
  set(USING_PRECOMPILED TRUE)
  message(STATUS "Using precompiled DLL: ${PRECOMPILED_DLL}")
  
  # Remove the source-based library and create an imported one instead
  # Note: We can't remove a target once created, so we'll handle this differently
  # For precompiled packages, the CMakeLists.txt should be different
  # But for compatibility, we'll check if source files exist
endif()

# ==========================================
# WebView2 - use NuGet package (only if not using precompiled)
# ==========================================
if(NOT USING_PRECOMPILED)
  set(WEBVIEW2_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.2592.51")
  if(EXISTS "${WEBVIEW2_PACKAGE_DIR}")
    # Add WebView2 include directory to both private and interface (public)
    target_include_directories(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/include")
    target_include_directories(${PLUGIN_NAME} INTERFACE "${WEBVIEW2_PACKAGE_DIR}/build/native/include")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      target_link_libraries(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2LoaderStatic.lib")
    else()
      target_link_libraries(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x86/WebView2LoaderStatic.lib")
    endif()
  else()
    message(FATAL_ERROR "WebView2 package not found at ${WEBVIEW2_PACKAGE_DIR}. Please run nuget install.")
  endif()
else()
  # Using precompiled DLL - WebView2 is already linked in the DLL
  message(STATUS "Skipping WebView2 package check (using precompiled DLL)")
endif()

# Windows SDK libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE
  shlwapi
  version
)

# List of absolute paths to libraries that should be bundled with the plugin
set(anywp_engine_bundled_libraries
  ""
  PARENT_SCOPE
)

# ==========================================
# Embedded SDK Configuration (v2.3.0+)
# SDK is now embedded in DLL as a Windows resource
# No need to copy external files - SDK is part of the DLL!
# ==========================================

# Define SDK source path for embedding (relative to windows/ directory)
set(SDK_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/dist/anywp_sdk.js")

# Verify SDK file exists at configure time (required for RC compilation)
if(NOT EXISTS "${SDK_SOURCE_PATH}")
  message(WARNING "========================================")
  message(WARNING "SDK file not found at: ${SDK_SOURCE_PATH}")
  message(WARNING "SDK will NOT be embedded in DLL!")
  message(WARNING "Please build SDK first: cd sdk/src && npm run build")
  message(WARNING "========================================")
else()
  message(STATUS "✓ SDK file found: ${SDK_SOURCE_PATH}")
  # Get file size for info
  file(SIZE "${SDK_SOURCE_PATH}" SDK_FILE_SIZE)
  math(EXPR SDK_SIZE_KB "${SDK_FILE_SIZE} / 1024")
  message(STATUS "✓ SDK will be embedded in DLL (${SDK_SIZE_KB} KB)")
endif()

# Note: SDK is automatically embedded by RC compiler at build time
# The sdk_resource.rc file references the SDK and compiles it into the DLL
# At runtime, sdk_loader.cpp extracts the SDK from DLL resources
# No external SDK files are needed for deployment!

# ==========================================
# INSTALL rules for precompiled DLL distribution
# NOTE: These rules cause Flutter build to fail with INSTALL step errors.
# They are commented out by default. The release.bat script handles file copying.
# Uncomment only if you need manual cmake --install support (not for Flutter builds).
# ==========================================

# # Define paths for precompiled binaries
# set(PRECOMPILED_DLL "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.dll")
# set(PRECOMPILED_LIB "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.lib")
# 
# # WebView2Loader.dll path
# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#   set(WEBVIEW2_DLL "${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2Loader.dll")
# else()
#   set(WEBVIEW2_DLL "${WEBVIEW2_PACKAGE_DIR}/build/native/x86/WebView2Loader.dll")
# endif()
# 
# # Install DLL files to bin/
# install(FILES "${PRECOMPILED_DLL}" DESTINATION bin
#         CONFIGURATIONS Release)
# install(FILES "${WEBVIEW2_DLL}" DESTINATION bin
#         CONFIGURATIONS Release)
# 
# # Install LIB file to lib/
# install(FILES "${PRECOMPILED_LIB}" DESTINATION lib
#         CONFIGURATIONS Release)
# 
# # Install C API header to include/anywp_engine/
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/anywp_engine_plugin_c_api.h"
#         DESTINATION include/anywp_engine)
# 
# # Install full C++ header (optional, for source integration)
# install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/anywp_engine_plugin.h"
#         DESTINATION include/anywp_engine)

