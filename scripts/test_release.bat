@echo off
setlocal enabledelayedexpansion

echo Testing release script...
echo.

REM Read version
for /f "tokens=2" %%a in ('findstr "^version:" "%~dp0..\pubspec.yaml"') do set VERSION=%%a
echo Version: %VERSION%

set PROJECT_ROOT=%~dp0..
set RELEASE_DIR=%PROJECT_ROOT%\release
set PACKAGE_DIR=%RELEASE_DIR%\anywp_engine_v%VERSION%

echo PROJECT_ROOT: %PROJECT_ROOT%
echo PACKAGE_DIR: %PACKAGE_DIR%
echo.

echo [Test 1] Building Release...
cd /d "%PROJECT_ROOT%\example"
echo Current dir: %CD%
flutter build windows --release
echo Build exit code: %ERRORLEVEL%
echo.

echo [Test 2] Returning to project root...
cd /d "%PROJECT_ROOT%"
echo Current dir: %CD%
echo.

echo [Test 3] Creating directories...
mkdir "%PACKAGE_DIR%\bin" 2>nul
if exist "%PACKAGE_DIR%\bin" (
    echo SUCCESS: bin directory created
) else (
    echo ERROR: Failed to create bin directory
)

echo.
echo Test complete
pause

