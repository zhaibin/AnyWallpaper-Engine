@echo off
chcp 65001 >nul 2>&1
REM ============================================================
REM 用途：构建可发布的预编译 DLL 包（发版必用）
REM 功能：编译 Release → 打包 DLL/头文件/文档 → 生成 ZIP
REM 产物：release/anywp_engine_v{版本号}.zip
REM 适用：准备发布新版本时使用
REM ============================================================

setlocal enabledelayedexpansion

echo ============================================
echo AnyWP Engine - 构建 Release 包 v2.0
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
set "TEMPLATE_DIR=%cd%\templates"
set "VERSION=1.2.1"
set "RELEASE_NAME=anywp_engine_v%VERSION%"
set "ERROR_COUNT=0"

echo [1/16] 清理旧的构建...
if exist "%EXAMPLE_DIR%\build" (
    rmdir /s /q "%EXAMPLE_DIR%\build" 2>nul
    if errorlevel 1 (
        echo [警告] 无法完全清理 build 目录
    )
)

echo [2/16] 运行 flutter clean...
cd "%EXAMPLE_DIR%"
flutter clean >nul 2>&1
if errorlevel 1 (
    echo [错误] flutter clean 失败
    set /a ERROR_COUNT+=1
)

echo [3/16] 获取依赖...
flutter pub get >nul 2>&1
if errorlevel 1 (
    echo [错误] flutter pub get 失败
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

echo [4/16] 构建 Release 版本...
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

echo [5/16] 创建 Release 目录结构...
cd ..
if not exist "%RELEASE_DIR%" (
    mkdir "%RELEASE_DIR%" 2>nul
)
if exist "%RELEASE_DIR%\%RELEASE_NAME%" (
    rmdir /s /q "%RELEASE_DIR%\%RELEASE_NAME%" 2>nul
    if errorlevel 1 (
        echo [错误] 无法清理旧的 Release 目录
        pause
        exit /b 1
    )
)
mkdir "%RELEASE_DIR%\%RELEASE_NAME%" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\bin" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\include" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\windows" 2>nul
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\windows\src" 2>nul

echo [6/16] 复制 DLL 和相关文件...
REM 插件 DLL
copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制插件 DLL
    set /a ERROR_COUNT+=1
)

copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制插件 LIB - 这是链接器必需的文件
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

REM WebView2Loader DLL
copy "windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 WebView2Loader.dll
    set /a ERROR_COUNT+=1
)

echo [7/16] 复制 Dart 源代码...
REM Dart 库 - 直接复制到 lib/ （标准位置）
copy "lib\anywp_engine.dart" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 Dart 源代码到 lib/
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

REM 同时复制到 lib/dart/ （向后兼容）
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart" 2>nul
copy "lib\anywp_engine.dart" "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart\" >nul 2>&1

echo [8/16] 复制 C++ 头文件...
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine" 2>nul
powershell -Command "Copy-Item -Path 'windows\include\anywp_engine\*' -Destination '%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine' -Recurse -Force" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 C++ 头文件
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

echo [9/16] 同步原生源文件和 SDK...
copy "windows\anywp_sdk.js" "%RELEASE_DIR%\%RELEASE_NAME%\windows\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 SDK 文件 (windows\\anywp_sdk.js)
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

copy "windows\anywp_engine_plugin.cpp" "%RELEASE_DIR%\%RELEASE_NAME%\windows\src\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 C++ 源文件 anywp_engine_plugin.cpp
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

copy "windows\anywp_engine_plugin.h" "%RELEASE_DIR%\%RELEASE_NAME%\windows\src\" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制 C++ 头文件 anywp_engine_plugin.h
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

if exist "windows\packages" (
    powershell -Command "Copy-Item -Path 'windows\packages' -Destination '%RELEASE_DIR%\%RELEASE_NAME%\windows' -Recurse -Force" >nul 2>&1
    if errorlevel 1 (
        echo [错误] 无法复制 WebView2 packages 目录
        set /a ERROR_COUNT+=1
        pause
        exit /b 1
    )
)

if exist "windows\packages.config" (
    copy "windows\packages.config" "%RELEASE_DIR%\%RELEASE_NAME%\windows\" >nul 2>&1
)

