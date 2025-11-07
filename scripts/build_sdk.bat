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

echo Building...
call npm run build
if errorlevel 1 (
    echo ERROR: Build failed
    cd ..\..
    exit /b 1
)

cd ..\..

echo.
echo ========================================
echo Web SDK built successfully!
echo Output: windows\anywp_sdk.js
echo ========================================

endlocal

