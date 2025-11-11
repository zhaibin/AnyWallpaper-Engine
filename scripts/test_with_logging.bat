@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Complex Interaction Mode Test with Logging
echo ========================================
echo.

set "EXE_PATH=%~dp0..\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
set "LOG_FILE=%~dp0..\test_logs\complex_interaction_test.log"

REM Create test_logs directory if not exists
if not exist "%~dp0..\test_logs" mkdir "%~dp0..\test_logs"

echo [INFO] Starting application with logging...
echo [INFO] Log file: %LOG_FILE%
echo.
echo Please:
echo   1. Check "Complex Interaction Mode" checkbox
echo   2. Select "Mouse Transparency (v2.0.2)" test page
echo   3. Click "Start Wallpaper"
echo   4. Then close the app window
echo.
echo Press any key to start...
pause > nul

REM Start the app and redirect output to log file
echo [%date% %time%] Test started > "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

"%EXE_PATH%" >> "%LOG_FILE%" 2>&1

echo.
echo [INFO] Application closed. Analyzing logs...
echo.

REM Display relevant log entries
echo ======================================== 
echo Key Log Entries:
echo ======================================== 
findstr /C:"[AnyWP]" /C:"[Plugin]" /C:"[WindowManager]" /C:"interaction" /C:"transparent" /C:"Monitor" "%LOG_FILE%"

echo.
echo ======================================== 
echo Full log saved to: %LOG_FILE%
echo ======================================== 
echo.

pause



