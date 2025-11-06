@echo off
chcp 65001 >nul 2>&1
REM ============================================================
REM 用途：调试模式运行，捕获所有日志到文件
REM 日志：debug_run.log（项目根目录）
REM 适用：问题排查、功能调试
REM ============================================================

echo ========================================
echo AnyWP Engine - 调试模式（带日志）
echo ========================================
echo.

cd /d "%~dp0.."

echo Stopping existing processes...
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1

echo.
echo Starting application with logging...
echo Log file: %CD%\debug_run.log
echo.

echo Running from: %CD%\example\build\windows\x64\runner\Release\
echo.

REM Run and capture output to log file
example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe > debug_run.log 2>&1

echo.
echo Application exited. Check debug_run.log for details.
pause

