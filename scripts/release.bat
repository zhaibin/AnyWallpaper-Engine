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

REM Setup PowerShell command (must be before reading SDK version)
set PWSH_CMD=pwsh
where %PWSH_CMD% >nul 2>nul
if ERRORLEVEL 1 set PWSH_CMD=powershell

REM Read version from pubspec.yaml (Flutter plugin version)
for /f "tokens=2" %%a in ('findstr "^version:" "%~dp0..\pubspec.yaml"') do set VERSION=%%a
echo Flutter Plugin Version: %VERSION%

REM Read JS SDK version from package.json using PowerShell for reliable JSON parsing
for /f "delims=" %%a in ('%PWSH_CMD% -NoLogo -NoProfile -Command "(Get-Content '%~dp0..\windows\sdk\package.json' | ConvertFrom-Json).version"') do set SDK_VERSION=%%a
echo JS SDK Version: %SDK_VERSION%
echo.

set PROJECT_ROOT=%~dp0..
set RELEASE_DIR=%PROJECT_ROOT%\release
set PRECOMPILED_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_precompiled
set SOURCE_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%_source
REM Web SDK package uses JS SDK version (set after reading SDK_VERSION above)
set WEB_SDK_DIR=%RELEASE_DIR%\anywp_web_sdk_v%SDK_VERSION%
set TOTAL_STEPS=31
set STEP=1

call :PrintStep "Checking version consistency..."
%PWSH_CMD% -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_ROOT%\scripts\check_version_consistency.ps1" -Version %VERSION%
if ERRORLEVEL 1 (
    echo ERROR: Version consistency check failed.
    if not defined NO_PAUSE pause
    exit /b 1
)

REM Step: Build Web SDK in production mode (generates minified version)
call :PrintStep "Building Web SDK in production mode..."
call "%PROJECT_ROOT%\scripts\build_sdk.bat" production
if errorlevel 1 (
    echo ERROR: SDK build failed
    if not defined NO_PAUSE pause
    exit /b 1
)

REM Step: Clean old release
call :PrintStep "Cleaning old release..."
if exist "%PRECOMPILED_DIR%" rmdir /s /q "%PRECOMPILED_DIR%"
if exist "%PRECOMPILED_DIR%.zip" del /q "%PRECOMPILED_DIR%.zip"
if exist "%SOURCE_DIR%" rmdir /s /q "%SOURCE_DIR%"
if exist "%SOURCE_DIR%.zip" del /q "%SOURCE_DIR%.zip"
if exist "%WEB_SDK_DIR%" rmdir /s /q "%WEB_SDK_DIR%"
if exist "%WEB_SDK_DIR%.zip" del /q "%WEB_SDK_DIR%.zip"

REM Step: Build Release
call :PrintStep "Building Release version..."
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

call :PrintStep "Creating precompiled package structure..."
mkdir "%PRECOMPILED_DIR%\bin"
mkdir "%PRECOMPILED_DIR%\lib"
mkdir "%PRECOMPILED_DIR%\include\anywp_engine"
mkdir "%PRECOMPILED_DIR%\windows"
mkdir "%PRECOMPILED_DIR%\sdk"
mkdir "%PRECOMPILED_DIR%\examples"

call :PrintStep "Copying DLL files to precompiled package..."
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%PRECOMPILED_DIR%\bin\"
copy "%PROJECT_ROOT%\windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%PRECOMPILED_DIR%\bin\"

call :PrintStep "Copying LIB file to precompiled package..."
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%PRECOMPILED_DIR%\lib\"

call :PrintStep "Copying Dart API to precompiled package..."
mkdir "%PRECOMPILED_DIR%\lib\dart"
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%PRECOMPILED_DIR%\lib\dart\"
REM Also copy to standard location for Flutter compatibility
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%PRECOMPILED_DIR%\lib\"

call :PrintStep "Copying C API header to precompiled package..."
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin_c_api.h" "%PRECOMPILED_DIR%\include\anywp_engine\"
REM Create Flutter wrapper header (any_w_p_engine_plugin.h)
(
echo // Wrapper header for Flutter's generated plugin registrant
echo // This file provides compatibility with Flutter's naming convention
echo #ifndef ANY_W_P_ENGINE_PLUGIN_H_
echo #define ANY_W_P_ENGINE_PLUGIN_H_
echo #include "anywp_engine_plugin_c_api.h"
echo #endif  // ANY_W_P_ENGINE_PLUGIN_H_
) > "%PRECOMPILED_DIR%\include\anywp_engine\any_w_p_engine_plugin.h"

