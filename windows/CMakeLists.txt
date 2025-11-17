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
# Copy minified SDK file to plugin build directory (where DLL is built)
# Use minified version for better performance
# SDK moved to top-level sdk/dist/ directory in v2.2.0
# ==========================================

# Define SDK source path (relative to windows/ directory)
set(SDK_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../sdk/dist/anywp_sdk.min.js")

# Verify SDK file exists at configure time
if(NOT EXISTS "${SDK_SOURCE_PATH}")
  message(WARNING "SDK file not found at: ${SDK_SOURCE_PATH}")
  message(WARNING "SDK will need to be built before running the application")
else()
  message(STATUS "SDK file found: ${SDK_SOURCE_PATH}")
endif()

# Copy SDK to plugin build directory
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying SDK to plugin directory..."
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/sdk/dist"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_SOURCE_PATH}"
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/sdk/dist/anywp_sdk.js"
  COMMAND ${CMAKE_COMMAND} -E echo "SDK copied to: $<TARGET_FILE_DIR:${PLUGIN_NAME}>/sdk/dist/anywp_sdk.js"
  COMMENT "Copying minified SDK to plugin directory ($<CONFIG>)"
  VERBATIM
)

# Copy SDK file to runner directory (where application runs)
# This ensures the SDK is available at runtime
# Path: plugins/anywp_engine/Debug -> ../../.. -> x64 -> runner/Debug/sdk/dist
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying SDK to runner directory..."
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/sdk/dist"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_SOURCE_PATH}"
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/sdk/dist/anywp_sdk.js"
  COMMAND ${CMAKE_COMMAND} -E echo "SDK copied to: $<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/sdk/dist/anywp_sdk.js"
  COMMENT "Copying minified SDK to runner directory ($<CONFIG>)"
  VERBATIM
)

# Copy SDK to Flutter assets directory (for Release builds with flutter pub run)
# This ensures SDK is available when packaged as Flutter asset
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying SDK to Flutter assets directory..."
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/data/flutter_assets/sdk/dist"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_SOURCE_PATH}"
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/data/flutter_assets/sdk/dist/anywp_sdk.js"
  COMMAND ${CMAKE_COMMAND} -E echo "SDK copied to Flutter assets"
  COMMENT "Copying SDK to Flutter assets ($<CONFIG>)"
  VERBATIM
)

# Copy SDK to relative path for backward compatibility
# Some C++ code may look for "../sdk/anywp_sdk.js" relative to executable
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying SDK for backward compatibility..."
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/sdk"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_SOURCE_PATH}"
    "$<TARGET_FILE_DIR:${PLUGIN_NAME}>/../../../runner/$<CONFIG>/sdk/anywp_sdk.js"
  COMMAND ${CMAKE_COMMAND} -E echo "SDK copied for compatibility: sdk/anywp_sdk.js"
  COMMENT "Copying SDK for backward compatibility ($<CONFIG>)"
  VERBATIM
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

