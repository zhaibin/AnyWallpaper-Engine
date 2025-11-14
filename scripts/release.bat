@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Release Build Script
REM Build release packages for distribution
REM ==========================================

echo ========================================
echo  AnyWP Engine - Release Build
echo ========================================
echo.

REM Read version from pubspec.yaml
for /f "tokens=2" %%a in ('findstr "^version:" "%~dp0..\pubspec.yaml"') do set VERSION=%%a
echo Version: %VERSION%
echo.

set PROJECT_ROOT=%~dp0..
set RELEASE_DIR=%PROJECT_ROOT%\release
set PRECOMPILED_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_precompiled
set SOURCE_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_source
set WEB_SDK_DIR=%RELEASE_DIR%\anywp_web_sdk_v%VERSION%

REM Step 1: Clean old release
echo [Step 1/23] Cleaning old release...
if exist "%PRECOMPILED_DIR%" rmdir /s /q "%PRECOMPILED_DIR%"
if exist "%PRECOMPILED_DIR%.zip" del /q "%PRECOMPILED_DIR%.zip"
if exist "%SOURCE_DIR%" rmdir /s /q "%SOURCE_DIR%"
if exist "%SOURCE_DIR%.zip" del /q "%SOURCE_DIR%.zip"
if exist "%WEB_SDK_DIR%" rmdir /s /q "%WEB_SDK_DIR%"
if exist "%WEB_SDK_DIR%.zip" del /q "%WEB_SDK_DIR%.zip"

REM Step 2: Build Release
echo [Step 2/23] Building Release version...
cd /d "%PROJECT_ROOT%\example"
call flutter build windows --release
set BUILD_ERROR=%ERRORLEVEL%
echo Build completed with exit code: %BUILD_ERROR%
if %BUILD_ERROR% NEQ 0 (
    echo ERROR: Build failed with code %BUILD_ERROR%
    if not defined NO_PAUSE pause
    exit /b 1
)
cd /d "%PROJECT_ROOT%"
echo Returned to project root: %CD%

REM ==========================================
REM Part A: Precompiled Package (DLL + LIB + Headers)
REM ==========================================

echo [Step 3/23] Creating precompiled package structure...
mkdir "%PRECOMPILED_DIR%\bin"
mkdir "%PRECOMPILED_DIR%\lib"
mkdir "%PRECOMPILED_DIR%\include\anywp_engine"
mkdir "%PRECOMPILED_DIR%\windows"
mkdir "%PRECOMPILED_DIR%\sdk"
mkdir "%PRECOMPILED_DIR%\examples"

echo [Step 4/23] Copying DLL files to precompiled package...
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%PRECOMPILED_DIR%\bin\"
copy "%PROJECT_ROOT%\windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%PRECOMPILED_DIR%\bin\"

echo [Step 5/23] Copying LIB file to precompiled package...
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%PRECOMPILED_DIR%\lib\"

echo [Step 6/23] Copying Dart API to precompiled package...
mkdir "%PRECOMPILED_DIR%\lib\dart"
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%PRECOMPILED_DIR%\lib\dart\"

echo [Step 7/23] Copying C API header to precompiled package...
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin_c_api.h" "%PRECOMPILED_DIR%\include\anywp_engine\"

echo [Step 8/23] Copying CMakeLists.txt to precompiled package...
copy "%PROJECT_ROOT%\windows\CMakeLists.precompiled.txt" "%PRECOMPILED_DIR%\windows\CMakeLists.txt"

echo [Step 9/23] Copying Web SDK to precompiled package...
copy "%PROJECT_ROOT%\windows\anywp_sdk.js" "%PRECOMPILED_DIR%\sdk\"

echo [Step 10/23] Copying example HTML files to precompiled package...
xcopy /E /I /Y "%PROJECT_ROOT%\examples\*.html" "%PRECOMPILED_DIR%\examples\"

echo [Step 11/23] Copying documentation to precompiled package...
copy "%PROJECT_ROOT%\README.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\CHANGELOG_CN.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\LICENSE" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\docs\PRECOMPILED_DLL_INTEGRATION.md" "%PRECOMPILED_DIR%\INTEGRATION_GUIDE.md"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE_CN.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\pubspec.yaml" "%PRECOMPILED_DIR%\"

echo [Step 12/23] Creating precompiled ZIP package...
cd /d "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path 'anywp_engine_v%VERSION%_precompiled\*' -DestinationPath 'anywp_engine_v%VERSION%_precompiled.zip' -Force"

REM ==========================================
REM Part B: Source Package (Full source code)
REM ==========================================

echo [Step 13/23] Creating source package structure...
mkdir "%SOURCE_DIR%\bin"
mkdir "%SOURCE_DIR%\lib"
mkdir "%SOURCE_DIR%\include\anywp_engine"
mkdir "%SOURCE_DIR%\sdk"
mkdir "%SOURCE_DIR%\windows\modules"
mkdir "%SOURCE_DIR%\windows\utils"
mkdir "%SOURCE_DIR%\windows\test"
mkdir "%SOURCE_DIR%\windows\packages"

