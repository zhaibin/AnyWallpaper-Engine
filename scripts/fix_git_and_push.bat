@echo off
setlocal enabledelayedexpansion

cd /d E:\Projects\AnyWallpaper\AnyWallpaper-Engine

echo [1/6] Removing corrupt references...
del /q ".git\refs\heads\feature\modularity-enhancement" 2>nul
del /q ".git\logs\refs\heads\feature\modularity-enhancement" 2>nul

echo [2/6] Fetching from origin...
git fetch origin main

echo [3/6] Checking if we can push using --force-with-lease...
git push origin main --force-with-lease

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Code pushed successfully
    goto PUSH_TAG
)

echo [4/6] Force push failed, trying alternative approach...
echo [5/6] Resetting to last known good remote state and reapplying changes...

REM Save current commit
for /f "tokens=1" %%a in ('git rev-parse HEAD') do set CURRENT_COMMIT=%%a
echo Current commit: %CURRENT_COMMIT%

REM Reset to origin/main
git reset --soft origin/main

REM Create new commit
echo release: 发布 v2.0.0 - 模块化架构重构 > commit_msg_new.txt
echo. >> commit_msg_new.txt
echo 重大架构升级 >> commit_msg_new.txt
git commit -F commit_msg_new.txt
del commit_msg_new.txt

echo [6/6] Pushing new commit...
git push origin main

:PUSH_TAG
echo.
echo [Pushing tag v2.0.0]
git push origin v2.0.0

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] All done! Code and tag pushed to GitHub
) else (
    echo.
    echo [WARNING] Code pushed but tag push failed
    echo You can manually push the tag later: git push origin v2.0.0
)

pause

