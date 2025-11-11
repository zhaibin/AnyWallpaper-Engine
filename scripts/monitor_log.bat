@echo off
REM ============================================================
REM Purpose: Real-time monitor application log output
REM Log files: test_logs/debug_run.log or test_logs/test_full_*.log
REM ============================================================

echo ===== AnyWP Engine - Log Monitor =====
echo.
echo Press Ctrl+C to stop monitoring
echo.

cd /d "%~dp0.."

:loop
timeout /t 2 /nobreak >nul
cls
echo ===== Latest Log (Last 50 Lines) =====
echo.

REM Priority: monitor debug_run.log first
if exist "test_logs\debug_run.log" (
    powershell -Command "Get-Content test_logs\debug_run.log -Tail 50 -ErrorAction SilentlyContinue"
) else (
    REM Find latest test_full log
    for /f "delims=" %%i in ('dir /b /o-d "test_logs\test_full_*.log" 2^>nul') do (
        powershell -Command "Get-Content test_logs\%%i -Tail 50 -ErrorAction SilentlyContinue"
        goto :found
    )
    echo Log file not found in test_logs directory
    echo Please run debug.bat or test_full.bat first
    :found
)

goto loop

