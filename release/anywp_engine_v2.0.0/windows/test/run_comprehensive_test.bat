@echo off
REM Comprehensive Test for All Independent Modules

echo ========================================
echo AnyWP Engine v2.0 - Comprehensive Tests
echo ========================================
echo.
echo Testing All Independent Modules:
echo - Logger
echo - PowerManager
echo - URLValidator
echo - StatePersistence
echo - IframeDetector
echo - MouseHookManager
echo - MonitorManager
echo - SDKBridge
echo - MemoryProfiler
echo - CPUProfiler
echo - StartupOptimizer
echo - DesktopWallpaperHelper
echo - ResourceTracker
echo.

REM Check compiler
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Visual Studio compiler not found
    echo Please run from "Developer Command Prompt for VS"
    exit /b 1
)

set WEBVIEW2_DIR=%cd%\..\packages\Microsoft.Web.WebView2.1.0.2592.51

if not exist "%WEBVIEW2_DIR%" (
    echo ERROR: WebView2 package not found
    echo Path: %WEBVIEW2_DIR%
    exit /b 1
)

echo Compiling comprehensive tests...
echo.

cl /EHsc /std:c++17 ^
   /I"%cd%\.." ^
   /I"%WEBVIEW2_DIR%\build\native\include" ^
   "%cd%\comprehensive_test.cpp" ^
   "%cd%\..\utils\logger.cpp" ^
   "%cd%\..\utils\resource_tracker.cpp" ^
   "%cd%\..\utils\desktop_wallpaper_helper.cpp" ^
   "%cd%\..\utils\url_validator.cpp" ^
   "%cd%\..\utils\state_persistence.cpp" ^
   "%cd%\..\utils\memory_profiler.cpp" ^
   "%cd%\..\utils\cpu_profiler.cpp" ^
   "%cd%\..\utils\startup_optimizer.cpp" ^
   "%cd%\..\modules\power_manager.cpp" ^
   "%cd%\..\modules\mouse_hook_manager.cpp" ^
   "%cd%\..\modules\monitor_manager.cpp" ^
   "%cd%\..\modules\iframe_detector.cpp" ^
   "%cd%\..\modules\sdk_bridge.cpp" ^
   /link /out:"comprehensive_test.exe" ^
   "%WEBVIEW2_DIR%\build\native\x64\WebView2LoaderStatic.lib" ^
   psapi.lib User32.lib Ole32.lib OleAut32.lib Shlwapi.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERROR: Compilation failed!
    echo ========================================
    exit /b 1
)

echo.
echo Compilation successful!
echo.
echo ========================================
echo Running Comprehensive Tests
echo ========================================
echo.

comprehensive_test.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo ========================================
if %TEST_RESULT% EQU 0 (
    echo All tests PASSED!
) else (
    echo Some tests FAILED!
)
echo ========================================
echo.

exit /b %TEST_RESULT%


