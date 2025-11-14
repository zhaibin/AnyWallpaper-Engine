cmake_minimum_required(VERSION 3.14)
project(anywp_engine_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# WebView2 NuGet package
set(WEBVIEW2_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../packages/Microsoft.Web.WebView2.1.0.2651.64")

# Add source files for basic unit tests (includes Phase 2 & 3 modules + v2.1.0)
add_executable(unit_tests
  unit_tests.cpp
  ../utils/logger.cpp
  ../utils/memory_profiler.cpp
  ../utils/cpu_profiler.cpp
  ../utils/startup_optimizer.cpp
  ../utils/error_handler.cpp
  ../modules/power_manager.cpp
  ../modules/instance_manager.cpp
  ../modules/window_manager.cpp
  ../modules/display_change_coordinator.cpp
  ../modules/monitor_manager.cpp
  ../modules/event_dispatcher.cpp
  ../modules/memory_optimizer.cpp
)

# Include directories for basic tests
target_include_directories(unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${WEBVIEW2_PACKAGE_DIR}/build/native/include
)

# Link system libraries for basic tests (includes WebView2 for Phase 2 modules)
target_link_libraries(unit_tests
  psapi.lib
  ${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2LoaderStatic.lib
  Ole32.lib
  OleAut32.lib
  Shlwapi.lib
)

# Disable C4819 warning
target_compile_options(unit_tests PRIVATE /wd4819)

# WebViewManager integration tests (requires WebView2)
add_executable(webview_tests
  webview_manager_tests.cpp
  ../utils/logger.cpp
  ../modules/webview_manager.cpp
)

# Include directories for WebView tests
target_include_directories(webview_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${WEBVIEW2_PACKAGE_DIR}/build/native/include
)

# Link libraries for WebView tests
target_link_libraries(webview_tests
  ${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2LoaderStatic.lib
  Ole32.lib
  OleAut32.lib
  Shlwapi.lib
)

# Disable warnings for WebView tests
target_compile_options(webview_tests PRIVATE /wd4819)

