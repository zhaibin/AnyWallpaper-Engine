cmake_minimum_required(VERSION 3.14)
project(anywp_engine_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(unit_tests
  unit_tests.cpp
  ../utils/logger.cpp
  ../modules/power_manager.cpp
)

# Include directories
target_include_directories(unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Link system libraries
target_link_libraries(unit_tests
  psapi.lib
)

# Disable C4819 warning
target_compile_options(unit_tests PRIVATE /wd4819)

