@echo off
setlocal enabledelayedexpansion
REM ==========================================
REM AnyWP Engine - Full Test Suite
REM Automated testing with performance monitoring
REM ==========================================

set "PROJECT_ROOT=%~dp0.."
set "LOG_DIR=%PROJECT_ROOT%\test_logs"
set "TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "TIMESTAMP=%TIMESTAMP: =0%"
set "TEST_REPORT=%LOG_DIR%\test_full_%TIMESTAMP%.log"
set "MEMORY_LOG=%LOG_DIR%\memory_%TIMESTAMP%.csv"
set "CPU_LOG=%LOG_DIR%\cpu_%TIMESTAMP%.csv"
set "BUILD_DIR=%PROJECT_ROOT%\example\build\windows\x64\runner\Debug"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

echo ================================
echo  AnyWP Engine - Full Test Suite
echo ================================
echo.
echo Test Time: %date% %time%
echo Log Directory: %LOG_DIR%
echo.

REM Initialize report
echo AnyWP Engine - Full Test Report > "%TEST_REPORT%"
echo Test Time: %date% %time% >> "%TEST_REPORT%"
echo ==================================== >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

REM Step 1: Clean environment
echo [Step 1/10] Cleaning environment...
echo [Step 1/10] Cleaning environment >> "%TEST_REPORT%"
taskkill /F /IM anywallpaper_engine_example.exe 2>nul
call :Sleep 2
echo Done.
echo.

REM Step 2: Build project
echo [Step 2/10] Building project (Debug)...
echo [Step 2/10] Building project >> "%TEST_REPORT%"
cd "%PROJECT_ROOT%\example"
call flutter pub get > "%LOG_DIR%\pub_get_%TIMESTAMP%.log" 2>&1
if errorlevel 1 (
    echo ERROR: Flutter pub get failed. Check pub_get_%TIMESTAMP%.log
    echo ERROR: Flutter pub get failed >> "%TEST_REPORT%"
    pause
    exit /b 1
)
call flutter build windows --debug --no-pub > "%LOG_DIR%\build_%TIMESTAMP%.log" 2>&1
if errorlevel 1 (
    echo ERROR: Build failed. Check build_%TIMESTAMP%.log
    echo ERROR: Build failed >> "%TEST_REPORT%"
    pause
    exit /b 1
)
echo Build successful
echo Build successful >> "%TEST_REPORT%"
echo.

REM Step 3: SDK Verification (v2.3.0+: SDK is embedded in DLL)
echo [Step 3/10] Verifying SDK embedding...
echo SDK is embedded in DLL (no external files needed)
echo Done.
echo.

REM Step 4: Create monitoring scripts
echo [Step 4/10] Creating monitoring scripts...

REM Memory monitor
(
echo $process = Get-Process anywallpaper_engine_example -ErrorAction SilentlyContinue
echo if ($process^) {
echo   $now = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
echo   $ws = [math]::Round($process.WorkingSet64 / 1MB, 2^)
echo   $pb = [math]::Round($process.PrivateMemorySize64 / 1MB, 2^)
echo   $vm = [math]::Round($process.VirtualMemorySize64 / 1MB, 2^)
echo   "$now,$ws,$pb,$vm"
echo }
) > "%LOG_DIR%\monitor_memory.ps1"

REM CPU monitor
(
echo $process = Get-Process anywallpaper_engine_example -ErrorAction SilentlyContinue
echo if ($process^) {
echo   $now = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
echo   $cpu = $process.CPU
echo   $cpu = [math]::Round($cpu, 2^)
echo   $threads = $process.Threads.Count
echo   $handles = $process.HandleCount
echo   "$now,$cpu,$threads,$handles"
echo }
) > "%LOG_DIR%\monitor_cpu.ps1"

echo Memory,WorkingSet(MB^),PrivateBytes(MB^),VirtualMemory(MB^) > "%MEMORY_LOG%"
echo CPU,CPUSeconds,Threads,Handles > "%CPU_LOG%"
echo Done.
echo.

REM Step 5: Start application
echo [Step 5/10] Starting application...
start "" "%BUILD_DIR%\anywallpaper_engine_example.exe" > "%LOG_DIR%\app_output_%TIMESTAMP%.log" 2>&1
call :Sleep 5
echo Application started
echo.

REM Step 6: Start monitoring
echo [Step 6/10] Starting performance monitoring...
start /MIN powershell -NoProfile -ExecutionPolicy Bypass -Command "while($true) { & '%LOG_DIR%\monitor_memory.ps1' | Add-Content -Path '%MEMORY_LOG%'; Start-Sleep 1 }"
start /MIN powershell -NoProfile -ExecutionPolicy Bypass -Command "while($true) { & '%LOG_DIR%\monitor_cpu.ps1' | Add-Content -Path '%CPU_LOG%'; Start-Sleep 1 }"
echo Monitoring started
echo.

REM Step 7: Run test sequence
echo [Step 7/10] Running test sequence (4 pages, ~50 seconds^)...
echo [Step 7/10] Running test sequence >> "%TEST_REPORT%"

set /a TEST_NUM=1
for %%p in (test_visibility.html test_api.html test_basic_click.html test_carousel_control.html) do (
    echo   [!TEST_NUM!/4] Testing: %%p
    echo   [!TEST_NUM!/4] Testing: %%p >> "%TEST_REPORT%"
    call :Sleep 12
    set /a TEST_NUM+=1
)
echo Test sequence complete
echo.

REM Step 8: Stop application
echo [Step 8/10] Stopping application...
call :Sleep 3
taskkill /F /IM anywallpaper_engine_example.exe 2>nul
echo Done.
echo.

REM Step 9: Stop monitoring
echo [Step 9/10] Stopping monitoring...
taskkill /F /IM powershell.exe 2>nul
call :Sleep 2
echo Done.
echo.

REM Step 10: Generate summary
echo [Step 10/10] Generating summary...
echo. >> "%TEST_REPORT%"
echo Test Sequence Complete >> "%TEST_REPORT%"
echo - Memory log: memory_%TIMESTAMP%.csv >> "%TEST_REPORT%"
echo - CPU log: cpu_%TIMESTAMP%.csv >> "%TEST_REPORT%"
echo - Build log: build_%TIMESTAMP%.log >> "%TEST_REPORT%"
echo - App output: app_output_%TIMESTAMP%.log >> "%TEST_REPORT%"
echo. >> "%TEST_REPORT%"

echo.
echo ================================
echo  Test Complete!
echo ================================
echo.
echo Test Report: %TEST_REPORT%
echo Memory Log: %MEMORY_LOG%
echo CPU Log: %CPU_LOG%
echo.
echo Run 'scripts\analyze.ps1' to analyze results
echo.
pause
exit /b 0

:Sleep
powershell -NoProfile -Command "Start-Sleep -Seconds %~1" >nul
exit /b 0
