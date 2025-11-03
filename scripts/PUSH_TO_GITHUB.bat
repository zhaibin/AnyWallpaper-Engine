@echo off
chcp 65001 >nul 2>&1
echo ================================
echo  Push to GitHub
echo ================================
echo.

REM Check git status
git status --short
if %errorlevel% neq 0 (
    echo ERROR: Not a git repository!
    pause
    exit /b 1
)

echo.
echo Recent commits:
git log --oneline -5

echo.
set /p CONFIRM="Push to GitHub? (y/n): "
if /i not "%CONFIRM%"=="y" (
    echo Cancelled.
    exit /b 0
)

echo.
echo Pushing to origin/main...
git push -u origin main

if %errorlevel% neq 0 (
    echo.
    echo Push failed! Check:
    echo   - Remote repository is set: git remote -v
    echo   - Branch exists: git branch
    echo   - Network connection
    pause
    exit /b 1
)

echo.
echo ================================
echo  Successfully pushed!
echo ================================

