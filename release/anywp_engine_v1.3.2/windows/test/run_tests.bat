@echo off
REM AnyWP Engine - Unit Test Build Script
REM 
REM This script compiles and runs the unit tests for AnyWP Engine modules.

echo ========================================
echo AnyWP Engine - Unit Tests
echo ========================================
echo.

REM Check if compiler exists
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Visual Studio compiler not found in PATH
    echo Please run this from "Developer Command Prompt for VS"
    pause
    exit /b 1
)

echo Compiling unit tests...
echo.

REM Compile unit tests
cl /EHsc /std:c++17 ^
   /I"%cd%\.." ^
   "%cd%\unit_tests.cpp" ^
   "%cd%\..\utils\logger.cpp" ^
   "%cd%\..\modules\power_manager.cpp" ^
   /link /out:"unit_tests.exe" ^
   psapi.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERROR: Compilation failed!
    echo ========================================
    pause
    exit /b 1
)

echo.
echo ========================================
echo Compilation successful!
echo ========================================
echo.

echo Running tests...
echo.

REM Run tests
unit_tests.exe

set TEST_RESULT=%ERRORLEVEL%

echo.
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo All tests PASSED! ✓
    echo ========================================
) else (
    echo ========================================
    echo Some tests FAILED!
    echo ========================================
)

pause
exit /b %TEST_RESULT%


