@echo off
REM ============================================================
REM Purpose: Build publishable precompiled DLL package (required for release)
REM Function: Compile Release -> Package DLL/headers/docs -> Generate ZIP
REM Output: release/anywp_engine_v{version}.zip
REM Usage: Use when preparing to release a new version
REM ============================================================

setlocal enabledelayedexpansion

echo ============================================
echo AnyWP Engine - Build Release Package v2.1
echo ============================================
echo.
echo This script will:
echo   - Build Flutter Release version
echo   - Package DLL + Headers + Docs
echo   - Create Flutter Plugin ZIP
echo   - Create Web SDK ZIP (NEW!)
echo.

REM Check if running from correct directory
if not exist "scripts\build_release_v2.bat" (
    echo [ERROR] Please run this script from project root directory
    echo.
    pause
    exit /b 1
)

REM Set variables
set "EXAMPLE_DIR=%cd%\example"
set "BUILD_DIR=%EXAMPLE_DIR%\build\windows\x64"
set "RELEASE_DIR=%cd%\release"
set "TEMPLATE_DIR=%cd%\templates"
set "VERSION=1.3.1"
set "RELEASE_NAME=anywp_engine_v%VERSION%"
set "ERROR_COUNT=0"

echo [1/18] Building Web SDK from modular source...
cd "%cd%\windows\sdk"
if exist "node_modules\" (
    call npm run build >nul 2>&1
    if errorlevel 1 (
        echo [WARNING] Web SDK build failed, using existing SDK
    ) else (
        echo   Web SDK built successfully
    )
) else (
    echo   Skipping SDK build - dependencies not installed
    echo   Run 'cd windows\sdk && npm install' to enable SDK building
)
cd "%cd%\..\..\"
echo.

echo [2/18] Cleaning old build...
if exist "%EXAMPLE_DIR%\build" (
    rmdir /s /q "%EXAMPLE_DIR%\build" 2>nul
    if errorlevel 1 (
        echo [WARNING] Cannot fully clean build directory
    )
)

echo [3/18] Running flutter clean...
cd "%EXAMPLE_DIR%"
flutter clean >nul 2>&1
if errorlevel 1 (
    echo [ERROR] flutter clean failed
    set /a ERROR_COUNT+=1
)

echo [4/18] Getting dependencies...
flutter pub get >nul 2>&1
if errorlevel 1 (
    echo [ERROR] flutter pub get failed
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

echo [5/18] Building Release version...
echo        This may take a few minutes, please wait...
flutter build windows --release
if errorlevel 1 (
    echo [ERROR] Release build failed
    cd ..
    pause
    exit /b 1
)

REM Check build artifacts
if not exist "%BUILD_DIR%\runner\Release\anywallpaper_engine_example.exe" (
    echo [ERROR] Build artifacts not found
    cd ..
    pause
    exit /b 1
)

echo [6/18] Creating Release directory structure...
cd ..
if not exist "%RELEASE_DIR%" (
    mkdir "%RELEASE_DIR%" 2>nul
)
if exist "%RELEASE_DIR%\%RELEASE_NAME%" (
    rmdir /s /q "%RELEASE_DIR%\%RELEASE_NAME%" 2>nul
    if errorlevel 1 (
        echo [ERROR] Cannot clean old Release directory
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

echo [7/18] Copying DLL and related files...
REM Plugin DLL
copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy plugin DLL
    set /a ERROR_COUNT+=1
)

copy "%BUILD_DIR%\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy plugin LIB - This is required for linker
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

REM WebView2Loader DLL
copy "windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%RELEASE_DIR%\%RELEASE_NAME%\bin\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy WebView2Loader.dll
    set /a ERROR_COUNT+=1
)

echo [8/18] Copying Dart source code...
REM Dart library - Copy directly to lib/ (standard location)
copy "lib\anywp_engine.dart" "%RELEASE_DIR%\%RELEASE_NAME%\lib\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy Dart source code to lib/
    set /a ERROR_COUNT+=1
    cd ..
    pause
    exit /b 1
)

REM Also copy to lib/dart/ (backward compatibility)
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart" 2>nul
copy "lib\anywp_engine.dart" "%RELEASE_DIR%\%RELEASE_NAME%\lib\dart\" >nul 2>&1

echo [9/18] Copying C++ header files...
mkdir "%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine" 2>nul
powershell -Command "Copy-Item -Path 'windows\include\anywp_engine\*' -Destination '%RELEASE_DIR%\%RELEASE_NAME%\include\anywp_engine' -Recurse -Force" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy C++ header files
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

