@echo off
REM Test AnyWP Draggable Functionality
setlocal

echo ========================================
echo AnyWP Engine - Draggable Feature Test
echo ========================================
echo.
echo Testing:
echo   - Drag and drop elements
echo   - Position persistence
echo   - Click handlers
echo   - State management
echo.
echo Instructions:
echo   1. Wait for wallpaper to load
echo   2. Try dragging colored boxes and widgets
echo   3. Click "Toggle Interaction" button
echo   4. Try "Reset All Positions" button
echo   5. Close and restart to verify position persistence
echo.

cd example

REM Clean build
if exist "build" (
    echo Cleaning old build...
    rmdir /s /q "build" 2>nul
)

echo Building...
flutter build windows --debug >nul 2>&1
if errorlevel 1 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo Starting draggable test...
echo.

REM Run with draggable test page
start /B "" "build\windows\x64\runner\Debug\anywp_engine_example.exe" --test-url="file:///%cd%\..\examples\test_draggable.html" > test_output.log 2>&1

REM Wait a bit for initialization
timeout /t 2 >nul

echo.
echo ========================================
echo Test running...
echo.
echo Expected behavior:
echo   1. Purple gradient background
echo   2. Info panel with instructions
echo   3. Three draggable elements:
echo      - "拖动我 #1" (blue box)
echo      - "数据面板" (white card)
echo      - "音乐播放器" (white card)
echo   4. Status panel showing SDK info
echo   5. Two buttons (Toggle/Reset)
echo.
echo Test checklist:
echo   [ ] Drag blue box around
echo   [ ] Drag data panel
echo   [ ] Drag music player
echo   [ ] Click "禁用交互模式" button
echo   [ ] Click "启用交互模式" button
echo   [ ] Click "复位所有位置" button
echo   [ ] Close and restart - positions restored?
echo.
echo Press Ctrl+C to stop test
echo ========================================

REM Monitor logs
timeout /t 3 >nul
if exist "test_output.log" (
    echo.
    echo === Recent Logs ===
    powershell -Command "Get-Content test_output.log -Tail 20"
)

cd ..
endlocal