call :PrintStep "Copying CMakeLists.txt to precompiled package..."
copy "%PROJECT_ROOT%\windows\CMakeLists.precompiled.txt" "%PRECOMPILED_DIR%\windows\CMakeLists.txt"

call :PrintStep "SDK Embedding Verification (v2.3.0+)..."
REM v2.3.0+: SDK is embedded in DLL as Windows resource
REM No external SDK files needed for runtime!
REM The SDK is compiled into anywp_engine_plugin.dll during build
echo   [INFO] SDK is embedded in DLL (no external files needed)
echo   [INFO] Precompiled package only contains DLL with embedded SDK
echo   [INFO] Users don't need to manually copy SDK files

REM Note: For backward compatibility testing, we can optionally include SDK files
REM But they won't be used at runtime - DLL will load from embedded resource
REM Uncomment below if you want to include SDK files for reference:
REM if not exist "%PRECOMPILED_DIR%\sdk\dist\" mkdir "%PRECOMPILED_DIR%\sdk\dist\"
REM if exist "%PROJECT_ROOT%\sdk\dist\anywp_sdk.js" (
REM     copy "%PROJECT_ROOT%\sdk\dist\anywp_sdk.js" "%PRECOMPILED_DIR%\sdk\dist\"
REM     echo   [OK] SDK reference file copied (not needed at runtime)
REM )

call :PrintStep "Copying example HTML files to precompiled package..."
xcopy /E /I /Y "%PROJECT_ROOT%\examples\*.html" "%PRECOMPILED_DIR%\examples\"

call :PrintStep "Copying documentation to precompiled package..."
copy "%PROJECT_ROOT%\README.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\CHANGELOG_CN.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\LICENSE" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\docs\PRECOMPILED_DLL_INTEGRATION.md" "%PRECOMPILED_DIR%\INTEGRATION_GUIDE.md"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE_CN.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE.md" "%PRECOMPILED_DIR%\"
copy "%PROJECT_ROOT%\pubspec.yaml" "%PRECOMPILED_DIR%\"

call :PrintStep "Verifying precompiled package integrity..."
set VERIFY_ERROR=0
if not exist "%PRECOMPILED_DIR%\lib\anywp_engine.dart" (
    echo [ERROR] Dart file not in standard location
    set VERIFY_ERROR=1
)
if not exist "%PRECOMPILED_DIR%\include\anywp_engine\any_w_p_engine_plugin.h" (
    echo [ERROR] Flutter wrapper header missing
    set VERIFY_ERROR=1
)
REM Verify CMakeLists.txt contains GLOBAL
findstr /C:"IMPORTED GLOBAL" "%PRECOMPILED_DIR%\windows\CMakeLists.txt" >nul
if errorlevel 1 (
    echo [WARNING] CMakeLists.txt may be missing GLOBAL keyword
)
if %VERIFY_ERROR%==1 (
    echo [FAILED] Package verification failed
    exit /b 1
) else (
    echo [SUCCESS] Package verification passed
)

call :PrintStep "Creating verify.bat script..."
copy "%PROJECT_ROOT%\scripts\verify_template.bat" "%PRECOMPILED_DIR%\verify.bat" >nul
if not exist "%PRECOMPILED_DIR%\verify.bat" (
    echo [ERROR] Failed to copy verify template
    exit /b 1
)

call :PrintStep "Creating precompiled ZIP package..."
cd /d "%PRECOMPILED_DIR%"
%PWSH_CMD% -NoLogo -NoProfile -Command "Compress-Archive -Path '*' -DestinationPath '..\anywp_engine_v%VERSION%_precompiled.zip' -Force"
cd /d "%RELEASE_DIR%"

REM ==========================================
REM Part B: Source Package (Full source code)
REM ==========================================

call :PrintStep "Creating source package structure..."
mkdir "%SOURCE_DIR%\bin"
mkdir "%SOURCE_DIR%\lib"
mkdir "%SOURCE_DIR%\include\anywp_engine"
mkdir "%SOURCE_DIR%\sdk"
mkdir "%SOURCE_DIR%\windows\modules"
mkdir "%SOURCE_DIR%\windows\utils"
mkdir "%SOURCE_DIR%\windows\test"
mkdir "%SOURCE_DIR%\windows\packages"

