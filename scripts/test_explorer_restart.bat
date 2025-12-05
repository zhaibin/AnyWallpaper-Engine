@echo off
REM ============================================================================
REM AnyWP Engine - Test Explorer Restart Recovery
REM ============================================================================
REM 
REM Purpose: Test wallpaper recovery after Explorer restart
REM Version: 2.4.1
REM Date: 2025-11-19
REM 
REM Usage:
REM   test_explorer_restart.bat
REM 
REM What this script does:
REM   1. Launch the example app
REM   2. Wait for wallpaper to initialize
REM   3. Kill Explorer process
REM   4. Restart Explorer
REM   5. Monitor auto-recovery
REM 
REM Expected Result:
REM   - Wallpaper should automatically reappear after Explorer restarts
REM   - Log should show "Retrying WorkerW creation after 500ms delay"
REM   - No "Failed to find WorkerW window" error
REM 
REM ============================================================================

setlocal

set LOG_FILE=..\test_logs\explorer_restart_test_%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%_%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%.log
set LOG_FILE=%LOG_FILE: =0%

echo ============================================================================
echo AnyWP Engine - Explorer Restart Test
echo ============================================================================
echo.
echo Test Started: %DATE% %TIME%
echo Log File: %LOG_FILE%
echo.

REM Step 1: Launch example app
echo [Step 1/5] Launching example app...
cd ..\example\build\windows\x64\runner\Release
start anywallpaper_engine_example.exe

echo Waiting 10 seconds for wallpaper to initialize...
timeout /t 10 /nobreak > nul

echo.
echo [Step 2/5] Wallpaper should now be visible on desktop
echo Press SPACE to continue with Explorer restart test...
pause > nul

echo.
echo [Step 3/5] Killing Explorer...
taskkill /F /IM explorer.exe

echo Waiting 2 seconds...
timeout /t 2 /nobreak > nul

echo.
echo [Step 4/5] Restarting Explorer...
start explorer.exe

echo Waiting 3 seconds for Explorer to initialize...
timeout /t 3 /nobreak > nul

echo.
echo [Step 5/5] Monitoring auto-recovery (15 seconds)...
echo Watch the desktop - wallpaper should automatically reappear
timeout /t 15 /nobreak

echo.
echo ============================================================================
echo Test Complete
echo ============================================================================
echo.
echo Expected behavior:
echo   - Wallpaper disappeared when Explorer was killed
echo   - Wallpaper automatically reappeared after Explorer restarted
echo   - Check application logs for "Retrying WorkerW creation" message
echo.
echo If wallpaper did NOT reappear:
echo   1. Check test_logs\debug_run.log for errors
echo   2. Look for "Failed to find WorkerW window" error
echo   3. Report issue with log file
echo.
echo Press any key to close example app and exit test...
pause > nul

echo.
echo Closing example app...
taskkill /F /IM anywallpaper_engine_example.exe > nul 2>&1

echo.
echo Test finished at: %DATE% %TIME%
echo.

cd E:\Projects\AnyWallpaper\AnyWP-Test\scripts

endlocal
