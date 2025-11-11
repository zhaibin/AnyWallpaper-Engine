@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul 2>&1

echo ========================================
echo Testing Interactive Mode Fix
echo ========================================
echo.

REM Kill existing instance
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1

echo Starting application...
echo Look for this output in console:
echo   - [AnyWP] ========== CRITICAL FIX ==========
echo   - [AnyWP] Has WS_EX_TRANSPARENT: YES/NO
echo   - [WindowManager] SetInteractive(true)
echo   - [AnyWP] After SetInteractive - Has WS_EX_TRANSPARENT: NO
echo.

REM Start application and keep window open
cd /d "%~dp0..\example\build\windows\x64\runner\Debug"
start "" anywallpaper_engine_example.exe

echo.
echo Application started. Please:
echo 1. Select a test page (e.g., Wallpaper Interactive)
echo 2. Watch the console window for debug output
echo 3. Copy the section between "========== CRITICAL FIX =========="
echo.
pause

