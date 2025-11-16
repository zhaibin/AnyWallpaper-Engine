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

REM ========================================
REM Part 1: Unit Tests (includes Phase 2 modules)
REM ========================================
echo [1/2] Compiling unit tests...
echo.

set WEBVIEW2_DIR=%cd%\..\packages\Microsoft.Web.WebView2.1.0.2592.51

REM Check if WebView2 package exists
if not exist "%WEBVIEW2_DIR%" (
    echo ERROR: WebView2 package not found
    echo Required for Phase 2 module tests
    echo Path: %WEBVIEW2_DIR%
    pause
    exit /b 1
)

REM Compile unit tests (Phase 1 + Phase 3 modules - Flutter-independent)
REM Note: Some Phase 1 modules (MouseHookManager, IframeDetector, SDKBridge) depend on Flutter headers
REM and will be tested via integration tests
cl /EHsc /std:c++17 ^
   /I"%cd%\.." ^
   /I"%WEBVIEW2_DIR%\build\native\include" ^
   "%cd%\unit_tests.cpp" ^
   "%cd%\..\utils\logger.cpp" ^
   "%cd%\..\utils\resource_tracker.cpp" ^
   "%cd%\..\utils\conflict_detector.cpp" ^
   "%cd%\..\utils\desktop_wallpaper_helper.cpp" ^
   "%cd%\..\utils\url_validator.cpp" ^
   "%cd%\..\utils\state_persistence.cpp" ^
   "%cd%\..\utils\memory_profiler.cpp" ^
   "%cd%\..\utils\cpu_profiler.cpp" ^
   "%cd%\..\utils\startup_optimizer.cpp" ^
   "%cd%\..\modules\power_manager.cpp" ^
   "%cd%\..\modules\monitor_manager.cpp" ^
   /link /out:"unit_tests.exe" ^
   "%WEBVIEW2_DIR%\build\native\x64\WebView2LoaderStatic.lib" ^
   psapi.lib User32.lib Ole32.lib OleAut32.lib Shlwapi.lib Shell32.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERROR: Unit tests compilation failed!
    echo ========================================
    pause
    exit /b 1
)

echo.
echo [1/2] Unit tests compilation successful!
echo.

REM ========================================
REM Part 2: WebViewManager Tests (optional)
REM ========================================
echo [2/2] Compiling WebViewManager tests...
echo.

REM Check if WebView2 package exists (already set above)
if not exist "%WEBVIEW2_DIR%" (
    echo WARNING: WebView2 package not found
    echo WebViewManager tests will be skipped
    echo Path: %WEBVIEW2_DIR%
    goto RUN_TESTS
)

REM Compile WebViewManager tests
cl /EHsc /std:c++17 ^
   /I"%cd%\.." ^
   /I"%WEBVIEW2_DIR%\build\native\include" ^
   "%cd%\webview_manager_tests.cpp" ^
   "%cd%\..\utils\logger.cpp" ^
   "%cd%\..\modules\webview_manager.cpp" ^
   /link /out:"webview_tests.exe" ^
   "%WEBVIEW2_DIR%\build\native\x64\WebView2LoaderStatic.lib" ^
   Ole32.lib OleAut32.lib Shlwapi.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo WARNING: WebViewManager tests compilation failed
    echo Continuing with unit tests only...
    goto RUN_TESTS
)

echo.
echo [2/2] WebViewManager tests compilation successful!
echo.

:RUN_TESTS
REM ========================================
REM Run Tests
REM ========================================
echo ========================================
echo Running Tests
echo ========================================
echo.

echo [1/2] Running unit tests (includes Phase 2 modules)...
echo.
unit_tests.exe
set UNIT_RESULT=%ERRORLEVEL%

echo.
if %UNIT_RESULT% EQU 0 (
    echo [1/2] Unit tests: PASSED
) else (
    echo [1/2] Unit tests: FAILED
)

REM Run WebView tests if executable exists
if exist "webview_tests.exe" (
    echo.
    echo [2/2] Running WebViewManager tests...
    echo.
    webview_tests.exe
    set WEBVIEW_RESULT=%ERRORLEVEL%
    
    echo.
    if %WEBVIEW_RESULT% EQU 0 (
        echo [2/2] WebViewManager tests: PASSED
    ) else (
        echo [2/2] WebViewManager tests: FAILED
    )
) else (
    echo.
    echo [2/2] WebViewManager tests: SKIPPED (not compiled)
    set WEBVIEW_RESULT=0
)

REM Calculate overall result
set /a TOTAL_RESULT=%UNIT_RESULT% + %WEBVIEW_RESULT%

echo.
echo ========================================
if %TOTAL_RESULT% EQU 0 (
    echo All tests PASSED!
) else (
    echo Some tests FAILED!
)
echo ========================================

pause
exit /b %TOTAL_RESULT%


