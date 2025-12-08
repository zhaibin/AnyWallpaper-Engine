# AnyWP Engine macOS Plugin CMake Configuration

cmake_minimum_required(VERSION 3.10)
project(anywp_engine VERSION 2.6.5 LANGUAGES OBJC)

# Set C++ standard (though we're using Objective-C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS specific settings
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum macOS deployment version")

# Plugin library
add_library(anywp_engine SHARED
  # Main plugin
  "Classes/AnyWPEnginePlugin.h"
  "Classes/AnyWPEnginePlugin.m"
  
  # Modules
  "Classes/Modules/MonitorManager.h"
  "Classes/Modules/MonitorManager.m"
  "Classes/Modules/WallpaperManager.h"
  "Classes/Modules/WallpaperManager.m"
  "Classes/Modules/PowerManager.h"
  "Classes/Modules/PowerManager.m"
  "Classes/Modules/MessageBridge.h"
  "Classes/Modules/MessageBridge.m"
  
  # Utils
  "Classes/Utils/Logger.h"
  "Classes/Utils/Logger.m"
  "Classes/Utils/StatePersistence.h"
  "Classes/Utils/StatePersistence.m"
  "Classes/Utils/EmbeddedSDK.h"
  "Classes/Utils/EmbeddedSDK.m"
  "Classes/Utils/AWPCustomSchemeHandler.h"
  "Classes/Utils/AWPCustomSchemeHandler.m"
  "Classes/Utils/LocalFileServer.h"
  "Classes/Utils/LocalFileServer.m"
)

# Apply Flutter plugin configuration
set_target_properties(anywp_engine PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  OUTPUT_NAME "anywp_engine"
)

# Link required frameworks
target_link_libraries(anywp_engine PRIVATE
  "-framework Foundation"
  "-framework AppKit"
  "-framework WebKit"
)

# Set the plugin install location
set(INSTALL_BUNDLE_LIB_DIR "${CMAKE_INSTALL_PREFIX}")

# Install the plugin library
install(TARGETS anywp_engine
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${INSTALL_BUNDLE_LIB_DIR}
  RUNTIME DESTINATION ${INSTALL_BUNDLE_LIB_DIR}
)

# Note: Flutter will handle the actual build integration

