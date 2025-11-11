@echo off
setlocal

set "LOG_FILE=test_logs\live_test.log"

echo Starting application with logging...
echo Log file: %LOG_FILE%
echo.

REM Clear old log
if exist "%LOG_FILE%" del "%LOG_FILE%"

REM Start application with output redirection
start "AnyWP Test" /B cmd /c "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe > %LOG_FILE% 2>&1"

echo.
echo Application started!
echo Waiting 5 seconds for initialization...
timeout /t 5 /nobreak > nul

echo.
echo ========================================
echo Monitoring log file...
echo Press Ctrl+C to stop monitoring
echo ========================================
echo.

REM Monitor log file for ElementFinder entries
powershell -Command "Get-Content '%LOG_FILE%' -Wait | Select-String 'ElementFinder|DOMDispatch|Searching at|Found.*interactive|HIT:|MISS:' | ForEach-Object { Write-Host $_.Line }"


