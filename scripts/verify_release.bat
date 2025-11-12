@echo off
setlocal enabledelayedexpansion

set VERSION=2.0.0
set BASE_PATH=E:\Projects\AnyWallpaper\AnyWallpaper-Engine\release\anywp_engine_v%VERSION%

echo [Verifying Release Package v%VERSION%]
echo.

set ERROR_COUNT=0

call :CheckFile "bin\anywp_engine_plugin.dll"
call :CheckFile "bin\WebView2Loader.dll"
call :CheckFile "lib\anywp_engine_plugin.lib"
call :CheckFile "lib\anywp_engine.dart"
call :CheckFile "lib\dart\anywp_engine.dart"
call :CheckFile "windows\anywp_engine_plugin.cpp"
call :CheckFile "windows\anywp_engine_plugin.h"
call :CheckFile "windows\anywp_sdk.js"
call :CheckFile "windows\CMakeLists.txt"
call :CheckFile "include\anywp_engine\any_w_p_engine_plugin.h"
call :CheckFile "README.md"
call :CheckFile "CHANGELOG_CN.md"
call :CheckFile "LICENSE"
call :CheckFile "pubspec.yaml"

echo.
if %ERROR_COUNT%==0 (
    echo [SUCCESS] All critical files verified - %ERROR_COUNT% errors
    exit /b 0
) else (
    echo [FAILED] Found %ERROR_COUNT% missing files
    exit /b 1
)

:CheckFile
if exist "%BASE_PATH%\%~1" (
    echo [OK] %~1
) else (
    echo [MISSING] %~1
    set /a ERROR_COUNT+=1
)
goto :eof

