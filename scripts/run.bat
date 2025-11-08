@echo off
REM ==========================================
REM AnyWP Engine - Quick Run Script
REM Run existing build (Release > Debug)
REM ==========================================

echo ================================
echo  AnyWP Engine - Quick Run
echo ================================
echo.

REM Detect executable (prefer Release)
set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe
set BUILD_TYPE=Release

if not exist "%EXE_PATH%" (
    set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
    set BUILD_TYPE=Debug
)

if not exist "%EXE_PATH%" (
    echo ERROR: Executable not found
    echo Please run: scripts\build.bat
    echo.
    pause
    exit /b 1
)

echo Running %BUILD_TYPE% version...
echo.
start "" "%EXE_PATH%"

