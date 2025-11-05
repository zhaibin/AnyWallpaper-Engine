@echo off
REM ============================================================
REM AnyWP Engine - 构建 Release 版本 v2.0
REM 用于生成可发布的预编译 DLL 包
REM ============================================================

setlocal enabledelayedexpansion

echo ============================================
echo AnyWP Engine - 构建 Release 版本 v2.0
echo ============================================
echo.

REM 检查是否在正确的目录
if not exist "scripts\build_release_v2.bat" (
    echo [错误] 请在项目根目录运行此脚本
    echo.
    pause
    exit /b 1
)

REM 设置变量
set "EXAMPLE_DIR=%cd%\example"
set "BUILD_DIR=%EXAMPLE_DIR%\build\windows\x64"
set "RELEASE_DIR=%cd%\release"
set "VERSION=1.1.0"
set "RELEASE_NAME=anywp_engine_v%VERSION%"
set "ERROR_COUNT=0"

echo [1/12] 清理旧的构建...
if exist "%EXAMPLE_DIR%\build" (
    rmdir /s /q "%EXAMPLE_DIR%\build" 2>nul
    if errorlevel 1 (
        echo [警告] 无法完全清理 build 目录
    )
)

echo [2/12] 运行 flutter clean...
cd "%EXAMPLE_DIR%"
flutter clean >nul 2>&1
if errorlevel 1 (
    echo [错误] flutter clean 失败
    set /a ERROR_COUNT+=1
)

echo [3/12] 获取依赖...
flutter pub get >nul 2>&1
if errorlevel 1 (
    echo [错误] flutter pub get 失败
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

echo [4/12] 构建 Release 版本...
echo       这可能需要几分钟，请耐心等待...
flutter build windows --release
if errorlevel 1 (
    echo [错误] Release 构建失败
    cd ..
    pause
    exit /b 1
)

REM 检查构建产物
if not exist "%BUILD_DIR%\runner\Release\anywallpaper_engine_example.exe" (
    echo [错误] 找不到构建产物
    cd ..
    pause
    exit /b 1
)

echo [5/12] 创建 Release 目录结构...
cd ..
if exist "%RELEASE_DIR%" (
    rmdir /s /q "%RELEASE_DIR%" 2>nul
)
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\bin" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\sdk" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\windows" 2>nul

echo [6/12] 复制 DLL 和相关文件...
REM 插件 DLL
copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制插件 DLL
    set /a ERROR_COUNT+=1
)

copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [警告] 无法复制插件 LIB
)

REM WebView2Loader DLL
copy "windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 WebView2Loader.dll
    set /a ERROR_COUNT+=1
)

echo [7/12] 复制 Dart 源代码...
REM Dart 库 - 直接复制到 lib/ 而不是 lib/dart/
copy "lib\anywp_engine.dart" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 Dart 源代码
    set /a ERROR_COUNT+=1
)

echo [8/12] 创建简化的头文件...
REM 创建简化的头文件（不依赖 WebView2 SDK）
(
echo // Precompiled plugin stub header
echo // This file is required by Flutter's plugin registration system
echo.
echo #ifndef FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_
echo #define FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_
echo.
echo #include ^<flutter/plugin_registrar_windows.h^>
echo.
echo #ifdef FLUTTER_PLUGIN_IMPL
echo #define FLUTTER_PLUGIN_EXPORT __declspec^(dllexport^)
echo #else
echo #define FLUTTER_PLUGIN_EXPORT __declspec^(dllimport^)
echo #endif
echo.
echo #if defined^(__cplusplus^)
echo extern "C" {
echo #endif
echo.
echo FLUTTER_PLUGIN_EXPORT void AnyWPEnginePluginRegisterWithRegistrar^(
echo     FlutterDesktopPluginRegistrarRef registrar^);
echo.
echo #if defined^(__cplusplus^)
echo }  // extern "C"
echo #endif
echo.
echo #endif  // FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_
) > "%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine\any_w_p_engine_plugin.h"

echo [9/12] 复制 SDK 文件...
copy "windows\anywp_sdk.js" "%RELEASE_DIR%\%RELEASE_NAME%\sdk\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 SDK 文件
    set /a ERROR_COUNT+=1
)

