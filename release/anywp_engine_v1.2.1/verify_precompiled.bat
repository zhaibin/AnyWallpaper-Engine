@echo off
chcp 65001 >nul 2>&1
setlocal

set "PACKAGE_DIR=%~dp0"
set "PACKAGE_DIR=%PACKAGE_DIR:~0,-1%"

echo ==============================================
echo  AnyWP Engine v1.2.1 棰勭紪璇戝寘瀹屾暣鎬ч獙璇?
echo ==============================================
echo 鐩綍锛?PACKAGE_DIR%
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
    echo 鉁?楠岃瘉閫氳繃锛氭墍鏈夊叧閿枃浠堕綈鍏?
) else (
    echo.
    echo 鉂?楠岃瘉澶辫触锛氬叡鏈?%ERROR% 涓叧閿枃浠剁己澶?
)

echo.
pause
exit /b %ERROR%

:CheckFile
set "REL_PATH=%~1"
if exist "%PACKAGE_DIR%\%REL_PATH%" (
    echo 鉁?%REL_PATH%
) else (
    echo 鉂?%REL_PATH%
    set /a ERROR+=1
)
exit /b 0

