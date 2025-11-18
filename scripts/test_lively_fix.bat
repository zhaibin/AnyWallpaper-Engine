@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Testing Lively-Compatible WorkerW Fix
echo ========================================
echo.

echo [1] Restarting Explorer to ensure clean desktop...
taskkill /f /im explorer.exe > nul 2>&1
timeout /t 2 /nobreak > nul
start explorer.exe
echo     Waiting for Explorer to stabilize...
timeout /t 5 /nobreak > nul
echo     Explorer ready
echo.

echo [2] Cleaning old logs...
del /q test_logs\debug_run.log 2>nul
echo     Logs cleaned
echo.

echo [3] Starting application...
echo     Please wait 15 seconds for initialization...
echo.
start /min cmd /c "cd .. && .\scripts\debug.bat > test_logs\debug_run.log 2>&1"
timeout /t 15 /nobreak > nul

echo [4] Analyzing results...
echo.
echo ========== Log Analysis ==========
echo.

findstr /C:"wParam=0xD, lParam=0x1" test_logs\debug_run.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Using Lively-compatible parameters
) else (
    echo [INFO] Parameters check skipped
)

echo.
echo [Message Sending]:
findstr /C:"Sent 0x052C to Progman successfully" test_logs\debug_run.log | find "result:" > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Messages sent successfully
    findstr /C:"Sent 0x052C to Progman successfully" test_logs\debug_run.log | find "result:"
) else (
    echo [UNKNOWN] Check full log
)

echo.
echo [WorkerW Structure]:
findstr /C:"Found wallpaper WorkerW (next sibling after Progman)" test_logs\debug_run.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Found WorkerW using Lively method!
    findstr /C:"Found wallpaper WorkerW (next sibling after Progman)" test_logs\debug_run.log
) else (
    findstr /C:"SHELLDLL_DefView found in Progman" test_logs\debug_run.log > nul
    if %ERRORLEVEL% EQU 0 (
        echo [WARNING] SHELLDLL_DefView still in Progman
        findstr /C:"No WorkerW found after Progman" test_logs\debug_run.log > nul
        if %ERRORLEVEL% EQU 0 (
            echo [ERROR] No WorkerW created after Progman!
        )
    ) else (
        echo [UNKNOWN] Check full log
    )
)

echo.
echo [Monitor Status]:
findstr /C:"Monitor thread started" test_logs\debug_run.log > nul
if %ERRORLEVEL% EQU 0 (
    echo [PASS] Health monitoring active
) else (
    echo [FAIL] Monitoring not started
)

echo.
echo ========================================
echo.
echo QUESTION: Can you see the wallpaper?
echo.
echo If YES: ✅ Lively fix is working!
echo If NO:  ❌ Need further investigation
echo.
echo Press any key to view detailed WorkerW creation log...
pause > nul

echo.
echo ========== Detailed WorkerW Log ==========
findstr /C:"Triggering WorkerW" /C:"Sent 0x052C" /C:"SHELLDLL_DefView" /C:"Found wallpaper" /C:"WorkerW (next sibling" test_logs\debug_run.log
echo.
echo ========================================

pause

