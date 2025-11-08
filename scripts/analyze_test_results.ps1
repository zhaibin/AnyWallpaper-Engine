# ========================================
# AnyWP Engine - 测试结果分析工具
# 版本: 1.0.0
# 日期: 2025-11-08
# ========================================
#
# 功能:
# - 解析性能日志并生成可视化报告
# - 分析内存泄漏趋势
# - 检测性能异常
# - 生成 HTML 报告
#
# ========================================

param(
    [string]$LogDir = "$PSScriptRoot\..\test_logs",
    [switch]$GenerateHTML = $false
)

$ErrorActionPreference = "SilentlyContinue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "AnyWP Engine - 测试结果分析工具" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 查找最新的测试日志
$memoryLogs = Get-ChildItem "$LogDir\memory_*.csv" | Sort-Object LastWriteTime -Descending
$cpuLogs = Get-ChildItem "$LogDir\cpu_*.csv" | Sort-Object LastWriteTime -Descending
$testReports = Get-ChildItem "$LogDir\comprehensive_test_*.log" | Sort-Object LastWriteTime -Descending

if ($memoryLogs.Count -eq 0 -or $cpuLogs.Count -eq 0) {
    Write-Host "❌ 未找到测试日志文件" -ForegroundColor Red
    Write-Host "请先运行: comprehensive_auto_test.bat" -ForegroundColor Yellow
    exit 1
}

$latestMemoryLog = $memoryLogs[0].FullName
$latestCpuLog = $cpuLogs[0].FullName
$latestReport = if ($testReports.Count -gt 0) { $testReports[0].FullName } else { $null }

Write-Host "📂 分析文件:" -ForegroundColor Green
Write-Host "  - 内存日志: $($memoryLogs[0].Name)" -ForegroundColor Gray
Write-Host "  - CPU日志: $($cpuLogs[0].Name)" -ForegroundColor Gray
if ($latestReport) {
    Write-Host "  - 测试报告: $($testReports[0].Name)" -ForegroundColor Gray
}
Write-Host ""

# 1. 分析内存数据
Write-Host "[1/5] 分析内存数据..." -ForegroundColor Yellow
$memoryData = Import-Csv $latestMemoryLog

if ($memoryData.Count -eq 0) {
    Write-Host "❌ 内存日志为空" -ForegroundColor Red
    exit 1
}

$wsValues = $memoryData | ForEach-Object { [double]$_.'WorkingSet(MB)' }
$pbValues = $memoryData | ForEach-Object { [double]$_.'PrivateBytes(MB)' }

$memoryStats = @{
    MaxWorkingSet = ($wsValues | Measure-Object -Maximum).Maximum
    AvgWorkingSet = [math]::Round(($wsValues | Measure-Object -Average).Average, 2)
    MinWorkingSet = ($wsValues | Measure-Object -Minimum).Minimum
    MaxPrivateBytes = ($pbValues | Measure-Object -Maximum).Maximum
    AvgPrivateBytes = [math]::Round(($pbValues | Measure-Object -Average).Average, 2)
    Samples = $memoryData.Count
}

# 检测内存增长趋势
$firstHalf = $wsValues[0..([math]::Floor($wsValues.Count / 2) - 1)]
$secondHalf = $wsValues[([math]::Floor($wsValues.Count / 2))..($wsValues.Count - 1)]
$avgFirstHalf = ($firstHalf | Measure-Object -Average).Average
$avgSecondHalf = ($secondHalf | Measure-Object -Average).Average
$memoryGrowthRate = [math]::Round((($avgSecondHalf - $avgFirstHalf) / $avgFirstHalf) * 100, 2)

$memoryStats.GrowthRate = $memoryGrowthRate

Write-Host "✅ 内存分析完成" -ForegroundColor Green
Write-Host "  - 最大工作集: $($memoryStats.MaxWorkingSet) MB" -ForegroundColor Gray
Write-Host "  - 平均工作集: $($memoryStats.AvgWorkingSet) MB" -ForegroundColor Gray
Write-Host "  - 内存增长率: $($memoryStats.GrowthRate)%" -ForegroundColor $(if ($memoryGrowthRate -gt 10) { "Red" } else { "Gray" })
Write-Host ""

# 2. 分析 CPU 数据
Write-Host "[2/5] 分析 CPU 数据..." -ForegroundColor Yellow
$cpuData = Import-Csv $latestCpuLog

if ($cpuData.Count -eq 0) {
    Write-Host "❌ CPU 日志为空" -ForegroundColor Red
    exit 1
}

$cpuValues = $cpuData | ForEach-Object { [double]$_.'CPU(%)' }
$threadValues = $cpuData | ForEach-Object { [int]$_.Threads }

