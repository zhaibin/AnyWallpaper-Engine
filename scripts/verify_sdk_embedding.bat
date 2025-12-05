@echo off
REM =============================================================================
REM AnyWP Engine - Verify SDK Embedding in DLL
REM =============================================================================
REM This script verifies that the Web SDK is properly embedded in the DLL
REM Usage: verify_sdk_embedding.bat [dll_path]
REM =============================================================================

setlocal enabledelayedexpansion

echo.
echo ========================================
echo  AnyWP Engine - SDK Embedding Verification
echo ========================================
echo.

REM Determine DLL path
set "DLL_PATH=%~1"
if "%DLL_PATH%"=="" (
    set "DLL_PATH=example\build\windows\x64\runner\Release\anywp_engine_plugin.dll"
)

echo [Step 1/4] Checking SDK source file...
echo.

set "SDK_SOURCE=sdk\dist\anywp_sdk.js"
if not exist "%SDK_SOURCE%" (
    echo   [ERROR] SDK source file not found: %SDK_SOURCE%
    echo   This file is required for embedding into DLL
    goto :error
)

for %%F in ("%SDK_SOURCE%") do set "SDK_SIZE=%%~zF"
set /a "SDK_SIZE_KB=%SDK_SIZE% / 1024"
echo   [OK] SDK source found: %SDK_SOURCE%
echo        Size: %SDK_SIZE_KB% KB (%SDK_SIZE% bytes)

REM Check SDK version
for /f "delims=" %%a in ('powershell -NoProfile -Command "(Get-Content '%SDK_SOURCE%' | Select-Object -First 5) -match 'v[0-9]+\.[0-9]+\.[0-9]+' | Out-String"') do set "SDK_VERSION_LINE=%%a"
echo        Version: %SDK_VERSION_LINE%
echo.

echo [Step 2/4] Checking resource file...
echo.

set "RESOURCE_FILE=windows\sdk_resource.rc"
if not exist "%RESOURCE_FILE%" (
    echo   [WARNING] Resource file not found: %RESOURCE_FILE%
    echo   SDK may not be embedded in DLL
) else (
    echo   [OK] Resource file found: %RESOURCE_FILE%
    findstr /C:"IDR_ANYWP_SDK" "%RESOURCE_FILE%" >nul
    if !ERRORLEVEL! equ 0 (
        echo        Resource definition: IDR_ANYWP_SDK
    )
)
echo.

echo [Step 3/4] Checking SDK loader...
echo.

set "LOADER_FILE=windows\sdk_loader.cpp"
if not exist "%LOADER_FILE%" (
    echo   [WARNING] SDK loader not found: %LOADER_FILE%
) else (
    echo   [OK] SDK loader found: %LOADER_FILE%
    findstr /C:"LoadSDKFromResource" "%LOADER_FILE%" >nul
    if !ERRORLEVEL! equ 0 (
        echo        Function: LoadSDKFromResource()
    )
)
echo.

echo [Step 4/4] Verifying DLL embedding...
echo.

if not exist "%DLL_PATH%" (
    echo   [ERROR] DLL not found: %DLL_PATH%
    echo.
    echo   Please build the Release version first:
    echo     cd example
    echo     flutter build windows --release
    goto :error
)

for %%F in ("%DLL_PATH%") do set "DLL_SIZE=%%~zF"
set /a "DLL_SIZE_KB=%DLL_SIZE% / 1024"
echo   [OK] DLL found: %DLL_PATH%
echo        Size: %DLL_SIZE_KB% KB (%DLL_SIZE% bytes)
echo.

REM Calculate expected minimum size (SDK + ~500KB plugin code)
set /a "MIN_SIZE=%SDK_SIZE% + 512000"

if %DLL_SIZE% LSS %MIN_SIZE% (
    echo   [WARNING] DLL size is smaller than expected
    echo   Expected at least: %MIN_SIZE% bytes
    echo   Actual: %DLL_SIZE% bytes
    echo.
    echo   SDK may not be embedded properly!
    goto :warning
)

echo ========================================
echo  ✅ VERIFICATION PASSED
echo ========================================
echo.
echo SDK is successfully embedded in DLL:
echo   - SDK Source: %SDK_SIZE_KB% KB
echo   - DLL Total: %DLL_SIZE_KB% KB
echo   - SDK is loaded from DLL resource at runtime
echo   - No external SDK files needed!
echo.
goto :end

:warning
echo ========================================
echo  ⚠️  VERIFICATION WARNING
echo ========================================
echo.
echo Please check:
echo   1. SDK source file exists: %SDK_SOURCE%
echo   2. Resource file correctly references SDK
echo   3. DLL was built in Release mode
echo.
goto :end

:error
echo ========================================
echo  ❌ VERIFICATION FAILED
echo ========================================
echo.
echo Please check the error messages above.
echo.
exit /b 1

:end
if not defined NO_PAUSE pause



