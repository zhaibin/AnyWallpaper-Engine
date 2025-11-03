@echo off
chcp 65001 >nul 2>&1
echo ===== 实时监控日志 =====
echo 日志文件: test_output.log
echo.
echo 按 Ctrl+C 停止监控
echo.

:loop
timeout /t 2 /nobreak >nul
cls
echo ===== 最新日志（最后50行）=====
echo.
type ..\test_output.log 2>nul | find /V "" | more +0
goto loop

