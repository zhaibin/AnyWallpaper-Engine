@echo off
REM 测试运行时 Explorer 重启的自动恢复
setlocal enabledelayedexpansion

echo ========================================
echo Testing Runtime Explorer Restart Recovery
echo ========================================
echo.

echo [Step 1] Cleaning old logs...
del /q test_logs\explorer_restart_test.log 2>nul
echo.

echo [Step 2] Starting application...
echo     Application will start in background
start /min cmd /c "cd /d "%~dp0.." && example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe > test_logs\explorer_restart_test.log 2>&1"
timeout /t 10 /nobreak > nul
echo     Application should be running now
echo.

echo [Step 3] Verifying wallpaper is visible...
echo     QUESTION: Can you see the wallpaper now?
echo     Press any key when you can see it...
pause > nul
echo.

echo [Step 4] Restarting Explorer to trigger recovery...
echo     WARNING: Desktop will refresh, wallpaper should auto-recover
timeout /t 3 /nobreak > nul
taskkill /f /im explorer.exe > nul 2>&1
timeout /t 2 /nobreak > nul
start explorer.exe
echo     Explorer restarted
echo.

echo [Step 5] Waiting for auto-recovery (30 seconds)...
echo     The health monitor should detect Explorer restart and trigger recovery
for /L %%i in (1,1,30) do (
    echo     Waiting... %%i/30 seconds
    timeout /t 1 /nobreak > nul
)
echo.

echo [Step 6] Analyzing recovery logs...
echo.
echo ========== Recovery Analysis ==========
echo.

findstr /C:"Explorer restart detected" test_logs\explorer_restart_test.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Explorer restart was detected
    findstr /C:"Explorer restart detected" test_logs\explorer_restart_test.log | find /V "FINDSTR"
) else (
    echo [FAIL] Explorer restart was NOT detected
)

echo.
findstr /C:"Triggering recovery due to Explorer restart" test_logs\explorer_restart_test.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Recovery was triggered
) else (
    echo [FAIL] Recovery was NOT triggered
)

echo.
findstr /C:"WorkerW Recovery" /C:"Re-finding WorkerW" test_logs\explorer_restart_test.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Recovery process executed
    findstr /C:"WorkerW Recovery" test_logs\explorer_restart_test.log | find /V "FINDSTR" | find /V "=========="
) else (
    echo [FAIL] Recovery process did NOT execute
)

echo.
echo ========================================
echo.
echo FINAL QUESTION: Can you see the wallpaper after Explorer restart?
echo.
echo If YES: ✅ Auto-recovery is working!
echo If NO:  ❌ Recovery mechanism needs fixing
echo.
echo Press any key to view detailed recovery log...
pause > nul

echo.
echo ========== Detailed Recovery Log ==========
findstr /C:"Explorer" /C:"Recovery" /C:"Re-finding" /C:"Re-parenting" /C:"Monitor thread" test_logs\explorer_restart_test.log | find /V "Initial Explorer PID"
echo.
echo ========================================

echo.
echo Application is still running. Press any key to stop it...
pause > nul
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1
echo Application stopped.