call :PrintStep "Copying binaries to source package..."
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "%SOURCE_DIR%\bin\"
copy "%PROJECT_ROOT%\windows\packages\Microsoft.Web.WebView2.1.0.2592.51\build\native\x64\WebView2Loader.dll" "%SOURCE_DIR%\bin\"
copy "%PROJECT_ROOT%\example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "%SOURCE_DIR%\lib\"

call :PrintStep "Copying Dart source to source package..."
copy "%PROJECT_ROOT%\lib\anywp_engine.dart" "%SOURCE_DIR%\lib\"

call :PrintStep "Copying C++ source to source package..."
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.cpp" "%SOURCE_DIR%\windows\"
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin.h" "%SOURCE_DIR%\windows\"
copy "%PROJECT_ROOT%\windows\anywp_engine_plugin_c_api.h" "%SOURCE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\modules" "%SOURCE_DIR%\windows\modules"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\utils" "%SOURCE_DIR%\windows\utils"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\test" "%SOURCE_DIR%\windows\test"

call :PrintStep "Copying headers to source package..."
REM Copy the wrapper header from precompiled package (generated during precompiled build)
copy "%PRECOMPILED_DIR%\include\anywp_engine\any_w_p_engine_plugin.h" "%SOURCE_DIR%\include\anywp_engine\"

call :PrintStep "Copying SDK to source package..."
REM Copy SDK source tree (v2.2.0+: top-level sdk/ directory)
xcopy /E /I /Y "%PROJECT_ROOT%\sdk" "%SOURCE_DIR%\sdk\"
REM Note: Source package includes full SDK source tree with TypeScript source, not just compiled JS

call :PrintStep "Copying CMake and WebView2 packages to source package..."
copy "%PROJECT_ROOT%\windows\CMakeLists.txt" "%SOURCE_DIR%\windows\"
xcopy /E /I /Y "%PROJECT_ROOT%\windows\packages" "%SOURCE_DIR%\windows\packages"
copy "%PROJECT_ROOT%\windows\packages.config" "%SOURCE_DIR%\windows\"

call :PrintStep "Copying documentation to source package..."
copy "%PROJECT_ROOT%\README.md" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\CHANGELOG_CN.md" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\LICENSE" "%SOURCE_DIR%\"
copy "%PROJECT_ROOT%\docs\PRECOMPILED_DLL_INTEGRATION.md" "%SOURCE_DIR%\INTEGRATION_GUIDE.md"
copy "%PROJECT_ROOT%\pubspec.yaml" "%SOURCE_DIR%\"

call :PrintStep "Creating source ZIP package..."
cd /d "%SOURCE_DIR%"
%PWSH_CMD% -NoLogo -NoProfile -Command "Compress-Archive -Path '*' -DestinationPath '..\anywp_engine_v%VERSION%_source.zip' -Force"
cd /d "%RELEASE_DIR%"

REM ==========================================
REM Part C: Web SDK Package
REM ==========================================

call :PrintStep "Creating Web SDK package structure..."
mkdir "%WEB_SDK_DIR%\sdk"
mkdir "%WEB_SDK_DIR%\examples"
mkdir "%WEB_SDK_DIR%\docs"

call :PrintStep "Copying Web SDK files..."
REM Copy Web SDK files from sdk/dist/ directory (v2.4.1+ corrected path)
REM Note: Web SDK package uses JS SDK version, not Flutter plugin version
if exist "%PROJECT_ROOT%\sdk\dist\anywp_sdk.min.js" (
    copy "%PROJECT_ROOT%\sdk\dist\anywp_sdk.min.js" "%WEB_SDK_DIR%\sdk\"
    echo   [OK] Copied minified SDK: anywp_sdk.min.js
) else (
    echo   [WARNING] Minified SDK not found at sdk\dist\anywp_sdk.min.js
)
if exist "%PROJECT_ROOT%\sdk\dist\anywp_sdk.js" (
    copy "%PROJECT_ROOT%\sdk\dist\anywp_sdk.js" "%WEB_SDK_DIR%\sdk\"
    echo   [OK] Copied unminified SDK: anywp_sdk.js
) else (
    echo   [ERROR] Unminified SDK not found at sdk\dist\anywp_sdk.js
)
xcopy /E /I /Y "%PROJECT_ROOT%\examples\*.html" "%WEB_SDK_DIR%\examples\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE_CN.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\WEB_DEVELOPER_GUIDE.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\docs\API_USAGE_EXAMPLES.md" "%WEB_SDK_DIR%\docs\"
copy "%PROJECT_ROOT%\LICENSE" "%WEB_SDK_DIR%\"

