@echo off
echo ==================================
echo  AnyWP Engine - Performance Test
echo ==================================
echo.

echo [INFO] Starting application...
cd /d "%~dp0.."

REM Start process and get PID
start "" "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
timeout /t 3 /nobreak >nul

echo [INFO] Collecting performance data for 15 seconds...
echo.

REM Use PowerShell to monitor
powershell -Command "$proc = Get-Process -Name 'anywallpaper_engine_example' -ErrorAction SilentlyContinue; if ($proc) { Write-Host '[OK] Process found: PID' $proc.Id -ForegroundColor Green; Write-Host ''; Write-Host 'Collecting samples...' -ForegroundColor Yellow; $samples = @(); for ($i=0; $i -lt 15; $i++) { Start-Sleep -Seconds 1; $proc.Refresh(); $mem = [math]::Round($proc.WorkingSet64 / 1MB, 2); $samples += $mem; Write-Host \"  Sample $($i+1): Memory = $mem MB\" -ForegroundColor Gray }; $avg = [math]::Round(($samples | Measure-Object -Average).Average, 2); $max = [math]::Round(($samples | Measure-Object -Maximum).Maximum, 2); $min = [math]::Round(($samples | Measure-Object -Minimum).Minimum, 2); Write-Host ''; Write-Host '=== Results ===' -ForegroundColor Cyan; Write-Host \"Memory (Avg):  $avg MB\" -ForegroundColor White; Write-Host \"Memory (Peak): $max MB\" -ForegroundColor White; Write-Host \"Memory (Min):  $min MB\" -ForegroundColor White; \"PERF_DATA:$avg|$max|$min\" | Out-File -FilePath 'perf_data.txt' -Encoding ASCII } else { Write-Host '[ERROR] Process not found' -ForegroundColor Red }"

echo.
echo [INFO] Test complete. Process still running for manual testing.
echo Press any key to stop the application...
pause >nul

taskkill /F /IM anywallpaper_engine_example.exe >nul 2>&1

