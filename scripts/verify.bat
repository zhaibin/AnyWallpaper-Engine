@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Verification Script
REM Verify module files and optimizations
REM ==========================================

echo ================================
echo  AnyWP Engine - Verification
echo ================================
echo.

set ERROR_COUNT=0
set SUCCESS_COUNT=0

echo [1] Verifying module files...
if exist "windows\modules\power_manager.cpp" (
    echo   [+] PowerManager module
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] PowerManager module - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\modules\mouse_hook_manager.cpp" (
    echo   [+] MouseHookManager module
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] MouseHookManager module - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\modules\monitor_manager.cpp" (
    echo   [+] MonitorManager module
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] MonitorManager module - MISSING
    set /a ERROR_COUNT+=1
)

echo.
echo [2] Verifying test framework...
if exist "windows\test\test_framework.h" (
    echo   [+] Test framework header
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] Test framework header - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\test\unit_tests.cpp" (
    echo   [+] Unit test file
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] Unit test file - MISSING
    set /a ERROR_COUNT+=1
)

echo.
echo [3] Checking error handling...
findstr /C:"try {" "windows\modules\power_manager.cpp" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [+] PowerManager error handling
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] PowerManager error handling - NOT FOUND
    set /a ERROR_COUNT+=1
)

findstr /C:"try {" "windows\modules\mouse_hook_manager.cpp" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [+] MouseHookManager error handling
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] MouseHookManager error handling - NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.
echo [4] Checking Logger enhancements...
findstr /C:"enum class Level" "windows\utils\logger.h" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [+] Logger Level enum
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] Logger Level enum - NOT FOUND
    set /a ERROR_COUNT+=1
)

findstr /C:"std::mutex" "windows\utils\logger.h" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [+] Logger thread safety
    set /a SUCCESS_COUNT+=1
) else (
    echo   [-] Logger thread safety - NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.
echo ================================
echo  Verification Results
echo ================================
echo  Passed: %SUCCESS_COUNT%
echo  Failed: %ERROR_COUNT%
echo ================================
echo.

if %ERROR_COUNT% GTR 0 (
    echo STATUS: FAILED - %ERROR_COUNT% issues found
    exit /b 1
) else (
    echo STATUS: ALL CHECKS PASSED
)

pause

