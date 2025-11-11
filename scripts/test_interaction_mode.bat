@echo off
REM Test Complex Interaction Mode with detailed logging
echo ===============================================
echo Complex Interaction Mode Test
echo ===============================================
echo.

REM Create log directory
if not exist "test_logs" mkdir test_logs

REM Set log file with timestamp
set TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
set LOG_FILE=test_logs\interaction_mode_%TIMESTAMP%.log

echo Starting test at %date% %time% > %LOG_FILE%
echo. >> %LOG_FILE%

echo [INFO] Starting Flutter app with debug output...
echo [INFO] Log file: %LOG_FILE%
echo.

REM Run the app and capture all output
start /B example\build\windows\x64\Debug\example.exe > %LOG_FILE% 2>&1

echo [INFO] App started. Please:
echo 1. Check the "Complex Interaction Mode" checkbox
echo 2. Select test page: Mouse Transparency
echo 3. Click "Start All Monitors" or "Start Wallpaper"
echo 4. Observe the mode indicator on the page
echo.
echo [INFO] Monitoring log file...
echo.

REM Wait a moment for app to initialize
timeout /t 3 /nobreak > nul

REM Show recent log entries
echo ===============================================
echo Recent log entries:
echo ===============================================
powershell -Command "Get-Content '%LOG_FILE%' -Tail 30"

echo.
echo.
echo To continue monitoring, run:
echo   powershell -Command "Get-Content '%LOG_FILE%' -Wait -Tail 20"
echo.
echo Press any key to stop monitoring...
pause > nul

