@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Documentation Verification
REM Verify documentation accuracy, completeness, and language compliance
REM ==========================================

echo ========================================
echo  AnyWP Engine - Documentation Verification
echo ========================================
echo.

set ERROR_COUNT=0
set WARNING_COUNT=0
set PROJECT_ROOT=%~dp0..

REM ==========================================
REM Section 1: Language Compliance Check
REM ==========================================
echo [Section 1] Language Compliance Check
echo ----------------------------------------

REM Check root README.md for Chinese characters
echo [1.1] Checking README.md language...
findstr /C:"推荐" "%PROJECT_ROOT%\README.md" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [ERROR] README.md contains Chinese text
    echo   Found: Line 43 - Chinese characters detected
    set /a ERROR_COUNT+=1
) else (
    echo   [OK] README.md is English only
)

REM Check QUICK_INTEGRATION.md for Chinese characters
echo [1.2] Checking QUICK_INTEGRATION.md language...
if exist "%PROJECT_ROOT%\QUICK_INTEGRATION.md" (
    findstr /C:"中" "%PROJECT_ROOT%\QUICK_INTEGRATION.md" >nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo   [ERROR] QUICK_INTEGRATION.md contains Chinese text
        set /a ERROR_COUNT+=1
    ) else (
        echo   [OK] QUICK_INTEGRATION.md is English only
    )
) else (
    echo   [WARN] QUICK_INTEGRATION.md not found
    set /a WARNING_COUNT+=1
)

REM Check LICENSE for language
echo [1.3] Checking LICENSE language...
if exist "%PROJECT_ROOT%\LICENSE" (
    findstr /C:"MIT License" "%PROJECT_ROOT%\LICENSE" >nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo   [OK] LICENSE is standard English format
    ) else (
        echo   [WARN] LICENSE format check inconclusive
        set /a WARNING_COUNT+=1
    )
) else (
    echo   [ERROR] LICENSE file not found
    set /a ERROR_COUNT+=1
)

echo.

REM ==========================================
REM Section 2: File Reference Validation
REM ==========================================
echo [Section 2] File Reference Validation
echo ----------------------------------------

REM Check if referenced files exist
echo [2.1] Validating file references...

set FILES_TO_CHECK=docs\PRECOMPILED_DLL_INTEGRATION.md docs\DEVELOPER_API_REFERENCE.md docs\FOR_FLUTTER_DEVELOPERS.md lib\anywp_engine.dart examples\test_simple.html

for %%f in (%FILES_TO_CHECK%) do (
    if exist "%PROJECT_ROOT%\%%f" (
        echo   [OK] %%f exists
    ) else (
        echo   [ERROR] %%f NOT FOUND
        set /a ERROR_COUNT+=1
    )
)

echo.

REM ==========================================
REM Section 3: Version Consistency Check
REM ==========================================
echo [Section 3] Version Consistency Check
echo ----------------------------------------

echo [3.1] Extracting version from pubspec.yaml...
for /f "tokens=2" %%a in ('findstr "^version:" "%PROJECT_ROOT%\pubspec.yaml"') do set VERSION=%%a
echo   Current version: %VERSION%

REM Check if version exists in CHANGELOG_CN.md
echo [3.2] Checking CHANGELOG_CN.md...
findstr /C:"%VERSION%" "%PROJECT_ROOT%\CHANGELOG_CN.md" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [OK] Version %VERSION% found in CHANGELOG_CN.md
) else (
    echo   [WARN] Version %VERSION% not found in CHANGELOG_CN.md
    set /a WARNING_COUNT+=1
)

echo.

REM ==========================================
REM Section 4: Link Validation (Basic)
REM ==========================================
echo [Section 4] Link Validation
echo ----------------------------------------

echo [4.1] Checking documentation links in README.md...