$cpuStats = @{
    MaxCPU = ($cpuValues | Measure-Object -Maximum).Maximum
    AvgCPU = [math]::Round(($cpuValues | Measure-Object -Average).Average, 2)
    MaxThreads = ($threadValues | Measure-Object -Maximum).Maximum
    AvgThreads = [math]::Round(($threadValues | Measure-Object -Average).Average, 2)
    Samples = $cpuData.Count
}

Write-Host "✅ CPU 分析完成" -ForegroundColor Green
Write-Host "  - 最大 CPU: $($cpuStats.MaxCPU)%" -ForegroundColor Gray
Write-Host "  - 平均 CPU: $($cpuStats.AvgCPU)%" -ForegroundColor Gray
Write-Host "  - 最大线程数: $($cpuStats.MaxThreads)" -ForegroundColor Gray
Write-Host ""

# 3. 性能评分
Write-Host "[3/5] 计算性能评分..." -ForegroundColor Yellow

$performanceScore = 100
$issues = @()

# 内存评分 (目标: < 300 MB)
if ($memoryStats.MaxWorkingSet -gt 300) {
    $performanceScore -= 20
    $issues += "❌ 内存占用过高 (>300 MB)"
} elseif ($memoryStats.MaxWorkingSet -gt 250) {
    $performanceScore -= 10
    $issues += "⚠️ 内存占用偏高 (>250 MB)"
}

# 内存增长评分 (目标: < 5%)
if ([math]::Abs($memoryStats.GrowthRate) -gt 10) {
    $performanceScore -= 30
    $issues += "❌ 疑似内存泄漏 (增长率 >10%)"
} elseif ([math]::Abs($memoryStats.GrowthRate) -gt 5) {
    $performanceScore -= 15
    $issues += "⚠️ 内存增长偏高 (>5%)"
}

# CPU 评分 (目标: < 10%)
if ($cpuStats.AvgCPU -gt 15) {
    $performanceScore -= 20
    $issues += "❌ CPU 占用过高 (>15%)"
} elseif ($cpuStats.AvgCPU -gt 10) {
    $performanceScore -= 10
    $issues += "⚠️ CPU 占用偏高 (>10%)"
}

# 稳定性评分 (样本数)
$expectedSamples = 100  # 约 100 秒测试
if ($memoryStats.Samples -lt 50) {
    $performanceScore -= 20
    $issues += "❌ 测试时间过短 (<50秒)"
}

$scoreColor = if ($performanceScore -ge 80) { "Green" } elseif ($performanceScore -ge 60) { "Yellow" } else { "Red" }
$scoreEmoji = if ($performanceScore -ge 80) { "🎉" } elseif ($performanceScore -ge 60) { "⚠️" } else { "❌" }

Write-Host "✅ 性能评分: $scoreEmoji $performanceScore/100" -ForegroundColor $scoreColor
if ($issues.Count -gt 0) {
    Write-Host ""
    Write-Host "发现问题:" -ForegroundColor Yellow
    foreach ($issue in $issues) {
        Write-Host "  $issue" -ForegroundColor $(if ($issue.StartsWith("❌")) { "Red" } else { "Yellow" })
    }
} else {
    Write-Host "  所有指标正常！" -ForegroundColor Green
}
Write-Host ""

# 4. 日志完整性检查
Write-Host "[4/5] 检查日志完整性..." -ForegroundColor Yellow

$logIssues = @()

# 检查是否有应用日志
$appLogs = Get-ChildItem "$LogDir\app_output_*.log" | Sort-Object LastWriteTime -Descending
if ($appLogs.Count -gt 0) {
    $latestAppLog = $appLogs[0].FullName
    $appLogContent = Get-Content $latestAppLog -Raw
    
    # 检查关键日志
    $hasSDKLoaded = $appLogContent -match "SDK loaded from"
    $hasSDKInjected = $appLogContent -match "SDK executed successfully"
    $hasTestStart = $appLogContent -match "开始自动测试|测试.*开始"
    $hasTestComplete = $appLogContent -match "测试完成|所有测试完成"
    
    if (-not $hasSDKLoaded) { $logIssues += "⚠️ 未找到 SDK 加载日志" }
    if (-not $hasSDKInjected) { $logIssues += "⚠️ 未找到 SDK 注入成功日志" }
    if (-not $hasTestStart) { $logIssues += "⚠️ 未找到测试开始日志" }
    if (-not $hasTestComplete) { $logIssues += "⚠️ 未找到测试完成日志" }
    
    # 检查错误日志
    $errorCount = ([regex]::Matches($appLogContent, "ERROR|Failed|失败|错误")).Count
    if ($errorCount -gt 0) {
        $logIssues += "❌ 发现 $errorCount 个错误日志"
    }
}

