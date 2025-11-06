@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

echo ==============================================
echo  AnyWP Engine v__VERSION__ 预编译包安装助手
echo ==============================================
echo.

REM 检查是否在 Flutter 项目根目录执行
if not exist "pubspec.yaml" (
    echo ❌ 错误：请在 Flutter 项目根目录运行此脚本
    echo    例如：flutter_project\packages\anywp_engine_v__VERSION__\setup_precompiled.bat
    echo.
    pause
    exit /b 1
)

REM 解析目标 packages 目录
set "PACKAGES_DIR=%cd%\packages"
if not exist "%PACKAGES_DIR%" (
    echo [1/4] 创建 packages 目录...
    mkdir "%PACKAGES_DIR%" >nul 2>&1
    if errorlevel 1 (
        echo ❌ 错误：无法创建 packages 目录：%PACKAGES_DIR%
        pause
        exit /b 1
    )
)

REM 定位预编译包目录（脚本所在位置）
set "SOURCE_DIR=%~dp0"
set "SOURCE_DIR=%SOURCE_DIR:~0,-1%"
set "TARGET_DIR=%PACKAGES_DIR%\anywp_engine_v__VERSION__"

echo [2/4] 同步预编译包到 packages/anywp_engine_v__VERSION__ ...
if exist "%TARGET_DIR%" (
    rmdir /s /q "%TARGET_DIR%" >nul 2>&1
)
mkdir "%TARGET_DIR%" >nul 2>&1
if errorlevel 1 (
    echo ❌ 错误：无法创建目标目录：%TARGET_DIR%
    pause
    exit /b 1
)

robocopy "%SOURCE_DIR%" "%TARGET_DIR%" /E /NFL /NDL /NJH /NJS /NC /NS /XD ".git" >nul
if errorlevel 8 (
    echo ❌ 错误：复制预编译包失败
    pause
    exit /b 1
)

echo [3/4] 验证关键文件...
set "ERROR_COUNT=0"

for %%F in ("
    bin\anywp_engine_plugin.dll"
    "bin\WebView2Loader.dll"
    "lib\anywp_engine_plugin.lib"
    "lib\anywp_engine.dart"
    "lib\dart\anywp_engine.dart"
    "windows\anywp_sdk.js"
    "windows\CMakeLists.txt"
) do (
    if not exist "%TARGET_DIR%%%F" (
        echo    ❌ 缺少文件：%TARGET_DIR%%%F
        set /a ERROR_COUNT+=1
    ) else (
        echo    ✅ %TARGET_DIR%%%F
    )
)

if %ERROR_COUNT% NEQ 0 (
    echo ❌ 验证失败：共有 %ERROR_COUNT% 个关键文件缺失
    echo    请重新解压预编译包后再试
    pause
    exit /b 1
)

echo [4/4] 更新依赖（flutter pub get）...
flutter pub get
if errorlevel 1 (
    echo ❌ 错误：flutter pub get 执行失败
    pause
    exit /b 1
)

echo.
echo ==============================================
echo ✅ AnyWP Engine v__VERSION__ 预编译包已安装完成
echo ==============================================
echo.
echo 使用方法：
echo 1. 打开 pubspec.yaml，添加依赖：
echo.
echo    dependencies:
echo      anywp_engine:
echo        path: ./packages/anywp_engine_v__VERSION__
echo.
echo 2. 重新运行 flutter build windows --release
echo.
pause
endlocal