echo [10/18] Syncing native source files and SDK...
copy "windows\anywp_sdk.js" "%RELEASE_DIR%\%RELEASE_NAME%\windows\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy SDK file (windows\\anywp_sdk.js)
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

copy "windows\anywp_engine_plugin.cpp" "%RELEASE_DIR%\%RELEASE_NAME%\windows\src\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy C++ source file anywp_engine_plugin.cpp
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

copy "windows\anywp_engine_plugin.h" "%RELEASE_DIR%\%RELEASE_NAME%\windows\src\" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy C++ header file anywp_engine_plugin.h
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

if exist "windows\packages" (
    powershell -Command "Copy-Item -Path 'windows\packages' -Destination '%RELEASE_DIR%\%RELEASE_NAME%\windows' -Recurse -Force" >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] Cannot copy WebView2 packages directory
        set /a ERROR_COUNT+=1
        pause
        exit /b 1
    )
)

if exist "windows\packages.config" (
    copy "windows\packages.config" "%RELEASE_DIR%\%RELEASE_NAME%\windows\" >nul 2>&1
)

echo [11/18] Creating CMake configuration...
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
echo   message^(STATUS "AnyWP Engine: Using precompiled DLL (${PRECOMPILED_DLL})"^)
echo   add_library^(${PLUGIN_NAME} SHARED IMPORTED GLOBAL^)
echo   set_target_properties^(${PLUGIN_NAME} PROPERTIES
echo     IMPORTED_LOCATION "${PRECOMPILED_DLL}"
echo     IMPORTED_IMPLIB "${PRECOMPILED_LIB}"
echo   ^)
echo   target_include_directories^(${PLUGIN_NAME} INTERFACE
echo     "${PRECOMPILED_DIR}/include"
echo   ^)
echo else()
echo   message^(STATUS "AnyWP Engine: Precompiled DLL not found, building from source"^)
echo   if^(NOT EXISTS ${SOURCE_FILE}^)
echo     message^(FATAL_ERROR "AnyWP Engine: Source file not found: ${SOURCE_FILE}"^)
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
echo     message^(WARNING "AnyWP Engine: WebView2 NuGet package not found, please run nuget restore"^)
echo   endif()
echo endif()
echo.
echo set^(anywp_engine_bundled_libraries
echo   "${PRECOMPILED_DIR}/bin/anywp_engine_plugin.dll"
echo   "${PRECOMPILED_DIR}/bin/WebView2Loader.dll"
echo   PARENT_SCOPE
echo ^)
) > "%RELEASE_DIR%\%RELEASE_NAME%\windows\CMakeLists.txt"

echo [12/18] Copying documentation...
copy "README.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "LICENSE" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1
copy "CHANGELOG_CN.md" "%RELEASE_DIR%\%RELEASE_NAME%\" >nul 2>&1

echo [13/18] Generating PRECOMPILED_README...
(
echo # AnyWP Engine v%VERSION% - 棰勭紪璇戠増鏈?
echo.
echo ## 馃摝 鍖呭惈鍐呭
echo.
echo - `bin/` - 杩愯鏃?DLL锛坅nywp_engine_plugin.dll锛変笌 WebView2Loader
echo - `lib/` - Dart 婧愮爜涓庨摼鎺ュ簱锛坅nywp_engine_plugin.lib锛?
echo - `include/` - C++ 鍏紑澶存枃浠?
echo - `windows/anywp_sdk.js` - JavaScript SDK
echo - `windows/src/` - C++ 瀹炵幇婧愮爜锛堝彲閫夋嫨鑷缂栬瘧锛?
echo - `windows/CMakeLists.txt` - 鑷姩妫€娴嬮缂栬瘧/婧愮爜妯″紡
echo.
echo ## 馃殌 蹇€熼泦鎴?
echo.
echo ### 1. 鎺ㄨ崘锛氳繍琛屽畨瑁呰剼鏈?
echo ```powershell
echo .\setup_precompiled.bat
echo ```
echo.
echo ### 2. 鎴栨墜鍔ㄥ湪 `pubspec.yaml` 涓坊鍔狅細
echo ```yaml
echo dependencies:
echo   anywp_engine:
echo     path: ./packages/anywp_engine_v%VERSION%
echo ```
echo.
echo ### 3. 瀹夎鍚庤繍琛?
echo ```bash
echo flutter pub get
echo flutter build windows
echo ```
echo.
echo ### 4. 鍚姩绀轰緥
echo ```bash
echo cd example_minimal
echo flutter run -d windows
echo ```
echo.
echo ## 馃摎 瀹屾暣鏂囨。
echo.
echo - README.md / CHANGELOG_CN.md
echo - setup_precompiled.bat锛堣嚜鍔ㄥ畨瑁咃級
echo - verify_precompiled.bat锛堝揩閫熼獙璇侊級
echo - generate_pubspec_snippet.bat锛坧ubspec 鐗囨锛?
echo.
echo 鏇村淇℃伅璇疯闂細https://github.com/zhaibin/AnyWallpaper-Engine
) > "%RELEASE_DIR%\%RELEASE_NAME%\PRECOMPILED_README.md"