if ($logIssues.Count -eq 0) {
    Write-Host "✅ 日志完整性检查通过" -ForegroundColor Green
} else {
    Write-Host "⚠️ 日志完整性检查发现问题:" -ForegroundColor Yellow
    foreach ($issue in $logIssues) {
        Write-Host "  $issue" -ForegroundColor $(if ($issue.StartsWith("❌")) { "Red" } else { "Yellow" })
    }
}
Write-Host ""

# 5. 生成 HTML 报告 (可选)
if ($GenerateHTML) {
    Write-Host "[5/5] 生成 HTML 报告..." -ForegroundColor Yellow
    
    $htmlReport = "$LogDir\performance_report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"
    
    $html = @"
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>AnyWP Engine 性能测试报告</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
        h2 { color: #34495e; margin-top: 30px; }
        .score { font-size: 48px; font-weight: bold; text-align: center; margin: 30px 0; color: $(if ($performanceScore -ge 80) { '#27ae60' } elseif ($performanceScore -ge 60) { '#f39c12' } else { '#e74c3c' }); }
        .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin: 20px 0; }
        .stat-card { background: #ecf0f1; padding: 20px; border-radius: 8px; border-left: 4px solid #3498db; }
        .stat-label { font-size: 14px; color: #7f8c8d; margin-bottom: 5px; }
        .stat-value { font-size: 24px; font-weight: bold; color: #2c3e50; }
        .issue { background: #fee; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #e74c3c; }
        .success { background: #efe; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #27ae60; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background: #3498db; color: white; }
        tr:hover { background: #f5f5f5; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 AnyWP Engine 性能测试报告</h1>
        <p><strong>测试时间:</strong> $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</p>
        <p><strong>测试版本:</strong> 1.3.2-dev</p>
        
        <div class="score">$scoreEmoji $performanceScore/100</div>
        
        <h2>📊 内存性能</h2>
        <div class="stats">
            <div class="stat-card">
                <div class="stat-label">最大工作集</div>
                <div class="stat-value">$($memoryStats.MaxWorkingSet) MB</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">平均工作集</div>
                <div class="stat-value">$($memoryStats.AvgWorkingSet) MB</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">内存增长率</div>
                <div class="stat-value">$($memoryStats.GrowthRate)%</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">采样次数</div>
                <div class="stat-value">$($memoryStats.Samples)</div>
            </div>
        </div>
        
        <h2>⚡ CPU 性能</h2>
        <div class="stats">
            <div class="stat-card">
                <div class="stat-label">最大 CPU</div>
                <div class="stat-value">$($cpuStats.MaxCPU)%</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">平均 CPU</div>
                <div class="stat-value">$($cpuStats.AvgCPU)%</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">最大线程数</div>
                <div class="stat-value">$($cpuStats.MaxThreads)</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">平均线程数</div>
                <div class="stat-value">$($cpuStats.AvgThreads)</div>
            </div>
        </div>
        
        <h2>🔍 问题诊断</h2>
        $(if ($issues.Count -eq 0) {
            '<div class="success">✅ 所有性能指标正常！</div>'
        } else {
            $issues | ForEach-Object { "<div class='issue'>$_</div>" }
        })
        
        <h2>📋 日志完整性</h2>
        $(if ($logIssues.Count -eq 0) {
            '<div class="success">✅ 日志完整性检查通过</div>'
        } else {
            $logIssues | ForEach-Object { "<div class='issue'>$_</div>" }
        })
        
        <h2>📁 测试文件</h2>
        <ul>
            <li>内存日志: <code>$($memoryLogs[0].Name)</code></li>
            <li>CPU日志: <code>$($cpuLogs[0].Name)</code></li>
            $(if ($latestReport) { "<li>测试报告: <code>$($testReports[0].Name)</code></li>" })
        </ul>
        
        <p style="margin-top: 40px; text-align: center; color: #7f8c8d; font-size: 12px;">
            由 AnyWP Engine 测试分析工具自动生成 | $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
        </p>
    </div>
</body>
</html>
"@
    
    $html | Out-File -FilePath $htmlReport -Encoding UTF8
    Write-Host "✅ HTML 报告已生成: $htmlReport" -ForegroundColor Green
    Write-Host ""
    
    # 打开报告
    Start-Process $htmlReport
} else {
    Write-Host "[5/5] 跳过 HTML 报告生成 (使用 -GenerateHTML 参数启用)" -ForegroundColor Gray
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "分析完成！" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "💡 提示: 使用以下命令生成 HTML 报告:" -ForegroundColor Yellow
Write-Host "  .\analyze_test_results.ps1 -GenerateHTML" -ForegroundColor Gray
Write-Host ""

