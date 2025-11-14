@echo off
echo Verifying AnyWP Engine precompiled package...
echo.
set ERROR_COUNT=0
REM Check required DLL files
if not exist "bin\anywp_engine_plugin.dll" (
    echo [ERROR] Missing: bin\anywp_engine_plugin.dll
    set /a ERROR_COUNT+=1
)
if not exist "bin\WebView2Loader.dll" (
    echo [ERROR] Missing: bin\WebView2Loader.dll
    set /a ERROR_COUNT+=1
)
REM Check LIB file
if not exist "lib\anywp_engine_plugin.lib" (
    echo [ERROR] Missing: lib\anywp_engine_plugin.lib
    set /a ERROR_COUNT+=1
)
REM Check Dart file (standard location)
if not exist "lib\anywp_engine.dart" (
    echo [ERROR] Missing: lib\anywp_engine.dart
    set /a ERROR_COUNT+=1
)
REM Check header files
if not exist "include\anywp_engine\anywp_engine_plugin_c_api.h" (
    echo [ERROR] Missing: include\anywp_engine\anywp_engine_plugin_c_api.h
    set /a ERROR_COUNT+=1
)
if not exist "include\anywp_engine\any_w_p_engine_plugin.h" (
    echo [ERROR] Missing: include\anywp_engine\any_w_p_engine_plugin.h
    set /a ERROR_COUNT+=1
)
REM Check CMakeLists.txt
if not exist "windows\CMakeLists.txt" (
    echo [ERROR] Missing: windows\CMakeLists.txt
    set /a ERROR_COUNT+=1
)
REM Verify CMakeLists.txt contains GLOBAL
findstr /C:"IMPORTED GLOBAL" "windows\CMakeLists.txt" >nul
if errorlevel 1 (
    echo [WARNING] CMakeLists.txt may be missing GLOBAL keyword
)
echo.
if %ERROR_COUNT%==0 (
    echo [SUCCESS] All required files are present
    echo Package is ready to use.
) else (
    echo [FAILED] Found %ERROR_COUNT% error(s).
    echo Please re-download the package.
)
pause
