@echo off
REM Monitor SDK injection logs in real-time
REM Usage: monitor_sdk_injection.bat

echo ========================================
echo SDK Injection Monitor
echo ========================================
echo.
echo Monitoring test_logs\debug_run.log for SDK injection events...
echo.
echo Looking for:
echo   - SDK loaded from
echo   - SDK registered for future pages
echo   - SDK verification: SDK loaded successfully
echo   - SDK version
echo.
echo Press Ctrl+C to stop monitoring
echo.

:loop
REM Check for SDK injection related logs
findstr /C:"SDK" /C:"Inject" /C:"anywp_sdk" /C:"SDKBridge" /C:"verification" test_logs\debug_run.log 2>nul | findstr /V /C:"Registered handler" | findstr /V /C:"Module initialized"

REM Wait 2 seconds before checking again
timeout /t 2 /nobreak >nul 2>&1
goto loop