echo [10/16] 创建 CMake 配置...
(
echo cmake_minimum_required^(VERSION 3.14^)
echo project^(anywp_engine LANGUAGES CXX^)
echo.
echo set^(PLUGIN_NAME "anywp_engine_plugin"^)
echo set^(PRECOMPILED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."^)
echo set^(PRECOMPILED_DLL "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"^)
echo set^(PRECOMPILED_LIB "${PRECOMPILED_DIR}/lib/anywp_engine_plugin.lib"^)
echo set^(SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/anywp_engine_plugin.cpp"^)
echo.
echo if^(EXISTS ${PRECOMPILED_DLL} AND EXISTS ${PRECOMPILED_LIB}^)
echo   message^(STATUS "AnyWP Engine: ✅ 使用预编译 DLL (${PRECOMPILED_DLL})"^)
echo   add_library^(${PLUGIN_NAME} SHARED IMPORTED GLOBAL^)
echo   set_target_properties^(${PLUGIN_NAME} PROPERTIES
echo     IMPORTED_LOCATION "${PRECOMPILED_DLL}"
echo     IMPORTED_IMPLIB "${PRECOMPILED_LIB}"
echo   ^)
echo   target_include_directories^(${PLUGIN_NAME} INTERFACE
echo     "${PRECOMPILED_DIR}/include"
echo   ^)
echo else()
echo   message^(STATUS "AnyWP Engine: 🔧 未找到预编译 DLL，转为源码构建"^)
echo   if^(NOT EXISTS ${SOURCE_FILE}^)
echo     message^(FATAL_ERROR "AnyWP Engine: 源码文件不存在: ${SOURCE_FILE}"^)
echo   endif()
echo   add_library^(${PLUGIN_NAME} SHARED
echo     "${SOURCE_FILE}"
echo   ^)
echo   apply_standard_settings^(${PLUGIN_NAME}^)
echo   set_target_properties^(${PLUGIN_NAME} PROPERTIES
echo     CXX_STANDARD 17
echo     CXX_STANDARD_REQUIRED ON
echo     CXX_VISIBILITY_PRESET hidden
echo   ^)
echo   target_compile_definitions^(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL^)
echo   target_include_directories^(${PLUGIN_NAME} PRIVATE
echo     "${PRECOMPILED_DIR}/include"
echo   ^)
echo   target_include_directories^(${PLUGIN_NAME} INTERFACE
echo     "${PRECOMPILED_DIR}/include"
echo   ^)
echo   target_link_libraries^(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin^)
echo   set^(WEBVIEW2_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.2592.51"^)
echo   if^(EXISTS ${WEBVIEW2_PACKAGE_DIR}^)
echo     target_include_directories^(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/include"^)
echo     if^(CMAKE_SIZEOF_VOID_P EQUAL 8^)
echo       target_link_libraries^(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2LoaderStatic.lib"^)
echo     else()
echo       target_link_libraries^(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x86/WebView2LoaderStatic.lib"^)
echo     endif()
echo   else()
echo     message^(WARNING "AnyWP Engine: 未找到 WebView2 NuGet 包，请先执行 nuget restore"^)
echo   endif()
echo endif()
echo.
echo set^(anywp_engine_bundled_libraries
echo   "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
echo   "${PRECOMPILED_DIR}/bin/WebView2Loader.dll"
echo   PARENT_SCOPE
echo ^)
) > "%RELEASE_DIR%\%RELEASE_NAME%\windows\CMakeLists.txt"

echo [11/16] 复制文档...
copy "README.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "LICENSE" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "CHANGELOG_CN.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1

echo [12/16] 生成 PRECOMPILED_README...
(
echo # AnyWP Engine v%VERSION% - 预编译版本
echo.
echo ## 📦 包含内容
echo.
echo - `bin/` - 运行时 DLL（anywp_engine_plugin.dll）与 WebView2Loader
echo - `lib/` - Dart 源码与链接库（anywp_engine_plugin.lib）
echo - `include/` - C++ 公开头文件
echo - `windows/anywp_sdk.js` - JavaScript SDK
echo - `windows/src/` - C++ 实现源码（可选择自行编译）
echo - `windows/CMakeLists.txt` - 自动检测预编译/源码模式
echo.
echo ## 🚀 快速集成
echo.
echo ### 1. 推荐：运行安装脚本
echo ```powershell
echo .\setup_precompiled.bat
echo ```
echo.
echo ### 2. 或手动在 `pubspec.yaml` 中添加：
echo ```yaml
echo dependencies:
echo   anywp_engine:
echo     path: ./packages/anywp_engine_v%VERSION%
echo ```
echo.
echo ### 3. 安装后运行
echo ```bash
echo flutter pub get
echo flutter build windows
echo ```
echo.
echo ### 4. 启动示例
echo ```bash
echo cd example_minimal
echo flutter run -d windows
echo ```
echo.
echo ## 📚 完整文档
echo.
echo - README.md / CHANGELOG_CN.md
echo - setup_precompiled.bat（自动安装）
echo - verify_precompiled.bat（快速验证）
echo - generate_pubspec_snippet.bat（pubspec 片段）
echo.
echo 更多信息请访问：https://github.com/zhaibin/AnyWallpaper-Engine
) > "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"

echo [13/16] 生成自动化辅助脚本...
powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\setup_precompiled.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\setup_precompiled.bat'" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法生成 setup_precompiled.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\verify_precompiled.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\verify_precompiled.bat'" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法生成 verify_precompiled.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\generate_pubspec_snippet.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\generate_pubspec_snippet.bat'" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法生成 generate_pubspec_snippet.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

echo [14/16] 复制最小示例项目...
powershell -Command "Copy-Item -Path '%TEMPLATE_DIR%\example_minimal' -Destination '%RELEASE_DIR%\%RELEASE_NAME%' -Recurse -Force" >nul 2>&1
if errorlevel 1 (
    echo [错误] 无法复制示例项目
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\pubspec.yaml') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\pubspec.yaml'" >nul 2>&1
powershell -Command "(Get-Content '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\README.md') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\README.md'" >nul 2>&1

echo [15/16] 生成 pubspec.yaml...
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
echo   assets:
echo     - windows/anywp_sdk.js
) > "%RELEASE_DIR%\%RELEASE_NAME%\pubspec.yaml"

echo [16/16] 打包 ZIP...
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

if defined NO_PAUSE goto end

pause

:end
endlocal

