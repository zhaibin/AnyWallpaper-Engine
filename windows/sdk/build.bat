@echo off
REM AnyWP SDK Build Script
REM Builds the modular SDK into a single anywp_sdk.js file

setlocal enabledelayedexpansion

echo ========================================
echo AnyWP SDK Build Script
echo ========================================
echo.

REM Check if node_modules exists
if not exist "node_modules\" (
    echo [1/3] Installing dependencies...
    call npm install
    if errorlevel 1 (
        echo ERROR: Failed to install dependencies
        exit /b 1
    )
    echo.
) else (
    echo [1/3] Dependencies already installed
    echo.
)

REM Build the SDK
echo [2/3] Building SDK...
call npm run build
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)
echo.

REM Verify output
if exist "..\anywp_sdk.js" (
    echo [3/3] Build successful!
    for %%A in ("..\anywp_sdk.js") do (
        echo   Output: windows\anywp_sdk.js
        echo   Size: %%~zA bytes
        echo   Time: %%~tA
    )
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
) else (
    echo ERROR: Output file not found
    exit /b 1
)

endlocal

