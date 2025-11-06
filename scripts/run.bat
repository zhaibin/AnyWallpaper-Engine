@echo off
chcp 65001 >nul 2>&1
setlocal
REM ============================================================
REM 用途：灵活运行工具（支持参数选项）
REM 参数：-d/--debug (Debug版) -r/--release (Release版)
REM       -f/--flutter (使用 Flutter 运行，支持热重载)
REM 适用：需要不同运行模式时使用
REM ============================================================

echo ================================
echo  AnyWP Engine - 运行工具
echo ================================
echo.

REM Parse command line arguments
set MODE=debug
set ACTION=run

:parse_args
if "%~1"=="" goto :end_parse
if /i "%~1"=="--release" set MODE=release
if /i "%~1"=="-r" set MODE=release
if /i "%~1"=="--debug" set MODE=debug
if /i "%~1"=="-d" set MODE=debug
if /i "%~1"=="--flutter" set ACTION=flutter
if /i "%~1"=="-f" set ACTION=flutter
if /i "%~1"=="--help" goto :show_help
if /i "%~1"=="-h" goto :show_help
shift
goto :parse_args

:end_parse

REM Display current configuration
echo Configuration:
echo   Mode: %MODE%
echo   Action: %ACTION%
echo.

if "%ACTION%"=="flutter" goto :run_flutter

REM Run compiled program
:run_compiled
if "%MODE%"=="release" (
    set BUILD_DIR=Release
    echo Running Release version...
) else (
    set BUILD_DIR=Debug
    echo Running Debug version...
)

set EXE_PATH=%~dp0..\example\build\windows\x64\runner\%BUILD_DIR%\anywallpaper_engine_example.exe

if not exist "%EXE_PATH%" (
    echo.
    echo ================================
    echo  ERROR: Executable not found!
    echo ================================
    echo.
    echo File path: %EXE_PATH%
    echo.
    echo Please build the project first:
    echo   .\scripts\build_and_run.bat
    echo.
    pause
    exit /b 1
)

echo.
echo Starting application...
echo Path: %EXE_PATH%
echo.
echo ================================
echo.

cd /d "%~dp0..\example\build\windows\x64\runner\%BUILD_DIR%"
start "" anywallpaper_engine_example.exe
goto :end

REM Run with Flutter
:run_flutter
echo Running with Flutter...
echo.
cd /d "%~dp0..\example"
if "%MODE%"=="release" (
    flutter run -d windows --release
) else (
    flutter run -d windows --debug
)
goto :end

:show_help
echo Usage: run.bat [options]
echo.
echo Options:
echo   -d, --debug      Run Debug version (default)
echo   -r, --release    Run Release version
echo   -f, --flutter    Run with Flutter (hot reload)
echo   -h, --help       Show this help
echo.
echo Examples:
echo   run.bat                Run Debug version
echo   run.bat --release      Run Release version
echo   run.bat -f             Run with Flutter
echo   run.bat -r -f          Run Release with Flutter
echo.
exit /b 0

:end
endlocal
