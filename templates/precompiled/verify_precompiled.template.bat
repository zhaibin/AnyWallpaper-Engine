@echo off
setlocal

set "PACKAGE_DIR=%~dp0"
set "PACKAGE_DIR=%PACKAGE_DIR:~0,-1%"

echo ==============================================
echo  AnyWP Engine v__VERSION__ Precompiled Package Verification
echo ==============================================
echo Directory: %PACKAGE_DIR%
echo.

set "ERROR=0"

call :CheckFile "bin\anywp_engine_plugin.dll"
call :CheckFile "bin\WebView2Loader.dll"
call :CheckFile "lib\anywp_engine_plugin.lib"
call :CheckFile "lib\anywp_engine.dart"
call :CheckFile "lib\dart\anywp_engine.dart"
call :CheckFile "windows\anywp_sdk.js"
call :CheckFile "windows\CMakeLists.txt"
call :CheckFile "windows\src\anywp_engine_plugin.cpp"

if %ERROR%==0 (
    echo.
    echo Verification passed: All critical files present
) else (
    echo.
    echo Verification failed: %ERROR% critical file(s) missing
)

echo.
pause
exit /b %ERROR%

:CheckFile
set "REL_PATH=%~1"
if exist "%PACKAGE_DIR%\%REL_PATH%" (
    echo OK: %REL_PATH%
) else (
    echo ERROR: %REL_PATH%
    set /a ERROR+=1
)
exit /b 0

