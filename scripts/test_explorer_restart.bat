@echo off
REM 测试 Explorer 重启时的 WorkerW 自动恢复功能
setlocal enabledelayedexpansion

echo ========================================
echo Testing WorkerW Recovery on Explorer Restart
echo ========================================
echo.

REM 1. 编译最新版本
echo [1] Building latest version...
cd example
flutter build windows --debug > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
echo    Build completed
echo.

REM 2. 启动应用
echo [2] Starting application...
cd build\windows\x64\runner\Debug
start "" anywallpaper_engine_example.exe

REM 3. 等待初始化
echo [3] Waiting 15 seconds for wallpaper initialization...
timeout /t 15 /nobreak > nul
echo    Initialization period completed
echo.

REM 4. 获取初始 Explorer PID
echo [4] Getting initial Explorer PID...
for /f "tokens=2" %%a in ('tasklist /fi "imagename eq explorer.exe" /fo list ^| findstr PID') do (
    set OLD_PID=%%a
)
echo    Initial Explorer PID: !OLD_PID!
echo.

REM 5. 重启 Explorer
echo [5] Restarting Explorer to trigger WorkerW recovery...
echo    WARNING: Your desktop will briefly disappear!
timeout /t 3 /nobreak > nul

taskkill /f /im explorer.exe > nul 2>&1
timeout /t 2 /nobreak > nul
start explorer.exe
timeout /t 3 /nobreak > nul

REM 6. 获取新 Explorer PID
echo [6] Getting new Explorer PID...
for /f "tokens=2" %%a in ('tasklist /fi "imagename eq explorer.exe" /fo list ^| findstr PID') do (
    set NEW_PID=%%a
)
echo    New Explorer PID: !NEW_PID!
echo.

REM 7. 等待恢复
echo [7] Waiting 10 seconds for WorkerW recovery...
timeout /t 10 /nobreak > nul
echo    Recovery period completed
echo.

REM 8. 检查日志
cd ..\..\..\..\..\..\test_logs
echo [8] Checking log for recovery events...
echo.
echo ========== Log Analysis ==========

REM 查找 Explorer 重启检测
findstr /C:"Explorer restart detected" debug_run.log
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Explorer restart detected
) else (
    echo [FAIL] Explorer restart NOT detected
)
echo.

REM 查找恢复触发
findstr /C:"Triggering recovery due to Explorer restart" debug_run.log
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Recovery triggered
) else (
    echo [FAIL] Recovery NOT triggered
)
echo.

REM 查找死锁错误
findstr /C:"resource deadlock" debug_run.log
if %ERRORLEVEL% EQU 0 (
    echo [FAIL] DEADLOCK DETECTED!
) else (
    echo [PASS] No deadlock
)
echo.

REM 查找恢复完成
findstr /C:"Recovery completed successfully" debug_run.log
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Recovery completed successfully
) else (
    echo [FAIL] Recovery did NOT complete
)
echo.

echo ========================================
echo Test completed. Check wallpaper visibility!
echo ========================================
echo.
echo Press any key to view full recovery log...
pause > nul

REM 显示完整恢复日志
echo.
echo ========== Full Recovery Log ==========
findstr /C:"WorkerW Recovery" debug_run.log
echo.
echo ========================================

cd ..\scripts
pause

