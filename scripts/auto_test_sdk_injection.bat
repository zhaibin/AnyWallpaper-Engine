@echo off
REM Auto test SDK injection with specific URL
REM This script will automatically trigger wallpaper initialization and monitor logs

setlocal enabledelayedexpansion

set TEST_URL=https://hkcw-web.pages.dev/wallpapers/theme1

echo ========================================
echo Auto Test SDK Injection
echo ========================================
echo.
echo Test URL: %TEST_URL%
echo.

REM Check if SDK file exists
if not exist "windows\anywp_sdk.js" (
    echo [ERROR] SDK file NOT found: windows\anywp_sdk.js
    echo Please build the SDK first: .\scripts\build_sdk.bat
    pause
    exit /b 1
)

echo [OK] SDK file found
echo.

REM Build application
echo Building application...
cd example
call flutter clean >nul 2>&1
call flutter pub get >nul 2>&1
call flutter build windows --debug > ..\test_logs\auto_test_build.log 2>&1

if errorlevel 1 (
    echo [ERROR] Build failed! Check test_logs\auto_test_build.log
    pause
    exit /b 1
)

echo [OK] Build successful
echo.

REM Start application in background
echo Starting application...
start "" "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" > ..\test_logs\auto_test_run.log 2>&1

REM Wait for application to start
timeout /t 3 /nobreak >nul

echo Application started. Waiting for initialization...
echo.

REM Monitor logs for SDK injection
echo Monitoring logs for SDK injection...
echo Press Ctrl+C to stop monitoring
echo.

:monitor_loop
REM Check for SDK injection logs
findstr /C:"SDK loaded" /C:"SDK registered" /C:"SDK injection" /C:"SDK verification" /C:"Navigation completed" /C:"initializeWallpaper" ..\test_logs\auto_test_run.log 2>nul
if errorlevel 0 (
    echo.
    echo [FOUND] SDK injection logs detected!
    echo.
)

REM Wait before next check
timeout /t 2 /nobreak >nul 2>&1
goto monitor_loop

endlocal

