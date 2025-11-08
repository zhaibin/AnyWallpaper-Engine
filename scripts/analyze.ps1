# ==========================================
# AnyWP Engine - Test Results Analyzer
# Analyze memory, CPU, and logs
# ==========================================

param(
    [switch]$GenerateHTML
)

$ErrorActionPreference = "Stop"

Write-Host "========================================"
Write-Host "AnyWP Engine - Test Results Analyzer"
Write-Host "========================================"
Write-Host ""

# Find latest test files
$logDir = Join-Path $PSScriptRoot "..\test_logs"
if (!(Test-Path $logDir)) {
    Write-Host "ERROR: test_logs directory not found"
    Write-Host "Please run test_full.bat first"
    exit 1
}

$memoryLog = Get-ChildItem "$logDir\memory_*.csv" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$cpuLog = Get-ChildItem "$logDir\cpu_*.csv" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$testReport = Get-ChildItem "$logDir\test_full_*.log" | Sort-Object LastWriteTime -Descending | Select-Object -First 1

if (!$memoryLog -or !$cpuLog) {
    Write-Host "ERROR: Test logs not found"
    exit 1
}

Write-Host "Analyzing files:"
Write-Host "  - Memory: $($memoryLog.Name)"
Write-Host "  - CPU: $($cpuLog.Name)"
if ($testReport) {
    Write-Host "  - Report: $($testReport.Name)"
}
Write-Host ""

# Analyze memory
Write-Host "[1/5] Analyzing memory data..."
$memoryData = Import-Csv $memoryLog.FullName -Header "Time","WorkingSet","PrivateBytes","VirtualMemory" | Select-Object -Skip 1
$maxWS = ($memoryData.WorkingSet | Measure-Object -Maximum).Maximum
$avgWS = ($memoryData.WorkingSet | Measure-Object -Average).Average
$minWS = ($memoryData.WorkingSet | Measure-Object -Minimum).Minimum
$growth = [math]::Round((($maxWS - $minWS) / $minWS) * 100, 2)

Write-Host "Memory Analysis:"
Write-Host "  - Max Working Set: $([math]::Round($maxWS, 2)) MB"
Write-Host "  - Avg Working Set: $([math]::Round($avgWS, 2)) MB"
Write-Host "  - Memory Growth: $growth%"
Write-Host ""

# Analyze CPU
Write-Host "[2/5] Analyzing CPU data..."
$cpuData = Import-Csv $cpuLog.FullName -Header "Time","CPU","Threads","Handles" | Select-Object -Skip 1
$maxCPU = ($cpuData.CPU | Measure-Object -Maximum).Maximum
$avgCPU = ($cpuData.CPU | Measure-Object -Average).Average
$maxThreads = ($cpuData.Threads | Measure-Object -Maximum).Maximum

Write-Host "CPU Analysis:"
Write-Host "  - Max CPU: $([math]::Round($maxCPU, 2))%"
Write-Host "  - Avg CPU: $([math]::Round($avgCPU, 2))%"
Write-Host "  - Max Threads: $maxThreads"
Write-Host ""

# Performance score
Write-Host "[3/5] Calculating performance score..."
$score = 100

if ($maxWS -gt 300) { $score -= 30 } elseif ($maxWS -gt 250) { $score -= 10 }
if ($growth -gt 10) { $score -= 20 } elseif ($growth -gt 5) { $score -= 10 }
if ($avgCPU -gt 15) { $score -= 20 } elseif ($avgCPU -gt 10) { $score -= 10 }

Write-Host "Performance Score: $score/100"
if ($score -ge 90) {
    Write-Host "Rating: Excellent" -ForegroundColor Green
} elseif ($score -ge 70) {
    Write-Host "Rating: Good" -ForegroundColor Yellow
} else {
    Write-Host "Rating: Needs Improvement" -ForegroundColor Red
}
Write-Host ""

# Check logs
Write-Host "[4/5] Checking log integrity..."
if ($testReport) {
    $content = Get-Content $testReport.FullName -Raw
    $hasErrors = $content -match "ERROR|Failed"
    if ($hasErrors) {
        Write-Host "WARNING: Errors found in test report" -ForegroundColor Yellow
    } else {
        Write-Host "Log integrity: OK" -ForegroundColor Green
    }
}
Write-Host ""

# Generate HTML report
if ($GenerateHTML) {
    Write-Host "[5/5] Generating HTML report..."
    $htmlFile = Join-Path $logDir "performance_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"
    
    $html = @"
<!DOCTYPE html>
<html>
<head>
    <title>AnyWP Engine - Performance Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background: #2196F3; color: white; padding: 20px; border-radius: 5px; }
        .section { margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }
        .metric { display: inline-block; margin: 10px; padding: 10px; background: #f5f5f5; border-radius: 3px; }
        .score { font-size: 48px; font-weight: bold; color: $(if($score -ge 90){'#4CAF50'}elseif($score -ge 70){'#FF9800'}else{'#F44336'}); }
    </style>
</head>
<body>
    <div class="header">
        <h1>AnyWP Engine - Performance Report</h1>
        <p>Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</p>
    </div>
    
    <div class="section">
        <h2>Performance Score</h2>
        <div class="score">$score / 100</div>
    </div>
    
    <div class="section">
        <h2>Memory Metrics</h2>
        <div class="metric">Max Working Set<br><strong>$([math]::Round($maxWS, 2)) MB</strong></div>
        <div class="metric">Avg Working Set<br><strong>$([math]::Round($avgWS, 2)) MB</strong></div>
        <div class="metric">Memory Growth<br><strong>$growth%</strong></div>
    </div>
    
    <div class="section">
        <h2>CPU Metrics</h2>
        <div class="metric">Max CPU<br><strong>$([math]::Round($maxCPU, 2))%</strong></div>
        <div class="metric">Avg CPU<br><strong>$([math]::Round($avgCPU, 2))%</strong></div>
        <div class="metric">Max Threads<br><strong>$maxThreads</strong></div>
    </div>
    
    <div class="section">
        <h2>Test Files</h2>
        <p>Memory Log: $($memoryLog.Name)</p>
        <p>CPU Log: $($cpuLog.Name)</p>
        <p>Test Report: $($testReport.Name)</p>
    </div>
</body>
</html>
"@
    
    $html | Out-File -FilePath $htmlFile -Encoding UTF8
    Write-Host "HTML report generated: $htmlFile" -ForegroundColor Green
    Start-Process $htmlFile
}

Write-Host ""
Write-Host "========================================"
Write-Host "Analysis Complete!"
Write-Host "========================================"

