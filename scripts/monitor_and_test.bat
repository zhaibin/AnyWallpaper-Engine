@echo off
REM AnyWP Engine - 自动监听日志和测试
setlocal enabledelayedexpansion

echo ========================================
echo  AnyWP Engine - Auto Test with Logging
echo ========================================
echo.

REM 清理旧日志
if exist "..\test_auto.log" del "..\test_auto.log"

echo [1/3] Starting application with logging...
cd ..\example
start /B "" "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" > "..\test_auto.log" 2>&1

echo [2/3] Waiting for application to start (10 seconds)...
timeout /t 10 /nobreak > nul

echo [3/3] Analyzing logs...
cd ..

REM 分析日志
echo.
echo ========================================
echo  Log Analysis Results
echo ========================================
echo.

findstr /I /C:"Inject" /C:"SDK" /C:"anywp_sdk" test_auto.log > nul
if %ERRORLEVEL% == 0 (
    echo [OK] SDK injection logs found
    findstr /I /C:"Inject" /C:"SDK" /C:"anywp_sdk" test_auto.log
) else (
    echo [ERROR] No SDK injection logs found!
    echo This means InjectAnyWallpaperSDK was NOT called
)

echo.
echo ========================================
echo.
echo Application is running. Please check the wallpaper.
echo.
echo Press any key to view full logs...
pause > nul

type test_auto.log

