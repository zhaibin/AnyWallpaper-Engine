@echo off
REM Build Web SDK from modular source
REM This script should be run from project root

setlocal enabledelayedexpansion

echo ========================================
echo Building AnyWP Web SDK
echo ========================================
echo.

cd windows\sdk

if not exist "node_modules\" (
    echo Installing dependencies...
    call npm install
    if errorlevel 1 (
        echo ERROR: Failed to install dependencies
        cd ..\..
        exit /b 1
    )
)

REM Check if production mode is requested
set PRODUCTION_MODE=%1
if "%PRODUCTION_MODE%"=="production" (
    echo Building in PRODUCTION mode - minified + unminified
    call npm run build:production
    if errorlevel 1 (
        echo ERROR: Production build failed
        cd ..\..
        exit /b 1
    )
    echo.
    echo ========================================
    echo Web SDK built successfully - PRODUCTION mode
    echo Output: windows\anywp_sdk.js - unminified
    echo Output: windows\anywp_sdk.min.js - minified
    echo ========================================
) else (
    echo Building in DEVELOPMENT mode - unminified only
    call npm run build
    if errorlevel 1 (
        echo ERROR: Build failed
        cd ..\..
        exit /b 1
    )
    echo.
    echo ========================================
    echo Web SDK built successfully!
    echo Output: windows\anywp_sdk.js
    echo Note: Use 'build_sdk.bat production' to generate minified version
    echo ========================================
)

cd ..\..

endlocal


