cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "anywp_engine")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "anywp_engine_plugin")

add_library(${PLUGIN_NAME} SHARED
  "anywp_engine_plugin.cpp"
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

# WebView2 - use NuGet package
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

