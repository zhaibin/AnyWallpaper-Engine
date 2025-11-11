@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul 2>&1

echo ========================================
echo Clear AnyWP Storage Data
echo ========================================
echo.

REM AnyWP Engine storage location
set "STORAGE_PATH=%LOCALAPPDATA%\AnyWPEngine"

echo Storage location: %STORAGE_PATH%
echo.

if exist "%STORAGE_PATH%" (
    echo Found existing storage data
    dir "%STORAGE_PATH%" /s
    echo.
    echo This will DELETE all saved positions and states.
    pause
    
    echo Deleting...
    rd /s /q "%STORAGE_PATH%"
    
    if not exist "%STORAGE_PATH%" (
        echo [OK] Storage cleared successfully
    ) else (
        echo [ERROR] Failed to clear storage
    )
) else (
    echo [INFO] No storage data found
)

echo.
pause

