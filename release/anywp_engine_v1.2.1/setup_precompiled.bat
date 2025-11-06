@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

echo ==============================================
echo  AnyWP Engine v1.2.1 棰勭紪璇戝寘瀹夎鍔╂墜
echo ==============================================
echo.

REM 妫€鏌ユ槸鍚﹀湪 Flutter 椤圭洰鏍圭洰褰曟墽琛?
if not exist "pubspec.yaml" (
    echo 鉂?閿欒锛氳鍦?Flutter 椤圭洰鏍圭洰褰曡繍琛屾鑴氭湰
    echo    渚嬪锛歠lutter_project\packages\anywp_engine_v1.2.1\setup_precompiled.bat
    echo.
    pause
    exit /b 1
)

REM 瑙ｆ瀽鐩爣 packages 鐩綍
set "PACKAGES_DIR=%cd%\packages"
if not exist "%PACKAGES_DIR%" (
    echo [1/4] 鍒涘缓 packages 鐩綍...
    mkdir "%PACKAGES_DIR%" >nul 2>&1
    if errorlevel 1 (
        echo 鉂?閿欒锛氭棤娉曞垱寤?packages 鐩綍锛?PACKAGES_DIR%
        pause
        exit /b 1
    )
)

REM 瀹氫綅棰勭紪璇戝寘鐩綍锛堣剼鏈墍鍦ㄤ綅缃級
set "SOURCE_DIR=%~dp0"
set "SOURCE_DIR=%SOURCE_DIR:~0,-1%"
set "TARGET_DIR=%PACKAGES_DIR%\anywp_engine_v1.2.1"

echo [2/4] 鍚屾棰勭紪璇戝寘鍒?packages/anywp_engine_v1.2.1 ...
if exist "%TARGET_DIR%" (
    rmdir /s /q "%TARGET_DIR%" >nul 2>&1
)
mkdir "%TARGET_DIR%" >nul 2>&1
if errorlevel 1 (
    echo 鉂?閿欒锛氭棤娉曞垱寤虹洰鏍囩洰褰曪細%TARGET_DIR%
    pause
    exit /b 1
)

robocopy "%SOURCE_DIR%" "%TARGET_DIR%" /E /NFL /NDL /NJH /NJS /NC /NS /XD ".git" >nul
if errorlevel 8 (
    echo 鉂?閿欒锛氬鍒堕缂栬瘧鍖呭け璐?
    pause
    exit /b 1
)

echo [3/4] 楠岃瘉鍏抽敭鏂囦欢...
set "ERROR_COUNT=0"

for %%F in ("
    bin\anywp_engine_plugin.dll"
    "bin\WebView2Loader.dll"
    "lib\anywp_engine_plugin.lib"
    "lib\anywp_engine.dart"
    "lib\dart\anywp_engine.dart"
    "windows\anywp_sdk.js"
    "windows\CMakeLists.txt"
) do (
    if not exist "%TARGET_DIR%%%F" (
        echo    鉂?缂哄皯鏂囦欢锛?TARGET_DIR%%%F
        set /a ERROR_COUNT+=1
    ) else (
        echo    鉁?%TARGET_DIR%%%F
    )
)

if %ERROR_COUNT% NEQ 0 (
    echo 鉂?楠岃瘉澶辫触锛氬叡鏈?%ERROR_COUNT% 涓叧閿枃浠剁己澶?
    echo    璇烽噸鏂拌В鍘嬮缂栬瘧鍖呭悗鍐嶈瘯
    pause
    exit /b 1
)

echo [4/4] 鏇存柊渚濊禆锛坒lutter pub get锛?..
flutter pub get
if errorlevel 1 (
    echo 鉂?閿欒锛歠lutter pub get 鎵ц澶辫触
    pause
    exit /b 1
)

echo.
echo ==============================================
echo 鉁?AnyWP Engine v1.2.1 棰勭紪璇戝寘宸插畨瑁呭畬鎴?
echo ==============================================
echo.
echo 浣跨敤鏂规硶锛?
echo 1. 鎵撳紑 pubspec.yaml锛屾坊鍔犱緷璧栵細
echo.
echo    dependencies:
echo      anywp_engine:
echo        path: ./packages/anywp_engine_v1.2.1
echo.
echo 2. 閲嶆柊杩愯 flutter build windows --release
echo.
pause
endlocal

