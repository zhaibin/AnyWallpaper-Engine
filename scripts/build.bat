@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Build Script
REM Build Debug version and launch application
REM ==========================================

set TOTAL_STEPS=3
set STEP=1

echo ================================
echo  AnyWP Engine - Build and Run
echo ================================
echo.

REM Check WebView2 SDK
call :PrintStep "Checking WebView2 SDK..."
cd /d "%~dp0..\windows"
if not exist "packages\Microsoft.Web.WebView2.1.0.2592.51" (
    echo WebView2 SDK not found. Installing...
    echo.
    cd /d "%~dp0"
    call setup.bat
    if errorlevel 1 (
        echo ERROR: Failed to install WebView2 SDK
        pause
        exit /b 1
    )
)

cd /d "%~dp0..\example"

call :PrintStep "Building Debug version..."
call flutter build windows --debug

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    pause
    exit /b 1
)

call :PrintStep "Starting application..."
start build\windows\x64\runner\Debug\anywallpaper_engine_example.exe

echo.
echo ================================
echo  Build Complete!
echo ================================
goto :EOF

:PrintStep
echo [Step !STEP!/%TOTAL_STEPS%] %~1
set /a STEP+=1
goto :EOF
