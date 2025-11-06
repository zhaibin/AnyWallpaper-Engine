@echo off
chcp 65001 >nul 2>&1
REM ============================================================
REM 用途：实时监控应用日志输出
REM 日志文件：debug_run.log 或 example/test_output.log
REM ============================================================

echo ===== AnyWP Engine - 日志监控 =====
echo.
echo 按 Ctrl+C 停止监控
echo.

cd /d "%~dp0.."

:loop
timeout /t 2 /nobreak >nul
cls
echo ===== 最新日志（最后 50 行）=====
echo.

REM 优先监控 debug_run.log
if exist "debug_run.log" (
    powershell -Command "Get-Content debug_run.log -Tail 50 -ErrorAction SilentlyContinue"
) else if exist "example\test_output.log" (
    powershell -Command "Get-Content example\test_output.log -Tail 50 -ErrorAction SilentlyContinue"
) else (
    echo 未找到日志文件
    echo 请先运行 debug_run.bat 或 build_and_run.bat
)

goto loop

