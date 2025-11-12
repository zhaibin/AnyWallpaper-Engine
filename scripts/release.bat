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
set PACKAGE_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%
set WEB_SDK_DIR=%RELEASE_DIR%\anywp_web_sdk_v%VERSION%

REM Step 1: Clean old release
echo [Step 1/17] Cleaning old release...
if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
if exist "%PACKAGE_DIR%.zip" del /q "%PACKAGE_DIR%.zip"
if exist "%WEB_SDK_DIR%" rmdir /s /q "%WEB_SDK_DIR%"
if exist "%WEB_SDK_DIR%.zip" del /q "%WEB_SDK_DIR%.zip"

REM Step 2: Build Release
echo [Step 2/17] Building Release version...
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

REM Step 3-16: Create package structure and copy files
echo [Step 3/17] Creating package structure...
mkdir "%PACKAGE_DIR%\bin"
mkdir "%PACKAGE_DIR%\lib"
mkdir "%PACKAGE_DIR%\include\anywp_engine"
mkdir "%PACKAGE_DIR%\sdk"
mkdir "%PACKAGE_DIR%\windows\modules"
mkdir "%PACKAGE_DIR%\windows\utils"
mkdir "%PACKAGE_DIR%\windows\test"
mkdir "%PACKAGE_DIR%\windows\packages"

echo [Step 4/17] Copying DLL files...
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%PACKAGE_DIR%\bin\"
copy "%PROJECT_ROOT%\windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%PACKAGE_DIR%\bin\"

echo [Step 5/17] Copying LIB file...
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%PACKAGE_DIR%\lib\"

echo [Step 6/17] Copying Dart source...
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%PACKAGE_DIR%\lib\"
mkdir "%PACKAGE_DIR%\lib\dart"
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%PACKAGE_DIR%\lib\dart\"

echo [Step 7/17] Copying C++ source...
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.cpp" "%PACKAGE_DIR%\windows\"
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.h" "%PACKAGE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\modules" "%PACKAGE_DIR%\windows\modules"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\utils" "%PACKAGE_DIR%\windows\utils"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\test" "%PACKAGE_DIR%\windows\test"

echo [Step 8/17] Copying headers...
copy "%PROJECT_ROOT%\windows\include\anywp_engine\any_w_p_engine_plugin.h" "%PACKAGE_DIR%\include\anywp_engine\"

echo [Step 9/17] Copying SDK...
copy "%PROJECT_ROOT%\windows\anywp_sdk.js" "%PACKAGE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\sdk" "%PACKAGE_DIR%\windows\sdk"

echo [Step 10/17] Copying CMake files...
copy "%PROJECT_ROOT%\windows\CMakeLists.txt" "%PACKAGE_DIR%\windows\"

echo [Step 11/17] Copying WebView2 packages...
xcopy /E /I /Y "%PROJECT_ROOT%\windows\packages" "%PACKAGE_DIR%\windows\packages"
copy "%PROJECT_ROOT%\windows\packages.config" "%PACKAGE_DIR%\windows\"

echo [Step 12/17] Copying documentation...
copy "%PROJECT_ROOT%\README.md" "%PACKAGE_DIR%\"
copy "%PROJECT_ROOT%\CHANGELOG_CN.md" "%PACKAGE_DIR%\"
copy "%PROJECT_ROOT%\LICENSE" "%PACKAGE_DIR%\"
copy "%PROJECT_ROOT%\docs\PRECOMPILED_DLL_INTEGRATION.md" "%PACKAGE_DIR%\PRECOMPILED_README.md"
copy "%PROJECT_ROOT%\pubspec.yaml" "%PACKAGE_DIR%\"

echo [Step 13/17] Creating ZIP package...
cd /d "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path 'anywp_engine_v%VERSION%\*' -DestinationPath 'anywp_engine_v%VERSION%.zip' -Force"

REM Step 14-17: Create Web SDK package
echo [Step 14/17] Creating Web SDK package...
mkdir "%WEB_SDK_DIR%\sdk"
mkdir "%WEB_SDK_DIR%\examples"
mkdir "%WEB_SDK_DIR%\docs"

echo [Step 15/17] Copying Web SDK files...
copy "%PROJECT_ROOT%\windows\anywp_sdk.js" "%WEB_SDK_DIR%\sdk\"
xcopy /E /I /Y "%PROJECT_ROOT%\examples\*.html" "%WEB_SDK_DIR%\examples\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE_CN.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\API_USAGE_EXAMPLES.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\LICENSE" "%WEB_SDK_DIR%\"

echo [Step 16/17] Creating Web SDK README...
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

echo [Step 17/17] Creating Web SDK ZIP...
cd /d "%RELEASE_DIR%"
powershell -Command "Compress-Archive -Path 'anywp_web_sdk_v%VERSION%\*' -DestinationPath 'anywp_web_sdk_v%VERSION%.zip' -Force"

REM Summary
echo.
echo ========================================
echo  Release Build Complete!
echo ========================================
echo.
echo Packages created:
echo   1. anywp_engine_v%VERSION%.zip
echo   2. anywp_web_sdk_v%VERSION%.zip
echo.
echo Location: %RELEASE_DIR%
echo.

if not defined NO_PAUSE pause

