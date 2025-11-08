@echo off
REM Remove manual SDK script tags from all test HTML files
REM The SDK is auto-injected by C++ plugin, manual loading causes duplicate execution

setlocal enabledelayedexpansion

echo ========================================
echo Removing Manual SDK Script Tags
echo ========================================
echo.

set "FILES_PROCESSED=0"
set "PATTERN=<script src=\"../windows/anywp_sdk.js\"></script>"
set "COMMENT=<!-- SDK is auto-injected by C++ plugin, no need to load manually -->"

for %%f in (examples\*.html) do (
    echo Processing: %%f
    
    REM Create temp file
    set "TEMP_FILE=%%f.tmp"
    
    REM Read file line by line and replace
    (
        for /f "delims=" %%a in (%%f) do (
            set "LINE=%%a"
            echo !LINE! | findstr /C:"anywp_sdk.js" >nul
            if errorlevel 1 (
                echo %%a
            ) else (
                echo     !COMMENT!
            )
        )
    ) > "!TEMP_FILE!"
    
    REM Replace original file
    move /Y "!TEMP_FILE!" "%%f" >nul
    
    set /a FILES_PROCESSED+=1
)

echo.
echo ========================================
echo Processed !FILES_PROCESSED! files
echo ========================================
echo.
echo Now rebuild the Flutter app:
echo   cd example
echo   flutter build windows --debug
echo.

pause