call :PrintStep "Creating Web SDK README..."
(
echo # AnyWP Engine - Web SDK v%SDK_VERSION%
echo.
echo **Web SDK Version**: v%SDK_VERSION%
echo **Compatible with AnyWP Engine**: v%VERSION%+
echo.
echo ^> **Note**: Web SDK has independent version numbers. This SDK is compatible with AnyWP Engine v%VERSION% and later versions.
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
echo See `examples/` folder for test pages.
echo.
echo ## License
echo.
echo MIT License - See LICENSE file
) > "%WEB_SDK_DIR%\README.md"

call :PrintStep "Creating Web SDK ZIP..."
cd /d "%WEB_SDK_DIR%"
REM Use SDK version for Web SDK package ZIP filename
%PWSH_CMD% -NoLogo -NoProfile -Command "Compress-Archive -Path '*' -DestinationPath '..\anywp_web_sdk_v%SDK_VERSION%.zip' -Force"
cd /d "%RELEASE_DIR%"

call :PrintStep "Generating GitHub release notes..."
%PWSH_CMD% -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_ROOT%\scripts\generate_release_notes.ps1" -Version %VERSION% -ProjectRoot "%PROJECT_ROOT%"
if ERRORLEVEL 1 (
    echo ERROR: Failed to generate release notes.
    if not defined NO_PAUSE pause
    exit /b 1
)

call :PrintStep "Preparing release commit template..."
%PWSH_CMD% -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_ROOT%\scripts\generate_commit_template.ps1" -Version %VERSION% -ProjectRoot "%PROJECT_ROOT%"
if ERRORLEVEL 1 (
    echo ERROR: Failed to generate commit template.
    if not defined NO_PAUSE pause
    exit /b 1
)

REM Summary
echo.
echo ========================================
echo  Release Build Complete!
echo ========================================
echo.
echo Version Information:
echo   Flutter Plugin Version: %VERSION%
echo   Web SDK Version:        %SDK_VERSION%
echo.
echo Packages created:
echo   1. anywp_engine_v%VERSION%_precompiled.zip  (Engine v%VERSION%)
echo   2. anywp_engine_v%VERSION%_source.zip       (Engine v%VERSION%)
echo   3. anywp_web_sdk_v%SDK_VERSION%.zip         (SDK v%SDK_VERSION%)
echo.
echo Location: %RELEASE_DIR%
echo.
echo Package descriptions:
echo   - Precompiled: For Flutter developers (minimal integration)
echo   - Source: For developers who need to modify or rebuild
echo   - Web SDK: For wallpaper developers (HTML/CSS/JS) - Independent version
echo.
echo ========================================
echo  Next Steps
echo ========================================
echo.
echo 1. Verify packages:
echo    .\scripts\verify_precompiled.bat %VERSION%
echo.
echo 2. Commit and push to Git (optional):
echo    .\scripts\release_git.bat %VERSION%
echo    Or manually:
echo    git add release/ CHANGELOG_CN.md pubspec.yaml docs/ README.md .cursorrules
echo    git commit -F release\commit_msg_v%VERSION%.txt
echo    git push origin main
echo    git tag -a v%VERSION% -m "AnyWP Engine v%VERSION%"
echo    git push origin v%VERSION%
echo.
echo 3. Create GitHub Release:
echo    - Visit: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
echo    - Tag: v%VERSION%
echo    - Description: Copy from release\GITHUB_RELEASE_NOTES_v%VERSION%.md
echo    - Upload 3 ZIP files from release\ directory
echo.

if not defined NO_PAUSE pause

goto :EOF


:PrintStep
echo [Step !STEP!/%TOTAL_STEPS%] %~1
set /a STEP+=1
goto :EOF
