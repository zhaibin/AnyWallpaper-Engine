@echo off
REM Test SDK injection for a specific URL
REM Usage: test_sdk_injection.bat [URL]

setlocal enabledelayedexpansion

set TEST_URL=%1
if "%TEST_URL%"=="" (
    echo Usage: test_sdk_injection.bat [URL]
    echo Example: test_sdk_injection.bat https://hkcw-web.pages.dev/wallpapers/theme1
    exit /b 1
)

echo ========================================
echo SDK Injection Test
echo ========================================
echo.
echo Testing URL: %TEST_URL%
echo.

REM Check if SDK file exists
if exist "windows\anywp_sdk.js" (
    echo [OK] SDK file found: windows\anywp_sdk.js
    for %%A in ("windows\anywp_sdk.js") do echo      Size: %%~zA bytes
) else (
    echo [ERROR] SDK file NOT found: windows\anywp_sdk.js
    echo.
    echo Please build the SDK first:
    echo   .\scripts\build_sdk.bat
    exit /b 1
)

echo.
echo Building and running application...
echo.

REM Build the application
cd example
call flutter clean >nul 2>&1
call flutter pub get >nul 2>&1
call flutter build windows --debug > ..\test_logs\sdk_injection_build.log 2>&1

if errorlevel 1 (
    echo [ERROR] Build failed! Check test_logs\sdk_injection_build.log
    exit /b 1
)

echo [OK] Build successful
echo.
echo Starting application...
echo.
echo IMPORTANT: After the app starts:
echo   1. Enter URL: %TEST_URL%
echo   2. Click "Start" button
echo   3. Check the console output for SDK injection logs
echo   4. Look for these log messages:
echo      - "[AnyWP] [SDKBridge] SDK loaded from: ..."
echo      - "[AnyWP] [SDKBridge] SDK registered for future pages"
echo      - "[AnyWP] [SDKBridge] SDK verification: SDK loaded successfully!"
echo      - "[AnyWP] [SDKBridge] SDK version: ..."
echo.
echo Logs will be saved to: test_logs\sdk_injection_*.log
echo.

REM Run the application
call flutter run -d windows > ..\test_logs\sdk_injection_run.log 2>&1

cd ..

echo.
echo ========================================
echo Test completed
echo ========================================
echo.
echo Check logs in: test_logs\sdk_injection_*.log
echo.

endlocal