echo [14/18] Generating automation helper scripts...
powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\setup_precompiled.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\setup_precompiled.bat'" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot generate setup_precompiled.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\verify_precompiled.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\verify_precompiled.bat'" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot generate verify_precompiled.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%TEMPLATE_DIR%\precompiled\generate_pubspec_snippet.template.bat') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\generate_pubspec_snippet.bat'" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot generate generate_pubspec_snippet.bat
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

echo [15/18] Copying minimal example project...
powershell -Command "Copy-Item -Path '%TEMPLATE_DIR%\example_minimal' -Destination '%RELEASE_DIR%\%RELEASE_NAME%' -Recurse -Force" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Cannot copy example project
    set /a ERROR_COUNT+=1
    pause
    exit /b 1
)

powershell -Command "(Get-Content '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\pubspec.yaml') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\pubspec.yaml'" >nul 2>&1
powershell -Command "(Get-Content '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\README.md') -replace '__VERSION__', '!VERSION!' | Set-Content -Encoding UTF8 '%RELEASE_DIR%\%RELEASE_NAME%\example_minimal\README.md'" >nul 2>&1

echo [16/18] Generating pubspec.yaml...
REM Create pubspec.yaml (Note: removed dartPluginClass)
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

echo [17/18] Creating ZIP archive...
cd "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path '%RELEASE_NAME%' -DestinationPath '%RELEASE_NAME%.zip' -Force" 2>nul
if errorlevel 1 (
    echo [ERROR] Cannot create ZIP file
    set /a ERROR_COUNT+=1
) else (
    REM Get file size
    for %%I in ("%RELEASE_NAME%.zip") do set "FILE_SIZE=%%~zI"
    set /a FILE_SIZE_KB=!FILE_SIZE! / 1024
)

cd ..

REM [17/17] Build Web SDK package
echo [18/18] Building Web SDK package...
powershell -ExecutionPolicy Bypass -File "scripts\build_web_sdk.ps1" -Version "%VERSION%" 2>nul
if errorlevel 1 (
    echo [WARNING] Web SDK package build failed
    set /a ERROR_COUNT+=1
) else (
    echo [SUCCESS] Web SDK package created: anywp_web_sdk_v%VERSION%.zip
)

echo.
echo ============================================
if !ERROR_COUNT! EQU 0 (
    echo Build completed successfully!
) else (
    echo Build completed with !ERROR_COUNT! warning(s)/error(s)
)
echo ============================================
echo.
echo Release packages location:
echo    1. Flutter Plugin: %RELEASE_DIR%\%RELEASE_NAME%.zip
if defined FILE_SIZE_KB (
    echo       Size: !FILE_SIZE_KB! KB
)
echo    2. Web SDK: %RELEASE_DIR%\anywp_web_sdk_v%VERSION%.zip
echo.
echo Extracted files location:
echo    %RELEASE_DIR%\%RELEASE_NAME%\
echo.
echo Next steps:
echo    1. Test if the precompiled package works
echo    2. Visit https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
echo    3. Select tag v%VERSION%
echo    4. Upload BOTH ZIP files:
echo       - %RELEASE_NAME%.zip (Flutter Plugin)
echo       - anywp_web_sdk_v%VERSION%.zip (Web SDK)
echo    5. Copy content from release\GITHUB_RELEASE_NOTES_v%VERSION%.md as description
echo.

if !ERROR_COUNT! GTR 0 (
    echo Please check the errors above before publishing
    echo.
)

if defined NO_PAUSE goto end

pause

:end
endlocal

