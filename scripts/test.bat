@echo off
chcp 65001 >nul 2>&1
color 0A
echo ========================================
echo  AnyWP Engine - Auto Test
echo ========================================
echo.
echo Test Configuration:
echo - URL: examples/test_simple.html
echo - Mode: Wallpaper Mode (Mouse Transparent)
echo - Auto Start: After 3 seconds
echo.
echo ========================================
echo Test Steps:
echo ========================================
echo.
echo 1. App will auto-start wallpaper in 3 seconds
echo 2. Minimize app window to view desktop
echo 3. Check the following:
echo.
echo    Expected Results:
echo    - Desktop background becomes purple gradient
echo    - See "AnyWP Engine" title
echo    - See real-time clock
echo    - Desktop icons remain visible
echo.
echo ========================================
echo Log Monitoring (Important!)
echo ========================================
echo.
echo Watch for these key logs:
echo.
echo [Initialization]
echo   [AnyWP] ========== Initializing Wallpaper ==========
echo.
echo [WorkerW Detection]
echo   [AnyWP] Found SHELLDLL_DefView in WorkerW
echo   [AnyWP] Found NEXT WorkerW (wallpaper layer)
echo.
echo [Window Creation]
echo   [AnyWP] Creating child window: [width]x[height] (full screen)
echo   [AnyWP] Window visible: 1
echo.
echo [WebView2]
echo   [AnyWP] WebView2 environment created
echo   [AnyWP] WebView2 controller created
echo   [AnyWP] Navigating to: file:///...
echo.
echo [Complete]
echo   [AnyWP] ========== Initialization Complete ==========
echo.
echo ========================================
echo If failed, check for [ERROR] logs
echo ========================================
echo.
echo Press any key to start the app...
pause >nul
echo.
echo Starting application...
echo.

REM Check if Release version exists
set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe

if not exist "%EXE_PATH%" (
    echo.
    echo ================================
    echo  WARNING: Release version not found
    echo ================================
    echo.
    echo Trying Debug version...
    set EXE_PATH=%~dp0..\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
)

if not exist "%EXE_PATH%" (
    echo.
    echo ================================
    echo  ERROR: Executable not found!
    echo ================================
    echo.
    echo Please build the project first:
    echo   .\scripts\build_and_run.bat
    echo.
    pause
    exit /b 1
)

echo Running: %EXE_PATH%
echo.
cd /d "%~dp0..\example\build\windows\x64\runner\Release"
if exist "anywallpaper_engine_example.exe" (
    anywallpaper_engine_example.exe
) else (
    cd /d "%~dp0..\example\build\windows\x64\runner\Debug"
    anywallpaper_engine_example.exe
)

echo.
echo ========================================
echo Test Finished
echo ========================================
pause
