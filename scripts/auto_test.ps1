# AnyWP Engine - Auto Test Script
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " AnyWP Engine - 自动测试脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 停止旧进程
Write-Host "[1/5] 停止旧进程..." -ForegroundColor Yellow
Get-Process -Name "anywallpaper_engine_example" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 2

# 清理旧日志
$logPath = "..\test_auto.log"
if (Test-Path $logPath) {
    Remove-Item $logPath -Force
}

# 启动应用
Write-Host "[2/5] 启动应用并记录日志..." -ForegroundColor Yellow
cd ..\example
$process = Start-Process -FilePath "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" `
    -RedirectStandardOutput "..\test_auto.log" `
    -RedirectStandardError "..\test_auto_error.log" `
    -PassThru `
    -WindowStyle Normal

# 等待启动
Write-Host "[3/5] 等待应用启动 (10秒)..." -ForegroundColor Yellow
Start-Sleep -Seconds 10

# 分析日志
cd ..
Write-Host "[4/5] 分析日志..." -ForegroundColor Yellow
Write-Host ""

$logContent = Get-Content $logPath -ErrorAction SilentlyContinue

# 检查 SDK 注入
$sdkInjectLines = $logContent | Select-String -Pattern "Inject|SDK.*loaded|SDK.*injected" -SimpleMatch:$false
if ($sdkInjectLines) {
    Write-Host "✅ SDK 注入日志:" -ForegroundColor Green
    $sdkInjectLines | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
} else {
    Write-Host "❌ 未找到 SDK 注入日志!" -ForegroundColor Red
}

Write-Host ""

# 检查导航
$navLines = $logContent | Select-String -Pattern "Navigating to.*test_react"
if ($navLines) {
    Write-Host "✅ 正在加载 React 测试页面:" -ForegroundColor Green
    $navLines | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
} else {
    Write-Host "⚠️  未检测到 React 页面加载" -ForegroundColor Yellow
    $anyNav = $logContent | Select-String -Pattern "Navigating to" | Select-Object -First 1
    if ($anyNav) {
        Write-Host "   当前加载: $anyNav" -ForegroundColor Gray
    }
}

Write-Host ""
Write-Host "[5/5] 测试完成" -ForegroundColor Yellow
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 总结" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$summary = @()

# SDK 版本检查
if ($sdkInjectLines -match "v4\.0") {
    $summary += "✅ SDK v4.0.0 已注入"
} elseif ($sdkInjectLines) {
    $summary += "⚠️  SDK 已注入但版本可能不正确"
} else {
    $summary += "❌ SDK 未注入"
}

# 页面检查
if ($navLines) {
    $summary += "✅ React 测试页面已加载"
} else {
    $summary += "❌ 未加载 React 页面"
}

$summary | ForEach-Object { Write-Host $_ }

Write-Host ""
Write-Host "应用正在运行，请检查壁纸是否显示正确。" -ForegroundColor Cyan
Write-Host "期望看到:" -ForegroundColor Cyan
Write-Host "  - AnyWP SDK v4.0.0" -ForegroundColor White
Write-Host "  - SPA Mode: Enabled" -ForegroundColor White
Write-Host "  - 可点击的按钮和卡片" -ForegroundColor White
Write-Host ""

