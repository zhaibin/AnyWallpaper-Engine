# Simple Performance Data Collection
param([int]$Samples = 10)

Write-Host "Starting application..." -ForegroundColor Yellow
cd E:\Projects\AnyWallpaper\AnyWallpaper-Engine
$proc = Start-Process -FilePath "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe" -WorkingDirectory $PWD -PassThru

Start-Sleep -Seconds 3
Write-Host "Collecting $Samples samples..." -ForegroundColor Yellow

$memData = @()
for ($i = 0; $i -lt $Samples; $i++) {
    $proc.Refresh()
    $memMB = $proc.WorkingSet64 / 1MB
    $memData += $memMB
    Write-Host "Sample $($i+1): $([math]::Round($memMB, 2)) MB"
    Start-Sleep -Seconds 1
}

$avg = [math]::Round(($memData | Measure-Object -Average).Average, 2)
$max = [math]::Round(($memData | Measure-Object -Maximum).Maximum, 2)

Write-Host ""
Write-Host "Average Memory: $avg MB" -ForegroundColor Green
Write-Host "Peak Memory: $max MB" -ForegroundColor Green

Stop-Process -Id $proc.Id -Force

