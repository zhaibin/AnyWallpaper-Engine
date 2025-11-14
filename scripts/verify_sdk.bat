@echo off
REM Verify SDK files in build directories
setlocal enabledelayedexpansion

echo ========================================
echo  SDK Version Verification
echo ========================================
echo.

echo [Source Files]
if exist "windows\anywp_sdk.js" (
    for %%A in ("windows\anywp_sdk.js") do echo   anywp_sdk.js     : %%~zA bytes ^(unminified^)
) else (
    echo   anywp_sdk.js     : MISSING
)

if exist "windows\anywp_sdk.min.js" (
    for %%A in ("windows\anywp_sdk.min.js") do echo   anywp_sdk.min.js : %%~zA bytes ^(minified^)
) else (
    echo   anywp_sdk.min.js : MISSING
)

echo.
echo [Debug Build]
if exist "example\build\windows\x64\runner\Debug\windows\anywp_sdk.js" (
    for %%A in ("example\build\windows\x64\runner\Debug\windows\anywp_sdk.js") do echo   √ SDK Found: %%~zA bytes
) else (
    echo   × SDK Missing
)

echo.
echo [Release Build]
if exist "example\build\windows\x64\runner\Release\windows\anywp_sdk.js" (
    for %%A in ("example\build\windows\x64\runner\Release\windows\anywp_sdk.js") do echo   √ SDK Found: %%~zA bytes
) else (
    echo   × SDK Missing
)

echo.
echo ========================================

endlocal