echo [Step 14/23] Copying binaries to source package...
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%SOURCE_DIR%\bin\"
copy "%PROJECT_ROOT%\windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%SOURCE_DIR%\bin\"
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%SOURCE_DIR%\lib\"

echo [Step 15/23] Copying Dart source to source package...
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%SOURCE_DIR%\lib\"

echo [Step 16/23] Copying C++ source to source package...
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.cpp" "%SOURCE_DIR%\windows\"
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.h" "%SOURCE_DIR%\windows\"
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin_c_api.h" "%SOURCE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\modules" "%SOURCE_DIR%\windows\modules"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\utils" "%SOURCE_DIR%\windows\utils"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\test" "%SOURCE_DIR%\windows\test"

echo [Step 17/23] Copying headers to source package...
copy "%PROJECT_ROOT%\windows\include\anywp_engine\any_w_p_engine_plugin.h" "%SOURCE_DIR%\include\anywp_engine\"

echo [Step 18/23] Copying SDK to source package...
copy "%PROJECT_ROOT%\windows\anywp_sdk.js" "%SOURCE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\sdk" "%SOURCE_DIR%\windows\sdk"

echo [Step 19/23] Copying CMake and WebView2 packages to source package...
copy "%PROJECT_ROOT%\windows\CMakeLists.txt" "%SOURCE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\packages" "%SOURCE_DIR%\windows\packages"
copy "%PROJECT_ROOT%\windows\packages.config" "%SOURCE_DIR%\windows\"

echo [Step 20/23] Copying documentation to source package...
copy "%PROJECT_ROOT%\README.md" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\CHANGELOG_CN.md" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\LICENSE" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\docs\PRECOMPILED_DLL_INTEGRATION.md" "%SOURCE_DIR%\INTEGRATION_GUIDE.md"
copy "%PROJECT_ROOT%\pubspec.yaml" "%SOURCE_DIR%\"

echo [Step 21/23] Creating source ZIP package...
cd /d "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path 'anywp_engine_v%VERSION%_source\*' -DestinationPath 'anywp_engine_v%VERSION%_source.zip' -Force"

REM ==========================================
REM Part C: Web SDK Package
REM ==========================================

echo [Step 20/23] Creating Web SDK package structure...
mkdir "%WEB_SDK_DIR%\sdk"
mkdir "%WEB_SDK_DIR%\examples"
mkdir "%WEB_SDK_DIR%\docs"

echo [Step 21/23] Copying Web SDK files...
copy "%PROJECT_ROOT%\windows\anywp_sdk.js" "%WEB_SDK_DIR%\sdk\"
xcopy /E /I /Y "%PROJECT_ROOT%\examples\*.html" "%WEB_SDK_DIR%\examples\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE_CN.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\API_USAGE_EXAMPLES.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\LICENSE" "%WEB_SDK_DIR%\"

echo [Step 22/23] Creating Web SDK README...
(
echo # AnyWP Engine - Web SDK v%VERSION%
echo.
echo Standalone Web SDK package for wallpaper developers.
echo.
echo ## Quick Start
echo.
echo ```html
echo ^<script src="sdk/anywp_sdk.js"^>^</script^>
echo ^<script^>
echo   AnyWP.onClick^(element, ^(^) =^> { /* handler */ }^);
echo ^</script^>
echo ```
echo.
echo ## Documentation
echo.
echo - `docs/WEB_DEVELOPER_GUIDE_CN.md` - Chinese guide
echo - `docs/WEB_DEVELOPER_GUIDE.md` - English guide
echo - `docs/API_USAGE_EXAMPLES.md` - API examples
echo.
echo ## Examples
echo.
echo See `examples/` folder for 8 test pages.
echo.
echo ## License
echo.
echo MIT License - See LICENSE file
) > "%WEB_SDK_DIR%\README.md"

echo [Step 23/23] Creating Web SDK ZIP...
cd /d "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path 'anywp_web_sdk_v%VERSION%\*' -DestinationPath 'anywp_web_sdk_v%VERSION%.zip' -Force"

REM Summary
echo.
echo ========================================
echo  Release Build Complete!
echo ========================================
echo.
echo Packages created:
echo   1. anywp_engine_v%VERSION%_precompiled.zip  (DLL + LIB + C API Header)
echo   2. anywp_engine_v%VERSION%_source.zip       (Full source code)
echo   3. anywp_web_sdk_v%VERSION%.zip             (Web SDK)
echo.
echo Location: %RELEASE_DIR%
echo.
echo Package descriptions:
echo   - Precompiled: For Flutter developers who want minimal integration
echo   - Source: For developers who need to modify or rebuild from source
echo   - Web SDK: For wallpaper developers (HTML/CSS/JS)
echo.

if not defined NO_PAUSE pause

