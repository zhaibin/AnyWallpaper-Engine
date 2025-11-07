@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

echo ===============================================================
echo        AnyWP Engine Refactoring Test Suite
echo ===============================================================
echo.

set PASS=0
set FAIL=0
set TOTAL=0

:: Test 1: Check module files exist
echo === Test 1: Module Files ===
call :TestFile "windows\utils\state_persistence.h" "StatePersistence header"
call :TestFile "windows\utils\state_persistence.cpp" "StatePersistence impl"
call :TestFile "windows\utils\logger.h" "Logger header"
call :TestFile "windows\utils\logger.cpp" "Logger impl"
call :TestFile "windows\utils\url_validator.h" "URLValidator header"
call :TestFile "windows\utils\url_validator.cpp" "URLValidator impl"
call :TestFile "windows\modules\iframe_detector.h" "IframeDetector header"
call :TestFile "windows\modules\iframe_detector.cpp" "IframeDetector impl"
call :TestFile "windows\modules\sdk_bridge.h" "SDKBridge header"
call :TestFile "windows\modules\sdk_bridge.cpp" "SDKBridge impl"
call :TestFile "windows\modules\mouse_hook_manager.h" "MouseHookManager header"
call :TestFile "windows\modules\mouse_hook_manager.cpp" "MouseHookManager impl"
call :TestFile "windows\modules\monitor_manager.h" "MonitorManager header"
call :TestFile "windows\modules\monitor_manager.cpp" "MonitorManager impl"
call :TestFile "windows\modules\power_manager.h" "PowerManager header"
call :TestFile "windows\modules\power_manager.cpp" "PowerManager impl"
echo.

:: Test 2: Check build artifacts
echo === Test 2: Build Artifacts ===
call :TestFile "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" "Example EXE"
call :TestFile "example\build\windows\x64\plugins\anywp_engine\Debug\anywp_engine_plugin.dll" "Plugin DLL"
echo.

:: Test 3: Check test pages
echo === Test 3: Test Pages ===
call :TestFile "examples\test_refactoring.html" "Refactoring test page"
call :TestFile "examples\test_simple.html" "Simple test page"
call :TestFile "examples\test_draggable.html" "Draggable test page"
echo.

:: Test 4: Check CMake configuration
echo === Test 4: CMake Configuration ===
findstr /C:"state_persistence.cpp" windows\CMakeLists.txt > nul
if !errorlevel! equ 0 (
    call :Pass "CMake includes state_persistence"
) else (
    call :Fail "CMake missing state_persistence"
)

findstr /C:"logger.cpp" windows\CMakeLists.txt > nul
if !errorlevel! equ 0 (
    call :Pass "CMake includes logger"
) else (
    call :Fail "CMake missing logger"
)

findstr /C:"url_validator.cpp" windows\CMakeLists.txt > nul
if !errorlevel! equ 0 (
    call :Pass "CMake includes url_validator"
) else (
    call :Fail "CMake missing url_validator"
)
echo.

:: Test 5: Check application status
echo === Test 5: Application Status ===
tasklist /FI "IMAGENAME eq anywallpaper_engine_example.exe" 2>NUL | find /I /N "anywallpaper_engine_example.exe" > nul
if !errorlevel! equ 0 (
    call :Pass "Application is running"
) else (
    echo [SKIP] Application not running
)
echo.

:: Summary
echo ===============================================================
echo                     Test Summary
echo ===============================================================
echo Total: !TOTAL!
echo Passed: !PASS!
echo Failed: !FAIL!
echo.

set /a PASS_RATE=!PASS!*100/!TOTAL!
echo Pass Rate: !PASS_RATE!%%
echo.

if !FAIL! equ 0 (
    echo [SUCCESS] All tests passed^^! Refactoring successful^^!
    exit /b 0
) else (
    echo [WARNING] %FAIL% test^(s^) failed^^!
    exit /b %FAIL%
)

:TestFile
if exist "%~1" (
    call :Pass "%~2"
) else (
    call :Fail "%~2 - File missing: %~1"
)
exit /b

:Pass
set /a TOTAL+=1
set /a PASS+=1
echo [PASS] %~1
exit /b

:Fail
set /a TOTAL+=1
set /a FAIL+=1
echo [FAIL] %~1
exit /b

