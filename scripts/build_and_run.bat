@echo off
chcp 65001 >nul 2>&1
REM ============================================================
REM 用途：构建并运行示例应用（开发常用）
REM 功能：自动检查 WebView2 SDK → 构建 Debug 版本 → 启动应用
REM 适用：日常开发测试
REM ============================================================

echo ================================
echo  AnyWP Engine - 构建并运行
echo ================================
echo.

REM Check if WebView2 SDK is installed
cd /d "%~dp0..\windows"
if not exist "packages\Microsoft.Web.WebView2.1.0.2592.51" (
    echo WebView2 SDK not found. Installing...
    echo.
    cd /d "%~dp0"
    call setup_webview2.bat
    if %errorlevel% neq 0 (
        echo.
        echo ERROR: Failed to install WebView2 SDK!
        pause
        exit /b 1
    )
)

cd /d "%~dp0..\example"

echo.
echo Building Debug version...
flutter build windows --debug

if %errorlevel% == 0 (
    echo.
    echo ================================
    echo  Build successful!
    echo ================================
    echo Starting application...
    start build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
    echo.
    echo Application started!
) else (
    echo.
    echo ================================
    echo  Build failed!
    echo ================================
    echo Check the error messages above.
    pause
    exit /b 1
)
