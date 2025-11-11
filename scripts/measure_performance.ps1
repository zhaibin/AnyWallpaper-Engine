# Performance Measurement Script for AnyWP Engine
# Measures startup time, memory usage, and CPU usage

param(
    [int]$Duration = 30  # Measurement duration in seconds
)

$ErrorActionPreference = "Continue"

Write-Host "==================================" -ForegroundColor Cyan
Write-Host " AnyWP Engine - Performance Test" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Find executable
$ExePath = "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "[ERROR] Executable not found: $ExePath" -ForegroundColor Red
    Write-Host "Please run: flutter build windows --debug" -ForegroundColor Yellow
    exit 1
}

Write-Host "[INFO] Executable: $ExePath" -ForegroundColor Green
Write-Host "[INFO] Measurement duration: $Duration seconds" -ForegroundColor Green
Write-Host ""

# Stop any existing instance
Stop-Process -Name "anywallpaper_engine_example" -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 1

# Measure startup time
Write-Host "[1/4] Measuring startup time..." -ForegroundColor Yellow
$StartTime = Get-Date

$Process = Start-Process -FilePath $ExePath -WorkingDirectory $PWD -PassThru -WindowStyle Hidden
Start-Sleep -Milliseconds 100

# Wait for process to be responsive (max 10 seconds)
$Timeout = 10
$Elapsed = 0
while ($Elapsed -lt $Timeout) {
    try {
        if ($Process.Responding) {
            break
        }
    } catch {}
    Start-Sleep -Milliseconds 100
    $Elapsed += 0.1
}

$StartupTime = ((Get-Date) - $StartTime).TotalMilliseconds
Write-Host "[OK] Startup time: $([math]::Round($StartupTime, 0)) ms" -ForegroundColor Green

# Wait a bit for full initialization
Start-Sleep -Seconds 3

# Measure memory usage (baseline)
Write-Host ""
Write-Host "[2/4] Measuring memory usage..." -ForegroundColor Yellow

$MemorySamples = @()
$CpuSamples = @()

for ($i = 0; $i -lt $Duration; $i++) {
    try {
        $Process.Refresh()
        $MemoryMB = [math]::Round($Process.WorkingSet64 / 1MB, 2)
        $CpuPercent = [math]::Round($Process.CPU / (Get-Date - $Process.StartTime).TotalSeconds, 2)
        
        $MemorySamples += $MemoryMB
        $CpuSamples += $CpuPercent
        
        if ($i % 5 -eq 0) {
            Write-Host "  Sample $($i+1)/$Duration - Memory: $MemoryMB MB, CPU: $CpuPercent %" -ForegroundColor Gray
        }
    } catch {
        Write-Host "  [WARN] Failed to get process info" -ForegroundColor Yellow
    }
    
    Start-Sleep -Seconds 1
}

# Calculate statistics
$AvgMemory = [math]::Round(($MemorySamples | Measure-Object -Average).Average, 2)
$MaxMemory = [math]::Round(($MemorySamples | Measure-Object -Maximum).Maximum, 2)
$MinMemory = [math]::Round(($MemorySamples | Measure-Object -Minimum).Minimum, 2)

$AvgCpu = [math]::Round(($CpuSamples | Measure-Object -Average).Average, 2)
$MaxCpu = [math]::Round(($CpuSamples | Measure-Object -Maximum).Maximum, 2)

Write-Host ""
Write-Host "[3/4] Memory Statistics:" -ForegroundColor Yellow
Write-Host "  Average: $AvgMemory MB" -ForegroundColor White
Write-Host "  Peak:    $MaxMemory MB" -ForegroundColor White
Write-Host "  Minimum: $MinMemory MB" -ForegroundColor White

Write-Host ""
Write-Host "[4/4] CPU Statistics:" -ForegroundColor Yellow
Write-Host "  Average: $AvgCpu %" -ForegroundColor White
Write-Host "  Peak:    $MaxCpu %" -ForegroundColor White

# Stop process
Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue

# Generate report
Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host " Performance Summary" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Startup Time:    $([math]::Round($StartupTime, 0)) ms" -ForegroundColor White
Write-Host "Memory (Avg):    $AvgMemory MB" -ForegroundColor White
Write-Host "Memory (Peak):   $MaxMemory MB" -ForegroundColor White
Write-Host "CPU (Idle Avg):  $AvgCpu %" -ForegroundColor White
Write-Host "CPU (Peak):      $MaxCpu %" -ForegroundColor White
Write-Host ""

# Save to file
$Report = @"
# AnyWP Engine - Performance Test Report
Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Test Configuration
- Build: Debug
- Duration: $Duration seconds
- Samples: $($MemorySamples.Count)

## Results
- Startup Time: $([math]::Round($StartupTime, 0)) ms
- Memory (Average): $AvgMemory MB
- Memory (Peak): $MaxMemory MB
- Memory (Minimum): $MinMemory MB
- CPU (Idle Average): $AvgCpu %
- CPU (Peak): $MaxCpu %

## Raw Data
Memory Samples (MB): $($MemorySamples -join ', ')
CPU Samples (%): $($CpuSamples -join ', ')
"@

$Report | Out-File -FilePath "performance_report.txt" -Encoding UTF8
Write-Host "[OK] Report saved to: performance_report.txt" -ForegroundColor Green
Write-Host ""

# Return data for scripting
return @{
    StartupTime = [math]::Round($StartupTime, 0)
    MemoryAvg = $AvgMemory
    MemoryPeak = $MaxMemory
    CpuAvg = $AvgCpu
    CpuPeak = $MaxCpu
}