echo [10/12] 创建 CMakeLists.txt...
(
echo cmake_minimum_required^(VERSION 3.14^)
echo set^(PROJECT_NAME "anywp_engine"^)
echo project^(${PROJECT_NAME} LANGUAGES CXX^)
echo.
echo # This value is used when generating builds using this plugin
echo set^(PLUGIN_NAME "anywp_engine_plugin"^)
echo.
echo # Use precompiled library
echo set^(PRECOMPILED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."^)
echo.
echo # Create an IMPORTED library that points to the precompiled DLL
echo add_library^(${PLUGIN_NAME} SHARED IMPORTED GLOBAL^)
echo.
echo # Set the DLL location
echo set_target_properties^(${PLUGIN_NAME} PROPERTIES
echo   IMPORTED_LOCATION "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
echo   IMPORTED_IMPLIB "${PRECOMPILED_DIR}/lib/anywp_engine_plugin.lib"
echo ^)
echo.
echo # Set include directories
echo target_include_directories^(${PLUGIN_NAME} INTERFACE
echo   "${PRECOMPILED_DIR}/include"
echo ^)
echo.
echo # List of absolute paths to libraries that should be bundled
echo set^(anywp_engine_bundled_libraries
echo   "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
echo   "${PRECOMPILED_DIR}/bin/WebView2Loader.dll"
echo   PARENT_SCOPE
echo ^)
) > "%RELEASE_DIR%\%RELEASE_NAME%\windows\CMakeLists.txt"

echo [11/12] 复制文档...
copy "README.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "LICENSE" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "CHANGELOG_CN.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1

REM 创建快速开始文档
(
echo # AnyWP Engine v%VERSION% - 预编译版本
echo.
echo ## 📦 包含内容
echo.
echo - `bin/` - 预编译的 DLL 文件
echo - `lib/` - Dart 源代码
echo - `include/` - C++ 头文件
echo - `sdk/` - JavaScript SDK
echo - `windows/` - CMake 配置
echo.
echo ## 🚀 快速集成
echo.
echo ### 1. 在你的 Flutter 项目 `pubspec.yaml` 中添加：
echo ```yaml
echo dependencies:
echo   anywp_engine:
echo     path: ./anywp_engine_v%VERSION%
echo ```
echo.
echo ### 2. 获取依赖并构建
echo ```bash
echo flutter pub get
echo flutter build windows
echo ```
echo.
echo ### 3. 开始使用：
echo ```dart
echo import 'package:anywp_engine/anywp_engine.dart';
echo.
echo await AnyWPEngine.initializeWallpaper^(url: 'https://example.com'^);
echo ```
echo.
echo ## 📚 完整文档
echo.
echo 请参阅 README.md 和 CHANGELOG_CN.md
echo.
echo 或访问：https://github.com/zhaibin/AnyWallpaper-Engine
) > "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"

REM 创建 pubspec.yaml（注意：移除 dartPluginClass）
(
echo name: anywp_engine
echo description: AnyWP - Flutter plugin for WebView2 desktop wallpaper engine with power saving and instant resume ^(Precompiled^)
echo version: %VERSION%
echo homepage: https://github.com/zhaibin/AnyWallpaper-Engine
echo.
echo environment:
echo   sdk: '^^>=3.0.0 ^<4.0.0'
echo   flutter: "^^>=3.0.0"
echo.
echo dependencies:
echo   flutter:
echo     sdk: flutter
echo   plugin_platform_interface: ^^2.0.0
echo.
echo flutter:
echo   plugin:
echo     platforms:
echo       windows:
echo         pluginClass: AnyWPEnginePlugin
) > "%RELEASE_DIR%\%RELEASE_NAME%\pubspec.yaml"

echo [12/12] 打包 ZIP...
cd "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path '%RELEASE_NAME%' -DestinationPath '%RELEASE_NAME%.zip' -Force" 2>nul
if errorlevel 1 (
    echo [错误] 无法创建 ZIP 文件
    set /a ERROR_COUNT+=1
) else (
    REM 获取文件大小
    for %%I in ("%RELEASE_NAME%.zip") do set "FILE_SIZE=%%~zI"
    set /a FILE_SIZE_KB=!FILE_SIZE! / 1024
)

cd ..

echo.
echo ============================================
if !ERROR_COUNT! EQU 0 (
    echo ✅ 构建完成！
) else (
    echo ⚠️ 构建完成但有 !ERROR_COUNT! 个警告/错误
)
echo ============================================
echo.
echo 📦 Release 包位置：
echo    %RELEASE_DIR%\%RELEASE_NAME%.zip
if defined FILE_SIZE_KB (
    echo    大小：!FILE_SIZE_KB! KB
)
echo.
echo 📁 解压后的文件位置：
echo    %RELEASE_DIR%\%RELEASE_NAME%\
echo.
echo 📝 下一步：
echo    1. 测试预编译包是否可用
echo    2. 访问 https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
echo    3. 选择标签 v%VERSION%
echo    4. 上传 %RELEASE_NAME%.zip
echo    5. 复制 release\GITHUB_RELEASE_NOTES_v%VERSION%.md 的内容作为说明
echo.

if !ERROR_COUNT! GTR 0 (
    echo ⚠️ 请检查上述错误后再发布
    echo.
)

pause
endlocal

