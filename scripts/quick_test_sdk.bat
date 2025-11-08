@echo off
REM Quick SDK Test - Open test pages in browser
setlocal

echo ========================================
echo AnyWP SDK - Quick Browser Test
echo ========================================
echo.
echo Built SDK version: TypeScript (100%%)
echo Location: windows\anywp_sdk.js
echo.

REM Get absolute path
set ROOT_DIR=%~dp0..
cd /d "%ROOT_DIR%"

echo Opening test pages in browser...
echo.
echo Test Pages:
echo   1. test_draggable.html - Drag and drop functionality
echo   2. test_api.html - Complete API demo
echo   3. test_react.html - React SPA support
echo   4. test_sdk_browser.html - Standalone SDK test
echo.

REM Start local server using Python (if available)
where python >nul 2>&1
if %errorlevel% equ 0 (
    echo Starting local server on http://localhost:8000
    echo.
    echo Open these URLs in your browser:
    echo   http://localhost:8000/examples/test_draggable.html
    echo   http://localhost:8000/examples/test_api.html
    echo   http://localhost:8000/examples/test_react.html
    echo   http://localhost:8000/examples/test_sdk_browser.html
    echo.
    echo Press Ctrl+C to stop server
    echo ========================================
    python -m http.server 8000
) else (
    echo Python not found. Opening files directly...
    echo.
    echo NOTE: Some features require a web server (CORS, localStorage)
    echo For full testing, install Python or use VS Code Live Server
    echo.
    
    REM Open in default browser
    start "" "examples\test_sdk_browser.html"
    timeout /t 2 >nul
    start "" "examples\test_draggable.html"
    
    echo.
    echo Test pages opened in browser.
    echo ========================================
)

endlocal

