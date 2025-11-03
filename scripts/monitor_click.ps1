# Auto Monitor Click Events
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 自动监听点击事件" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 清理旧日志
$logPath = "..\click_test.log"
if (Test-Path $logPath) { Remove-Item $logPath -Force }

# 启动应用
Write-Host "[1/3] 启动应用..." -ForegroundColor Yellow
cd ..\example
$process = Start-Process -FilePath "build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" `
    -RedirectStandardOutput "..\click_test.log" `
    -RedirectStandardError "..\click_test_error.log" `
    -PassThru

Write-Host "[2/3] 等待 12 秒启动..." -ForegroundColor Yellow
Start-Sleep -Seconds 12

cd ..

Write-Host "[3/3] 监听中..." -ForegroundColor Yellow
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host " 请点击壁纸上的红色边框区域" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "监听 20 秒，等待您的点击..." -ForegroundColor Cyan

# 持续监听 20 秒
$startTime = Get-Date
$foundClick = $false
$foundHandler = $false
$foundHit = $false

while (((Get-Date) - $startTime).TotalSeconds -lt 20) {
    Start-Sleep -Milliseconds 500
    
    if (Test-Path $logPath) {
        $content = Get-Content $logPath -ErrorAction SilentlyContinue
        
        # 检查点击事件
        $clickEvents = $content | Select-String "AnyWP:click event received"
        if ($clickEvents -and -not $foundClick) {
            $foundClick = $true
            Write-Host "✅ 检测到点击事件！" -ForegroundColor Green
            $clickEvents | Select-Object -Last 1 | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
        }
        
        # 检查处理器
        $handlers = $content | Select-String "Registered handlers:"
        if ($handlers -and -not $foundHandler) {
            $foundHandler = $true
            Write-Host "✅ 处理器信息：" -ForegroundColor Green
            $handlers | Select-Object -Last 1 | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
        }
        
        # 检查命中
        $hits = $content | Select-String "HIT! Calling callback"
        if ($hits -and -not $foundHit) {
            $foundHit = $true
            Write-Host "✅ 命中目标！回调已触发" -ForegroundColor Green
        }
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 诊断结果" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if (-not $foundClick) {
    Write-Host "❌ 未检测到点击事件" -ForegroundColor Red
    Write-Host "   问题：鼠标钩子未触发" -ForegroundColor Yellow
    Write-Host "   原因：C++ 鼠标钩子代码可能有问题" -ForegroundColor Yellow
} elseif (-not $foundHit) {
    Write-Host "⚠️  检测到点击但未命中" -ForegroundColor Yellow
    Write-Host "   问题：坐标不匹配" -ForegroundColor Yellow
    Write-Host "   原因：DPI 缩放或边界计算错误" -ForegroundColor Yellow
    
    # 显示详细信息
    if (Test-Path $logPath) {
        Write-Host ""
        Write-Host "详细日志：" -ForegroundColor Cyan
        Get-Content $logPath | Select-String "handleClick|Checking handler|Is in bounds" | Select-Object -Last 10 | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
    }
} else {
    Write-Host "✅ 点击事件完整流程正常" -ForegroundColor Green
    Write-Host "   如果页面没反应，问题在 React 回调函数" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "应用仍在运行，按任意键查看完整日志..." -ForegroundColor Cyan

