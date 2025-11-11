@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Simple Click Test
echo ========================================
echo.
echo Test Page: test_dom_events_simple.html
echo.
echo Instructions:
echo   1. Check "Complex Interaction Mode" checkbox
echo   2. Select "DOM Events Test (Simple)" from dropdown
echo   3. Click "Start Wallpaper"
echo   4. Click the BIG BUTTON in the center
echo   5. Watch the counters increase
echo.
echo Expected Results:
echo   - DOM click counter increases
echo   - Button click counter increases
echo   - Button changes color on click
echo.
pause

cd /d "%~dp0.."
start "" "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" > "test_logs\simple_click_test.log" 2>&1

echo.
echo Application started. Check the wallpaper!
echo Log file: test_logs\simple_click_test.log
echo.
pause


