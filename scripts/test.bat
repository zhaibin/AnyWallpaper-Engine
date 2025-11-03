@echo off
chcp 65001 >nul 2>&1
echo ================================
echo  AnyWP Engine - Test
echo ================================
echo.

REM Auto-detect executable (Release > Debug)
set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe
set BUILD_TYPE=Release

if not exist "%EXE_PATH%" (
    set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
    set BUILD_TYPE=Debug
)

if not exist "%EXE_PATH%" (
    echo ERROR: Executable not found!
    echo.
    echo Please build first: .\scripts\build_and_run.bat
    echo.
    pause
    exit /b 1
)

echo Running %BUILD_TYPE% version...
echo.
echo Expected: Purple gradient wallpaper with clock
echo Watch logs for: [AnyWP] initialization steps
echo.

start "" "%EXE_PATH%"
