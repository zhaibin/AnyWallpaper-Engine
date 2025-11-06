@echo off
REM ============================================================
REM Purpose: Real-time monitor application log output
REM Log files: debug_run.log or example/test_output.log
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
if exist "debug_run.log" (
    powershell -Command "Get-Content debug_run.log -Tail 50 -ErrorAction SilentlyContinue"
) else if exist "example\test_output.log" (
    powershell -Command "Get-Content example\test_output.log -Tail 50 -ErrorAction SilentlyContinue"
) else (
    echo Log file not found
    echo Please run debug_run.bat or build_and_run.bat first
)

goto loop

