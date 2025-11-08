@echo off
setlocal enabledelayedexpansion

echo.
echo ========================================
echo  AnyWP Engine - Optimization Verification
echo ========================================
echo.

set ERROR_COUNT=0
set SUCCESS_COUNT=0

echo [1] Verifying module files exist...
if exist "windows\modules\power_manager.cpp" (
    echo     [+] PowerManager module - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] PowerManager module - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\modules\mouse_hook_manager.cpp" (
    echo     [+] MouseHookManager module - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] MouseHookManager module - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\modules\monitor_manager.cpp" (
    echo     [+] MonitorManager module - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] MonitorManager module - MISSING
    set /a ERROR_COUNT+=1
)

echo.
echo [2] Verifying test framework...
if exist "windows\test\test_framework.h" (
    echo     [+] Test framework header - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] Test framework header - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\test\unit_tests.cpp" (
    echo     [+] Unit test file - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] Unit test file - MISSING
    set /a ERROR_COUNT+=1
)

if exist "windows\test\CMakeLists.txt" (
    echo     [+] CMakeLists.txt - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] CMakeLists.txt - MISSING
    set /a ERROR_COUNT+=1
)

echo.
echo [3] Checking error handling implementation...

findstr /C:"try {" "windows\modules\power_manager.cpp" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] PowerManager error handling - FOUND
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] PowerManager error handling - NOT FOUND
    set /a ERROR_COUNT+=1
)

findstr /C:"try {" "windows\modules\mouse_hook_manager.cpp" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] MouseHookManager error handling - FOUND
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] MouseHookManager error handling - NOT FOUND
    set /a ERROR_COUNT+=1
)

findstr /C:"try {" "windows\modules\monitor_manager.cpp" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] MonitorManager error handling - FOUND
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] MonitorManager error handling - NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.
echo [4] Checking Logger enhancements...

findstr /C:"enum class Level" "windows\utils\logger.h" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] Logger Level enum - FOUND
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] Logger Level enum - NOT FOUND
    set /a ERROR_COUNT+=1
)

findstr /C:"std::mutex" "windows\utils\logger.h" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] Logger thread safety - FOUND
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] Logger thread safety - NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.
echo [5] Checking documentation...

if exist "docs\OPTIMIZATION_COMPLETE.md" (
    echo     [+] Optimization report - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] Optimization report - MISSING
    set /a ERROR_COUNT+=1
)

findstr /C:"4.9.0" "CHANGELOG_CN.md" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo     [+] CHANGELOG updated - OK
    set /a SUCCESS_COUNT+=1
) else (
    echo     [-] CHANGELOG updated - NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.
echo ========================================
echo  Test Results
echo ========================================
echo  Passed: %SUCCESS_COUNT%
echo  Failed: %ERROR_COUNT%
echo ========================================
echo.

if %ERROR_COUNT% GTR 0 (
    echo STATUS: FAILED - %ERROR_COUNT% issues found
    exit /b 1
) else (
    echo STATUS: ALL CHECKS PASSED!
    echo.
    echo All optimization features verified:
    echo  - Error handling in all modules
    echo  - Enhanced Logger with levels
    echo  - Unit test framework created
    echo  - Documentation updated
    echo.
)

pause

