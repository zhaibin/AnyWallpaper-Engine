@echo off
REM Test position tracking feature across all test pages
REM This script compiles and runs the app to test automatic position tracking

echo ========================================
echo Position Tracking Feature Test
echo ========================================
echo.
echo Test Pages:
echo   1. test_position_tracking.html - Dedicated position tracking test
echo   2. test_refactoring.html - Updated with SDK auto-tracking
echo   3. test_basic_click.html - Basic click test
echo   4. test_draggable.html - Draggable elements
echo   5. test_api.html - Full API test
echo.

cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine\example

REM Kill existing instance if running
taskkill /F /IM anywallpaper_engine_example.exe 2>nul

echo Starting application...
echo.
start "" "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"

echo.
echo ========================================
echo Test Instructions:
echo ========================================
echo.
echo TEST 1: Position Tracking Test Page
echo   1. Click "Position Tracking Test" button
echo   2. Click "Start" button
echo   3. Use arrow buttons to move the target button
echo   4. Click the target button to verify click region follows
echo.
echo TEST 2: Refactoring Page (Auto-tracking)
echo   1. Click "Refactoring Test" button
echo   2. Click any button to add logs
echo   3. Watch as buttons remain clickable even when layout changes
echo.
echo TEST 3: Animated Elements
echo   1. Go to "Position Tracking Test" page
echo   2. Click the bouncing/rotating/pulsing boxes
echo   3. Verify clicks are detected despite continuous animation
echo.
echo Expected Results:
echo   - Click regions automatically follow element position changes
echo   - No need for manual refresh or layout watchers
echo   - Console shows: "ResizeObserver detected change" messages
echo.
echo ========================================

pause