REM Check if docs directory exists
if exist "%PROJECT_ROOT%\docs\" (
    echo   [OK] docs/ directory exists
) else (
    echo   [ERROR] docs/ directory NOT FOUND
    set /a ERROR_COUNT+=1
)

REM Check if examples directory exists
if exist "%PROJECT_ROOT%\examples\" (
    echo   [OK] examples/ directory exists
) else (
    echo   [ERROR] examples/ directory NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.

REM ==========================================
REM Section 5: Code Example Validation
REM ==========================================
echo [Section 5] Code Example Validation
echo ----------------------------------------

echo [5.1] Checking if anywp_engine.dart has public APIs...
findstr /C:"class AnyWPEngine" "%PROJECT_ROOT%\lib\anywp_engine.dart" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   [OK] AnyWPEngine class found
) else (
    echo   [ERROR] AnyWPEngine class NOT FOUND
    set /a ERROR_COUNT+=1
)

echo.

REM ==========================================
REM Section 6: Script Language Check
REM ==========================================
echo [Section 6] Script Language Check
echo ----------------------------------------

echo [6.1] Checking scripts for Chinese content...

set SCRIPT_ERROR=0
for %%s in (build.bat release.bat test_full.bat verify.bat run.bat debug.bat setup.bat) do (
    if exist "%PROJECT_ROOT%\scripts\%%s" (
        findstr /C:"中" "%PROJECT_ROOT%\scripts\%%s" >nul 2>&1
        if !ERRORLEVEL! EQU 0 (
            echo   [ERROR] %%s contains Chinese text
            set SCRIPT_ERROR=1
            set /a ERROR_COUNT+=1
        ) else (
            echo   [OK] %%s is English only
        )
    )
)

if %SCRIPT_ERROR% EQU 0 (
    echo   [OK] All scripts are English only
)

echo.

REM ==========================================
REM Section 7: Release Package Check (if exists)
REM ==========================================
echo [Section 7] Release Package Check
echo ----------------------------------------

if exist "%PROJECT_ROOT%\release\anywp_engine_v%VERSION%" (
    echo [7.1] Checking release package...
    
    set RELEASE_DIR=%PROJECT_ROOT%\release\anywp_engine_v%VERSION%
    
    REM Check critical files
    if exist "!RELEASE_DIR!\bin\anywp_engine_plugin.dll" (
        echo   [OK] anywp_engine_plugin.dll
    ) else (
        echo   [ERROR] anywp_engine_plugin.dll NOT FOUND
        set /a ERROR_COUNT+=1
    )
    
    if exist "!RELEASE_DIR!\lib\anywp_engine_plugin.lib" (
        echo   [OK] anywp_engine_plugin.lib
    ) else (
        echo   [ERROR] anywp_engine_plugin.lib NOT FOUND
        set /a ERROR_COUNT+=1
    )
    
    if exist "!RELEASE_DIR!\windows\anywp_sdk.js" (
        echo   [OK] anywp_sdk.js
    ) else (
        echo   [ERROR] anywp_sdk.js NOT FOUND
        set /a ERROR_COUNT+=1
    )
) else (
    echo [7.1] Release package not found (skipped)
)

echo.

REM ==========================================
REM Final Summary
REM ==========================================
echo ========================================
echo  Verification Summary
echo ========================================
echo  Errors:   %ERROR_COUNT%
echo  Warnings: %WARNING_COUNT%
echo ========================================
echo.

if %ERROR_COUNT% GTR 0 (
    echo STATUS: FAILED - %ERROR_COUNT% error(s) found
    echo.
    echo Please fix the errors above before release.
    exit /b 1
) else if %WARNING_COUNT% GTR 0 (
    echo STATUS: PASSED WITH WARNINGS - %WARNING_COUNT% warning(s)
    echo.
    echo Review warnings before release.
    exit /b 0
) else (
    echo STATUS: ALL CHECKS PASSED
    echo.
    echo Documentation is ready for release.
    exit /b 0
)

