@echo off
setlocal

set "LOG_FILE=test_logs\coordinate_test.log"

echo ========================================
echo Coordinate Conversion Test
echo ========================================
echo Log file: %LOG_FILE%
echo.

REM Clear old log
if exist "%LOG_FILE%" del "%LOG_FILE%"

echo Starting application...
example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe > "%LOG_FILE%" 2>&1

echo.
echo Application closed. Analyzing log...
echo.


