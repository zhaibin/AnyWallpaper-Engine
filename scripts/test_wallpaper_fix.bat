@echo off
echo ========================================
echo Wallpaper Interactive Test v2.0.1
echo ========================================
echo.
echo Starting application...
echo.
echo Instructions:
echo 1. Wait for app to load
echo 2. Click "Wallpaper Interactive (v2.0.1)" test page
echo 3. Click the test box (DO NOT move mouse)
echo 4. Check if mouseover count increases
echo 5. Check console logs for debug output
echo.
echo Press Ctrl+C to stop the test
echo ========================================
echo.

cd example\build\windows\x64\runner\Debug
start anywallpaper_engine_example.exe

echo.
echo Application launched!
echo Check the app window for test results.
echo.
pause

