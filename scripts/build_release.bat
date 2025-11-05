@echo off
REM ============================================================
REM AnyWP Engine - 构建 Release 版本
REM 用于生成可发布的 DLL 包
REM ============================================================

echo ============================================
echo AnyWP Engine - 构建 Release 版本
echo ============================================
echo.

REM 检查是否在正确的目录
if not exist "scripts\build_release.bat" (
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

echo [1] 清理旧的构建...
if exist "%EXAMPLE_DIR%\build" (
    rmdir /s /q "%EXAMPLE_DIR%\build"
)

echo [2] 运行 flutter clean...
cd "%EXAMPLE_DIR%"
flutter clean

echo [3] 获取依赖...
flutter pub get

echo [4] 构建 Release 版本...
flutter build windows --release

REM 检查构建是否成功
if not exist "%BUILD_DIR%\runner\Release\anywallpaper_engine_example.exe" (
    echo [错误] Release 构建失败
    cd ..
    pause
    exit /b 1
)

echo [5] 创建 Release 目录结构...
cd ..
if exist "%RELEASE_DIR%" (
    rmdir /s /q "%RELEASE_DIR%"
)
mkdir "%RELEASE_DIR%"
mkdir "%RELEASE_DIR%\%RELEASE_NAME%"
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\bin"
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\include"
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib"
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\sdk"

echo [6] 复制 DLL 和相关文件...
REM 插件 DLL
copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul
copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul

REM WebView2Loader DLL (运行时需要)
copy "windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul

REM 头文件
copy "windows\include\anywp_engine\anywp_engine_plugin.h" "%RELEASE_DIR%\%RELEASE_NAME%\include\" >nul
copy "windows\include\anywp_engine\anywp_engine_plugin_c_api.h" "%RELEASE_DIR%\%RELEASE_NAME%\include\" >nul

REM SDK 文件
copy "windows\anywp_sdk.js" "%RELEASE_DIR%\%RELEASE_NAME%\sdk\" >nul

REM Dart 库
xcopy "lib" "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart" /E /I /Q >nul

REM 文档
copy "README.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul
copy "LICENSE" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul
copy "CHANGELOG_CN.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul

echo [7] 创建集成文档...
echo # AnyWP Engine v%VERSION% - 预编译版本 > "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ## 📦 包含内容 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo - `bin/` - 预编译的 DLL 文件 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo - `lib/` - 静态库文件和 Dart 源代码 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo - `include/` - C++ 头文件 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo - `sdk/` - JavaScript SDK >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ## 🚀 快速集成 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ### 1. 下载并解压此包 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ### 2. 在你的 Flutter 项目 `pubspec.yaml` 中添加： >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ```yaml >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo dependencies: >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo   anywp_engine: >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo     path: ./anywp_engine_v%VERSION% >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ``` >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ### 3. 复制预编译的 DLL 到你的项目： >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ```bash >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo copy anywp_engine_v%VERSION%\bin\*.dll your_project\windows\plugins\anywp_engine\ >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ``` >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ### 4. 开始使用： >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ```dart >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo import 'package:anywp_engine/anywp_engine.dart'; >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo await AnyWPEngine.initializeWallpaper(url: 'https://example.com'); >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ``` >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo ## 📚 完整文档 >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo. >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"
echo 请参阅 README.md 和 CHANGELOG_CN.md >> "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"

echo [8] 创建 pubspec.yaml...
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
echo         fileName: anywp_engine_plugin.cpp
echo         dartPluginClass: AnyWPEngine
) > "%RELEASE_DIR%\%RELEASE_NAME%\pubspec.yaml"

echo [9] 打包 ZIP...
cd "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path '%RELEASE_NAME%' -DestinationPath '%RELEASE_NAME%.zip' -Force"

echo.
echo ============================================
echo ✅ 构建完成！
echo ============================================
echo.
echo 📦 Release 包位置：
echo    %RELEASE_DIR%\%RELEASE_NAME%.zip
echo.
echo 📁 解压后的文件位置：
echo    %RELEASE_DIR%\%RELEASE_NAME%\
echo.
echo 📝 发布到 GitHub：
echo    1. 访问 https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
echo    2. 上传 %RELEASE_NAME%.zip
echo    3. 填写版本号和更新说明
echo.

pause

