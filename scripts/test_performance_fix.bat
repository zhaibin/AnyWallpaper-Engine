@echo off
REM Performance test script after mouse logging optimization
REM Tests page navigation speed and log reduction

setlocal enabledelayedexpansion

echo ========================================
echo Performance Test - Mouse Logging Fix
echo ========================================
echo.

set "LOG_FILE=test_logs\performance_fix_test.log"
set "EXE_PATH=example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"

REM Ensure test_logs directory exists
if not exist "test_logs" mkdir test_logs

echo [INFO] Starting application...
echo [INFO] Log file: %LOG_FILE%
echo.
echo [TEST] Please perform the following actions:
echo   1. Wait for app to start (5 seconds)
echo   2. Start wallpaper
echo   3. Switch between test pages (Simple, Visibility, API)
echo   4. Move mouse around for 10 seconds
echo   5. Close the application
echo.
echo Press any key to start test...
pause >nul

REM Start application with logging
start /B "" "%EXE_PATH%" > "%LOG_FILE%" 2>&1

echo [INFO] Application started
echo [INFO] Please follow the test steps above
echo.
echo Press any key when test is complete (after closing app)...
pause >nul

echo.
echo ========================================
echo Test Results
echo ========================================
echo.

REM Count total log lines
for /f %%a in ('type "%LOG_FILE%" ^| find /c /v ""') do set TOTAL_LINES=%%a
echo Total log lines: %TOTAL_LINES%

REM Count mouse-related logs
for /f %%a in ('type "%LOG_FILE%" ^| find /c "MouseHookManager"') do set MOUSE_LOGS=%%a
echo MouseHookManager logs: %MOUSE_LOGS%

REM Count mousemove logs (should be 0 now)
for /f %%a in ('type "%LOG_FILE%" ^| find /c "type=mousemove"') do set MOUSEMOVE_LOGS=%%a
echo MouseMove logs: %MOUSEMOVE_LOGS%

REM Count mousedown/mouseup logs
for /f %%a in ('type "%LOG_FILE%" ^| find /c "type=mousedown"') do set MOUSEDOWN_LOGS=%%a
for /f %%a in ('type "%LOG_FILE%" ^| find /c "type=mouseup"') do set MOUSEUP_LOGS=%%a
echo MouseDown logs: %MOUSEDOWN_LOGS%
echo MouseUp logs: %MOUSEUP_LOGS%

echo.
echo ========================================
echo Analysis
echo ========================================
echo.

if %MOUSEMOVE_LOGS% EQU 0 (
    echo [SUCCESS] Mouse move logging eliminated!
    echo [SUCCESS] Performance optimization successful
) else (
    echo [WARNING] Still found %MOUSEMOVE_LOGS% mouse move logs
    echo [WARNING] Optimization may not be complete
)

echo.
echo Log file location: %LOG_FILE%
echo.
pause

