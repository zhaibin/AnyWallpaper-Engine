@echo off
REM 测试全新桌面环境下的 WorkerW 创建（Explorer 重启场景）
setlocal enabledelayedexpansion

echo ========================================
echo Testing WorkerW Creation on Fresh Desktop
echo ========================================
echo.

REM 1. 检查是否有其他壁纸程序运行
echo [1] Checking for other wallpaper applications...
tasklist /fi "imagename eq lively.exe" 2>nul | find /i "lively.exe" >nul
if %ERRORLEVEL% EQU 0 (
    echo    WARNING: Lively is running! Please close it first.
    pause
    exit /b 1
)
echo    No other wallpaper applications detected
echo.

REM 2. 重启 Explorer 创建全新桌面环境
echo [2] Restarting Explorer to create fresh desktop...
echo    WARNING: Desktop icons will briefly disappear!
timeout /t 3 /nobreak > nul

taskkill /f /im explorer.exe > nul 2>&1
timeout /t 2 /nobreak > nul
start explorer.exe
echo    Explorer restarted, waiting for desktop to stabilize...
timeout /t 5 /nobreak > nul
echo    Desktop ready
echo.

REM 3. 清理旧日志
echo [3] Cleaning old logs...
del /q test_logs\debug_run.log 2>nul
echo    Old logs cleared
echo.

REM 4. 启动应用
echo [4] Starting application...
echo    Please wait for wallpaper to initialize (about 10 seconds)...
echo.
start /min .\scripts\debug.bat
timeout /t 15 /nobreak > nul

REM 5. 检查日志
echo [5] Checking logs for WorkerW creation results...
echo.
echo ========== Log Analysis ==========

REM 检查 SendMessageTimeout 结果
echo.
echo [Message Sending]:
findstr /C:"Sent 0x052C to Progman successfully" test_logs\debug_run.log
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Messages sent successfully
) else (
    echo [FAIL] Failed to send messages
)

REM 检查 SHELLDLL_DefView 位置
echo.
echo [Desktop Structure]:
findstr /C:"SHELLDLL_DefView found in Progman" test_logs\debug_run.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [FAIL] SHELLDLL_DefView still in Progman ^(Problem!^)
    echo    This means WorkerW structure was NOT created correctly
) else (
    findstr /C:"WorkerW found successfully" test_logs\debug_run.log > nul
    if %ERRORLEVEL% EQU 0 (
        echo [PASS] WorkerW structure created successfully
        echo    SHELLDLL_DefView is in WorkerW ^(Correct!^)
    ) else (
        echo [UNKNOWN] Check log for details
    )
)

REM 检查监控线程
echo.
echo [Health Monitoring]:
findstr /C:"Monitor thread started" test_logs\debug_run.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Health monitoring started
) else (
    echo [FAIL] Health monitoring NOT started
)

echo.
echo ========================================
echo.
echo QUESTION: Can you see the wallpaper on your desktop?
echo.
echo If YES: ✅ Fix is working!
echo If NO:  ❌ Still has issues, check log above
echo.
echo ========================================
echo.
echo Press any key to view detailed WorkerW creation log...
pause > nul

echo.
echo ========== Detailed WorkerW Creation Log ==========
findstr /C:"Triggering WorkerW" /C:"Sent 0x052C" /C:"WorkerW with SHELLDLL_DefView" /C:"WorkerW found" test_logs\debug_run.log
echo.
echo ========================================

pause

