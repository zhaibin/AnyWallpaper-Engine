@echo off
chcp 65001 >nul 2>&1
setlocal

set "PACKAGE_DIR=%~dp0"
set "PACKAGE_DIR=%PACKAGE_DIR:~0,-1%"

echo ==============================================
echo  AnyWP Engine v__VERSION__ 预编译包完整性验证
echo ==============================================
echo 目录：%PACKAGE_DIR%
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
    echo ✅ 验证通过：所有关键文件齐全
) else (
    echo.
    echo ❌ 验证失败：共有 %ERROR% 个关键文件缺失
)

echo.
pause
exit /b %ERROR%

:CheckFile
set "REL_PATH=%~1"
if exist "%PACKAGE_DIR%\%REL_PATH%" (
    echo ✅ %REL_PATH%
) else (
    echo ❌ %REL_PATH%
    set /a ERROR+=1
)
exit /b 0

