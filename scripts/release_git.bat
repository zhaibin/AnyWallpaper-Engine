@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Release Git Automation
REM Automatically commit, tag and push release
REM ==========================================

if "%1"=="" (
    echo Usage: release_git.bat VERSION [--no-push]
    echo Example: release_git.bat 2.1.5
    echo Example: release_git.bat 2.1.5 --no-push
    exit /b 1
)

set VERSION=%1
set NO_PUSH=0
if "%2"=="--no-push" set NO_PUSH=1

set PROJECT_ROOT=%~dp0..
set RELEASE_DIR=%PROJECT_ROOT%\release
set COMMIT_MSG_FILE=%RELEASE_DIR%\commit_msg_v%VERSION%.txt

echo ========================================
echo  AnyWP Engine - Release Git Automation
echo  Version: %VERSION%
echo ========================================
echo.

REM Check if commit message file exists
if not exist "%COMMIT_MSG_FILE%" (
    echo ERROR: Commit message file not found: %COMMIT_MSG_FILE%
    echo Please run release.bat first to generate the commit template.
    exit /b 1
)

REM Check if there are changes to commit
git status --porcelain | findstr /R "." >nul
if errorlevel 1 (
    echo No changes to commit.
    goto create_tag
)

echo [Step 1/3] Staging files...
git add release/ CHANGELOG_CN.md pubspec.yaml docs/ README.md .cursorrules
if ERRORLEVEL 1 (
    echo ERROR: Failed to stage files.
    exit /b 1
)

echo [Step 2/3] Committing changes...
git commit -F "%COMMIT_MSG_FILE%"
if ERRORLEVEL 1 (
    echo ERROR: Failed to commit changes.
    exit /b 1
)
echo Commit successful.

:create_tag
echo.
echo [Step 3/3] Creating and pushing tag...
git tag -a v%VERSION% -m "AnyWP Engine v%VERSION%"
if ERRORLEVEL 1 (
    echo WARNING: Tag v%VERSION% may already exist. Skipping tag creation.
) else (
    echo Tag v%VERSION% created successfully.
)

if %NO_PUSH%==1 (
    echo.
    echo Skipping push (--no-push flag set).
    echo.
    echo To push manually, run:
    echo   git push origin main
    echo   git push origin v%VERSION%
    exit /b 0
)

echo.
echo Pushing to remote...
git push origin main
if ERRORLEVEL 1 (
    echo ERROR: Failed to push to main branch.
    exit /b 1
)

git push origin v%VERSION%
if ERRORLEVEL 1 (
    echo WARNING: Failed to push tag v%VERSION%. You may need to push it manually.
) else (
    echo Tag v%VERSION% pushed successfully.
)

echo.
echo ========================================
echo  Git Release Complete!
echo ========================================
echo.
echo Next steps:
echo   1. Visit: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
echo   2. Tag: v%VERSION%
echo   3. Title: AnyWP Engine v%VERSION%
echo   4. Description: Copy from release/GITHUB_RELEASE_NOTES_v%VERSION%.md
echo   5. Upload 3 ZIP files from release/ directory
echo   6. Check "Set as the latest release"
echo.

