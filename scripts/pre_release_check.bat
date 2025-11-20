@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Pre-Release Check Script
REM Comprehensive verification before release
REM ==========================================

echo ========================================
echo  AnyWP Engine - Pre-Release Check
echo ========================================
echo.

REM Setup PowerShell command
set PWSH_CMD=pwsh
where %PWSH_CMD% >nul 2>nul
if ERRORLEVEL 1 set PWSH_CMD=powershell

REM Read Flutter plugin version from pubspec.yaml
for /f "tokens=2" %%a in ('findstr "^version:" "%~dp0..\pubspec.yaml"') do set VERSION=%%a
echo Flutter Plugin Version: %VERSION%

REM Read Web SDK version from package.json
for /f "delims=" %%a in ('%PWSH_CMD% -NoLogo -NoProfile -Command "(Get-Content '%~dp0..\windows\sdk\package.json' | ConvertFrom-Json).version"') do set SDK_VERSION=%%a
echo Web SDK Version: %SDK_VERSION%
echo.

set ERROR_COUNT=0
set WARNING_COUNT=0
set TOTAL_CHECKS=13
set CHECK=1

call :PrintCheck "Version consistency check..."
%PWSH_CMD% -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0check_version_consistency.ps1" -Version %VERSION%
if ERRORLEVEL 1 (
    call :AddError "Version consistency check failed"
) else (
    echo   [PASS] All versions are consistent
)

call :PrintCheck "Checking Web SDK package.json..."
echo   [INFO] Web SDK version: %SDK_VERSION% (independent from plugin version %VERSION%)

call :PrintCheck "Checking Windows version header..."
if exist "%~dp0..\windows\version.h" (
    findstr /C:"#define ANYWP_ENGINE_VERSION \"%VERSION%\"" "%~dp0..\windows\version.h" >nul
    if ERRORLEVEL 1 (
        call :AddError "windows/version.h version mismatch"
    ) else (
        echo   [PASS] version.h matches %VERSION%
    )
) else (
    call :AddError "windows/version.h not found"
)

call :PrintCheck "Checking CHANGELOG_CN.md..."
findstr /C:"## [%VERSION%]" "%~dp0..\CHANGELOG_CN.md" >nul
if ERRORLEVEL 1 (
    call :AddError "CHANGELOG_CN.md missing version %VERSION%"
) else (
    echo   [PASS] CHANGELOG_CN.md contains version %VERSION%
)

call :PrintCheck "Checking .cursorrules version..."
findstr /C:"**Version**: %VERSION%" "%~dp0..\.cursorrules" >nul
if ERRORLEVEL 1 (
    call :AddError ".cursorrules version not updated"
) else (
    echo   [PASS] .cursorrules version matches
)

call :PrintCheck "Checking Flutter lints..."
cd /d "%~dp0..\example"
flutter analyze >nul 2>&1
if ERRORLEVEL 1 (
    call :AddWarning "Flutter analyze found issues"
    echo   Run 'flutter analyze' for details
) else (
    echo   [PASS] No lint errors
)
cd /d "%~dp0"

call :PrintCheck "Checking WebView2 SDK..."
if exist "%~dp0..\windows\packages\Microsoft.Web.WebView2.1.0.2592.51" (
    echo   [PASS] WebView2 SDK found
) else (
    call :AddError "WebView2 SDK not found"
)

call :PrintCheck "Checking Web SDK build files..."
if exist "%~dp0..\sdk\dist\anywp_sdk.js" (
    findstr /C:"SDK v%VERSION%" "%~dp0..\sdk\dist\anywp_sdk.js" >nul
    if ERRORLEVEL 1 (
        call :AddWarning "Web SDK needs rebuild (version mismatch in built file)"
        echo   Run: .\scripts\build_sdk.bat
    ) else (
        echo   [PASS] Web SDK built with correct version
    )
) else (
    call :AddError "Web SDK not built (sdk/dist/anywp_sdk.js missing)"
)

call :PrintCheck "Checking critical documentation..."
set DOC_MISSING=0
if not exist "%~dp0..\README.md" set DOC_MISSING=1
if not exist "%~dp0..\CHANGELOG_CN.md" set DOC_MISSING=1
if not exist "%~dp0..\docs\PRECOMPILED_DLL_INTEGRATION.md" set DOC_MISSING=1
if not exist "%~dp0..\docs\WEB_DEVELOPER_GUIDE_CN.md" set DOC_MISSING=1
if !DOC_MISSING!==1 (
    call :AddError "Critical documentation missing"
) else (
    echo   [PASS] All critical docs present
)

call :PrintCheck "Checking Git status..."
git diff-index --quiet HEAD --
if ERRORLEVEL 1 (
    call :AddWarning "Uncommitted changes detected"
    echo   Run 'git status' to review
) else (
    echo   [PASS] Working tree clean
)

call :PrintCheck "Checking release directory..."
if exist "%~dp0..\release\anywp_engine_v%VERSION%_precompiled.zip" (
    call :AddWarning "Release package already exists for v%VERSION%"
    echo   Consider cleaning release/ directory first
) else (
    echo   [PASS] No conflicting release packages
)

call :PrintCheck "Running Web SDK tests..."
cd /d "%~dp0..\windows\sdk"
call npm test >nul 2>&1
if ERRORLEVEL 1 (
    call :AddError "Web SDK tests failed"
    echo   Run: cd windows\sdk; npm test
) else (
    echo   [PASS] All SDK tests passed
)
cd /d "%~dp0"

call :PrintCheck "Checking documentation consistency..."
%PWSH_CMD% -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0check_docs_consistency.ps1" -EngineVersion %VERSION% -SdkVersion %SDK_VERSION%
if ERRORLEVEL 1 (
    call :AddError "Documentation consistency check failed"
) else (
    echo   [PASS] Documentation consistency verified
    echo   [NOTE] Please manually review docs for content updates
)

REM Summary
echo.
echo ========================================
echo  Check Summary
echo ========================================
echo   Total Checks: %TOTAL_CHECKS%
echo   Errors:       !ERROR_COUNT!
echo   Warnings:     !WARNING_COUNT!
echo ========================================
echo.

if !ERROR_COUNT! GTR 0 (
    echo [FAILED] Release blocked due to errors
    echo Fix all errors before proceeding with release.
    echo.
    pause
    exit /b 1
)

if !WARNING_COUNT! GTR 0 (
    echo [WARNING] %WARNING_COUNT% warnings detected
    echo You may proceed, but review warnings first.
    echo.
    set /p CONTINUE="Continue with release? (y/N): "
    if /i not "!CONTINUE!"=="y" (
        echo Release cancelled by user.
        exit /b 1
    )
)

echo [SUCCESS] All checks passed!
echo Ready to proceed with release.
echo.
echo Next steps:
echo   1. Run: .\scripts\release.bat
echo   2. Verify packages: .\scripts\verify_precompiled.bat %VERSION%
echo   3. Commit and push: .\scripts\release_git.bat %VERSION%
echo.
goto :EOF

:PrintCheck
echo [Check !CHECK!/%TOTAL_CHECKS%] %~1
set /a CHECK+=1
goto :EOF

:AddError
echo   [ERROR] %~1
set /a ERROR_COUNT+=1
goto :EOF

:AddWarning
echo   [WARN] %~1
set /a WARNING_COUNT+=1
goto :EOF

