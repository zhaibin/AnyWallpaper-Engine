@echo off
REM ============================================================
REM Purpose: run demo in debug mode and capture logs to file
REM Log file: debug_run.log (project root)
REM Usage: troubleshooting and feature debugging
REM ============================================================

echo ========================================
echo AnyWP Engine - Debug Mode with Logging
echo ========================================
echo.

cd /d "%~dp0.."

echo Stopping existing processes...
taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1

echo.
echo Starting application with logging...
echo Log file: %CD%\debug_run.log
echo.

REM Check if Debug or Release exists
if exist "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" (
    echo Running from: %CD%\example\build\windows\x64\runner\Debug\
    echo.
    REM Run and capture output to log file
    start /B example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe > debug_run.log 2>&1
) else if exist "example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe" (
    echo Running from: %CD%\example\build\windows\x64\runner\Release\
    echo.
    REM Run and capture output to log file
    start /B example\build\windows\x64\runner\Release\anywallpaper_engine_example.exe > debug_run.log 2>&1
) else (
    echo ERROR: Executable not found in Debug or Release folder!
    echo Please build the project first.
    pause
    exit /b 1
)

echo.
echo Application exited. Check debug_run.log for details.
pause

