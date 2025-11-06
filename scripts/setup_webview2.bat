@echo off
chcp 65001 >nul 2>&1
REM ============================================================
REM 用途：安装 WebView2 SDK（首次必须运行）
REM 功能：下载 NuGet → 安装 Microsoft.Web.WebView2 包
REM 位置：windows/packages/Microsoft.Web.WebView2.1.0.2592.51
REM 适用：首次开发或 SDK 丢失时
REM ============================================================

echo ================================
echo  AnyWP - WebView2 SDK 安装
echo ================================
echo.

cd /d "%~dp0..\windows"

REM Check if WebView2 package already exists
if exist "packages\Microsoft.Web.WebView2.1.0.2592.51" (
    echo WebView2 SDK is already installed.
    echo.
    goto :end
)

REM Download NuGet if not exists
if not exist nuget.exe (
    echo Downloading NuGet...
    powershell -Command "try { Invoke-WebRequest -Uri 'https://dist.nuget.org/win-x86-commandline/latest/nuget.exe' -OutFile 'nuget.exe' -ErrorAction Stop; Write-Host 'NuGet downloaded successfully.' } catch { Write-Host 'Failed to download NuGet:' $_.Exception.Message; exit 1 }"
    if %errorlevel% neq 0 (
        echo ERROR: Failed to download NuGet!
        goto :error
    )
)

echo Installing WebView2 package...
nuget.exe install Microsoft.Web.WebView2 -Version 1.0.2592.51 -OutputDirectory packages
if %errorlevel% neq 0 (
    echo ERROR: Failed to install WebView2 package!
    goto :error
)

echo.
echo ================================
echo  WebView2 SDK installed successfully!
echo ================================
goto :end

:error
echo.
echo ================================
echo  Setup failed! Please check the errors above.
echo ================================
cd ..
pause
exit /b 1

:end
cd ..
echo.
echo You can now build the project.
echo.
