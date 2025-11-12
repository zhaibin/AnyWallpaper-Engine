@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Verify Precompiled Package
REM Verify that all required files exist
REM ==========================================

if "%1"=="" (
    echo Usage: verify_precompiled.bat VERSION
    echo Example: verify_precompiled.bat 2.1.0
    exit /b 1
)

set VERSION=%1
set PROJECT_ROOT=%~dp0..
set RELEASE_DIR=%PROJECT_ROOT%\release
set PRECOMPILED_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_precompiled
set SOURCE_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_source

echo ========================================
echo  AnyWP Engine - Package Verification
echo  Version: %VERSION%
echo ========================================
echo.

set ERROR_COUNT=0

REM ==========================================
REM Verify Precompiled Package
REM ==========================================

echo [1/2] Verifying precompiled package...
echo.

if not exist "%PRECOMPILED_DIR%" (
    echo ERROR: Precompiled directory does not exist
    set /a ERROR_COUNT+=1
    goto verify_source
)

REM Check DLL files
echo Checking DLL files...
if not exist "%PRECOMPILED_DIR%\bin\anywp_engine_plugin.dll" (
    echo   [MISSING] bin\anywp_engine_plugin.dll
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] bin\anywp_engine_plugin.dll
)

if not exist "%PRECOMPILED_DIR%\bin\WebView2Loader.dll" (
    echo   [MISSING] bin\WebView2Loader.dll
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] bin\WebView2Loader.dll
)

REM Check LIB file
echo Checking LIB file...
if not exist "%PRECOMPILED_DIR%\lib\anywp_engine_plugin.lib" (
    echo   [MISSING] lib\anywp_engine_plugin.lib
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] lib\anywp_engine_plugin.lib
)

REM Check Dart API
echo Checking Dart API...
if not exist "%PRECOMPILED_DIR%\lib\dart\anywp_engine.dart" (
    echo   [MISSING] lib\dart\anywp_engine.dart
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] lib\dart\anywp_engine.dart
)

REM Check C API header
echo Checking C API header...
if not exist "%PRECOMPILED_DIR%\include\anywp_engine\anywp_engine_plugin_c_api.h" (
    echo   [MISSING] include\anywp_engine\anywp_engine_plugin_c_api.h
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] include\anywp_engine\anywp_engine_plugin_c_api.h
)

REM Check CMakeLists.txt
echo Checking CMake files...
if not exist "%PRECOMPILED_DIR%\windows\CMakeLists.txt" (
    echo   [MISSING] windows\CMakeLists.txt
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\CMakeLists.txt
)

REM Check documentation
echo Checking documentation...
if not exist "%PRECOMPILED_DIR%\README.md" (
    echo   [MISSING] README.md
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] README.md
)

if not exist "%PRECOMPILED_DIR%\INTEGRATION_GUIDE.md" (
    echo   [MISSING] INTEGRATION_GUIDE.md
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] INTEGRATION_GUIDE.md
)

if not exist "%PRECOMPILED_DIR%\LICENSE" (
    echo   [MISSING] LICENSE
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] LICENSE
)

REM Check ZIP
echo Checking ZIP package...
if not exist "%PRECOMPILED_DIR%.zip" (
    echo   [MISSING] anywp_engine_v%VERSION%_precompiled.zip
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] anywp_engine_v%VERSION%_precompiled.zip
)

:verify_source

REM ==========================================
REM Verify Source Package
REM ==========================================

echo.
echo [2/2] Verifying source package...
echo.

if not exist "%SOURCE_DIR%" (
    echo ERROR: Source directory does not exist
    set /a ERROR_COUNT+=1
    goto summary
)

REM Check binaries
echo Checking binaries...
if not exist "%SOURCE_DIR%\bin\anywp_engine_plugin.dll" (
    echo   [MISSING] bin\anywp_engine_plugin.dll
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] bin\anywp_engine_plugin.dll
)

if not exist "%SOURCE_DIR%\lib\anywp_engine_plugin.lib" (
    echo   [MISSING] lib\anywp_engine_plugin.lib
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] lib\anywp_engine_plugin.lib
)

REM Check C++ source
echo Checking C++ source...
if not exist "%SOURCE_DIR%\windows\anywp_engine_plugin.cpp" (
    echo   [MISSING] windows\anywp_engine_plugin.cpp
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\anywp_engine_plugin.cpp
)

if not exist "%SOURCE_DIR%\windows\anywp_engine_plugin.h" (
    echo   [MISSING] windows\anywp_engine_plugin.h
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\anywp_engine_plugin.h
)

if not exist "%SOURCE_DIR%\windows\anywp_engine_plugin_c_api.h" (
    echo   [MISSING] windows\anywp_engine_plugin_c_api.h
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\anywp_engine_plugin_c_api.h
)

REM Check modules directory
echo Checking modules...
if not exist "%SOURCE_DIR%\windows\modules" (
    echo   [MISSING] windows\modules\
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\modules\
)

REM Check utils directory
echo Checking utils...
if not exist "%SOURCE_DIR%\windows\utils" (
    echo   [MISSING] windows\utils\
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\utils\
)

REM Check SDK
echo Checking SDK...
if not exist "%SOURCE_DIR%\windows\anywp_sdk.js" (
    echo   [MISSING] windows\anywp_sdk.js
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\anywp_sdk.js
)

if not exist "%SOURCE_DIR%\windows\sdk" (
    echo   [MISSING] windows\sdk\
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\sdk\
)

REM Check WebView2 packages
echo Checking WebView2 packages...
if not exist "%SOURCE_DIR%\windows\packages" (
    echo   [MISSING] windows\packages\
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] windows\packages\
)

REM Check ZIP
echo Checking ZIP package...
if not exist "%SOURCE_DIR%.zip" (
    echo   [MISSING] anywp_engine_v%VERSION%_source.zip
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] anywp_engine_v%VERSION%_source.zip
)

:summary

REM ==========================================
REM Summary
REM ==========================================

echo.
echo ========================================
echo  Verification Complete
echo ========================================
echo.

if %ERROR_COUNT% EQU 0 (
    echo Status: PASSED
    echo All required files are present.
    echo.
    echo Ready for GitHub Release:
    echo   - anywp_engine_v%VERSION%_precompiled.zip
    echo   - anywp_engine_v%VERSION%_source.zip
    exit /b 0
) else (
    echo Status: FAILED
    echo Missing files: %ERROR_COUNT%
    echo.
    echo Please run release.bat again to rebuild packages.
    exit /b 1
)

