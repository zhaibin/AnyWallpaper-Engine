@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Mouse Transparent Bug Fix Test
echo ========================================
echo.
echo This script will test both modes:
echo   1. Mouse transparent ENABLED (should pass through)
echo   2. Mouse transparent DISABLED (should be interactive)
echo.
echo Please follow the instructions for each test.
echo.
pause

REM Test 1: Mouse transparent ENABLED
echo.
echo ========================================
echo TEST 1: Mouse Transparent ENABLED
echo ========================================
echo.
echo Expected: Mouse should pass through the wallpaper
echo           Desktop icons should be clickable
echo           Wallpaper content should NOT respond to clicks
echo.
echo Press any key to start test 1...
pause >nul

start "" "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"

echo.
echo Test 1 running... Please verify:
echo   [ ] Mouse passes through wallpaper
echo   [ ] Desktop icons are clickable
echo   [ ] Wallpaper does NOT respond to clicks
echo.
echo Press any key when done testing, then close the app...
pause >nul

echo.
echo.

REM Test 2: Mouse transparent DISABLED
echo ========================================
echo TEST 2: Mouse Transparent DISABLED
echo ========================================
echo.
echo Instructions:
echo   1. In the app, UNCHECK "Mouse Transparent" option
echo   2. The wallpaper should become interactive
echo.
echo Expected: Mouse should interact with wallpaper
echo           Wallpaper content should respond to clicks
echo           Desktop icons should be blocked by wallpaper
echo.
echo Press any key to continue...
pause >nul

echo.
echo Test 2 verification:
echo   [ ] Wallpaper responds to clicks
echo   [ ] Can drag elements in wallpaper
echo   [ ] Desktop icons are blocked (if wallpaper covers them)
echo.
echo Press any key to finish testing...
pause >nul

echo.
echo ========================================
echo Testing Complete
echo ========================================
echo.
echo If both tests passed, the bug is fixed!
echo If not, please report the results.
echo.
pause


