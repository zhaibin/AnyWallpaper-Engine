@echo off
REM ============================================================
REM Purpose: Install WebView2 SDK (required for first time)
REM Function: Download NuGet -> Install Microsoft.Web.WebView2 package
REM Location: windows/packages/Microsoft.Web.WebView2.1.0.2592.51
REM Usage: First time development or when SDK is missing
REM ============================================================

echo ================================
echo  AnyWP - WebView2 SDK Setup
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
