@echo off
REM ==========================================
REM AnyWP Engine - Setup Script
REM Install WebView2 SDK via NuGet
REM ==========================================

echo ================================
echo  AnyWP Engine - Setup
echo  Installing WebView2 SDK
echo ================================
echo.

cd /d "%~dp0..\windows"

REM Check if NuGet exists
where nuget >nul 2>&1
if errorlevel 1 (
    echo NuGet not found. Downloading...
    powershell -Command "Invoke-WebRequest -Uri https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -OutFile nuget.exe"
    if errorlevel 1 (
        echo ERROR: Failed to download NuGet
        pause
        exit /b 1
    )
    set NUGET=nuget.exe
) else (
    set NUGET=nuget
)

echo Installing Microsoft.Web.WebView2...
%NUGET% install Microsoft.Web.WebView2 -Version 1.0.2592.51 -OutputDirectory packages

if errorlevel 1 (
    echo ERROR: Failed to install WebView2 SDK
    pause
    exit /b 1
)

echo.
echo ================================
echo  Setup Complete!
echo ================================
echo WebView2 SDK installed successfully
echo.
pause

