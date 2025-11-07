@echo off
setlocal enabledelayedexpansion

echo ==============================================
echo  AnyWP Engine v__VERSION__ Precompiled Package Installer
echo ==============================================
echo.

REM Check if running from Flutter project root directory
if not exist "pubspec.yaml" (
    echo ERROR: Please run this script from Flutter project root directory
    echo    Example: flutter_project\packages\anywp_engine_v__VERSION__\setup_precompiled.bat
    echo.
    pause
    exit /b 1
)

REM Parse target packages directory
set "PACKAGES_DIR=%cd%\packages"
if not exist "%PACKAGES_DIR%" (
    echo [1/4] Creating packages directory...
    mkdir "%PACKAGES_DIR%" >nul 2>&1
    if errorlevel 1 (
        echo ERROR: Cannot create packages directory: %PACKAGES_DIR%
        pause
        exit /b 1
    )
)

REM Locate precompiled package directory (where script is located)
set "SOURCE_DIR=%~dp0"
set "SOURCE_DIR=%SOURCE_DIR:~0,-1%"
set "TARGET_DIR=%PACKAGES_DIR%\anywp_engine_v__VERSION__"

echo [2/4] Syncing precompiled package to packages/anywp_engine_v__VERSION__ ...
if exist "%TARGET_DIR%" (
    rmdir /s /q "%TARGET_DIR%" >nul 2>&1
)
mkdir "%TARGET_DIR%" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Cannot create target directory: %TARGET_DIR%
    pause
    exit /b 1
)

robocopy "%SOURCE_DIR%" "%TARGET_DIR%" /E /NFL /NDL /NJH /NJS /NC /NS /XD ".git" >nul
if errorlevel 8 (
    echo ERROR: Failed to copy precompiled package
    pause
    exit /b 1
)

echo [3/4] Verifying critical files...
set "ERROR_COUNT=0"

for %%F in ("
    bin\anywp_engine_plugin.dll"
    "bin\WebView2Loader.dll"
    "lib\anywp_engine_plugin.lib"
    "lib\anywp_engine.dart"
    "lib\dart\anywp_engine.dart"
    "windows\anywp_sdk.js"
    "windows\CMakeLists.txt"
    "windows\src\anywp_engine_plugin.cpp"
) do (
    if not exist "%TARGET_DIR%%%F" (
        echo    ERROR: Missing file: %TARGET_DIR%%%F
        set /a ERROR_COUNT+=1
    ) else (
        echo    OK: %TARGET_DIR%%%F
    )
)

if %ERROR_COUNT% NEQ 0 (
    echo ERROR: Verification failed: %ERROR_COUNT% critical file(s) missing
    echo    Please re-extract the precompiled package and try again
    pause
    exit /b 1
)

echo [4/4] Updating dependencies (flutter pub get)...
flutter pub get
if errorlevel 1 (
    echo ERROR: flutter pub get failed
    pause
    exit /b 1
)

echo.
echo ==============================================
echo AnyWP Engine v__VERSION__ precompiled package installed successfully
echo ==============================================
echo.
echo Usage:
echo 1. Open pubspec.yaml and add dependency:
echo.
echo    dependencies:
echo      anywp_engine:
echo        path: ./packages/anywp_engine_v__VERSION__
echo.
echo 2. Run flutter build windows --release again
echo.
pause
endlocal

