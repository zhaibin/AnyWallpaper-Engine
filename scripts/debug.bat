@echo off
REM ==========================================
REM AnyWP Engine - Debug Script
REM Run with output logging to test_logs/debug_run.log
REM ==========================================

echo ================================
echo  AnyWP Engine - Debug Mode
echo ================================
echo.

cd /d "%~dp0.."

REM Create test_logs directory if not exists
if not exist "test_logs" mkdir test_logs

echo Stopping existing processes...
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1
timeout /t 1 /nobreak >nul

REM Find executable
set EXE_PATH=
if exist "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" (
    set EXE_PATH=example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
) else if exist "example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe" (
    set EXE_PATH=example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe
)

if "%EXE_PATH%"=="" (
    echo ERROR: Executable not found
    echo Please run: scripts\build.bat
    pause
    exit /b 1
)

echo Starting with logging...
echo Log file: %CD%\test_logs\debug_run.log
echo.
%EXE_PATH% > test_logs\debug_run.log 2>&1

echo.
echo Application exited. Check test_logs\debug_run.log
pause

