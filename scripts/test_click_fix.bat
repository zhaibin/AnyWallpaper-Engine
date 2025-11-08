@echo off
REM Quick test for click bug fix
REM This script compiles and runs the app to test the click functionality

echo ========================================
echo Click Bug Fix Test
echo ========================================
echo.
echo Starting application...
echo.

cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine\example

REM Kill existing instance if running
taskkill /F /IM anywallpaper_engine_example.exe 2>nul

REM Start the app
start "" "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"

echo.
echo ========================================
echo Test Steps:
echo ========================================
echo 1. Click "Click Test" button
echo 2. Click "Start" button
echo 3. Click the "+ Increment" button once
echo 4. Check if counter increases by 1 (not 3)
echo 5. Click the "- Decrement" button once
echo 6. Check if counter decreases by 1 (not 3)
echo.
echo Expected Result:
echo - Click once = +1 or -1
echo - NOT +3 or -3
echo.
echo Console Messages:
echo - You should see: "Element already has click handler, skipping duplicate registration"
echo - This means duplicate registration is being prevented
echo.
echo ========================================

pause

