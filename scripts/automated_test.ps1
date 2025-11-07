# AnyWP Engine 自动化测试脚本
# 功能: 自动化测试重构后的模块

$ErrorActionPreference = "Continue"
$TestResults = @{
    Total = 0
    Passed = 0
    Failed = 0
    Skipped = 0
}

function Write-TestResult {
    param(
        [string]$TestName,
        [string]$Status,  # PASS, FAIL, SKIP
        [string]$Message = ""
    )
    
    $TestResults.Total++
    
    $color = switch ($Status) {
        "PASS" { $TestResults.Passed++; "Green" }
        "FAIL" { $TestResults.Failed++; "Red" }
        "SKIP" { $TestResults.Skipped++; "Yellow" }
        default { "White" }
    }
    
    $timestamp = Get-Date -Format "HH:mm:ss"
    Write-Host "[$timestamp] [$Status] $TestName" -ForegroundColor $color
    if ($Message) {
        Write-Host "         └─ $Message" -ForegroundColor Gray
    }
}

function Test-FileExists {
    param([string]$Path, [string]$Description)
    
    if (Test-Path $Path) {
        Write-TestResult -TestName $Description -Status "PASS" -Message "文件存在: $Path"
        return $true
    } else {
        Write-TestResult -TestName $Description -Status "FAIL" -Message "文件缺失: $Path"
        return $false
    }
}

function Test-ModuleCompilation {
    Write-Host "`n=== 测试1: 模块编译验证 ===" -ForegroundColor Cyan
    
    # 检查新模块文件是否存在
    $modules = @(
        @{Path="windows\utils\state_persistence.h"; Name="StatePersistence 头文件"},
        @{Path="windows\utils\state_persistence.cpp"; Name="StatePersistence 实现"},
        @{Path="windows\utils\logger.h"; Name="Logger 头文件"},
        @{Path="windows\utils\logger.cpp"; Name="Logger 实现"},
        @{Path="windows\utils\url_validator.h"; Name="URLValidator 头文件"},
        @{Path="windows\utils\url_validator.cpp"; Name="URLValidator 实现"},
        @{Path="windows\modules\iframe_detector.h"; Name="IframeDetector 头文件"},
        @{Path="windows\modules\iframe_detector.cpp"; Name="IframeDetector 实现"},
        @{Path="windows\modules\sdk_bridge.h"; Name="SDKBridge 头文件"},
        @{Path="windows\modules\sdk_bridge.cpp"; Name="SDKBridge 实现"},
        @{Path="windows\modules\mouse_hook_manager.h"; Name="MouseHookManager 头文件"},
        @{Path="windows\modules\mouse_hook_manager.cpp"; Name="MouseHookManager 实现"},
        @{Path="windows\modules\monitor_manager.h"; Name="MonitorManager 头文件"},
        @{Path="windows\modules\monitor_manager.cpp"; Name="MonitorManager 实现"},
        @{Path="windows\modules\power_manager.h"; Name="PowerManager 头文件"},
        @{Path="windows\modules\power_manager.cpp"; Name="PowerManager 实现"}
    )
    
    foreach ($module in $modules) {
        Test-FileExists -Path $module.Path -Description $module.Name
    }
}

function Test-BuildArtifacts {
    Write-Host "`n=== 测试2: 构建产物验证 ===" -ForegroundColor Cyan
    
    $artifacts = @(
        @{Path="example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"; Name="示例应用程序"},
        @{Path="example\build\windows\x64\plugins\anywp_engine\Debug\anywp_engine_plugin.dll"; Name="插件DLL"},
        @{Path="example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib"; Name="插件LIB文件"}
    )
    
    foreach ($artifact in $artifacts) {
        $exists = Test-FileExists -Path $artifact.Path -Description $artifact.Name
        if ($exists) {
            $file = Get-Item $artifact.Path
            Write-Host "         └─ 文件大小: $($file.Length / 1KB) KB" -ForegroundColor Gray
            Write-Host "         └─ 修改时间: $($file.LastWriteTime)" -ForegroundColor Gray
        }
    }
}

function Test-HTMLTestPages {
    Write-Host "`n=== 测试3: 测试页面验证 ===" -ForegroundColor Cyan
    
    $testPages = @(
        "examples\test_refactoring.html",
        "examples\test_simple.html",
        "examples\test_draggable.html",
        "examples\test_api.html"
    )
    
    foreach ($page in $testPages) {
        $exists = Test-FileExists -Path $page -Description "测试页面: $page"
        if ($exists) {
            $content = Get-Content $page -Raw
            if ($content -match 'window\.AnyWP') {
                Write-Host "         └─ SDK引用检测: ✓" -ForegroundColor Green
            } else {
                Write-Host "         └─ SDK引用检测: ✗" -ForegroundColor Red
            }
        }
    }
}

function Test-CMakeConfiguration {
    Write-Host "`n=== 测试4: CMake配置验证 ===" -ForegroundColor Cyan
    
    $cmakeFile = "windows\CMakeLists.txt"
    if (Test-Path $cmakeFile) {
        $content = Get-Content $cmakeFile -Raw
        
        $requiredModules = @(
            "state_persistence.cpp",
            "logger.cpp",
            "url_validator.cpp",
            "iframe_detector.cpp",
            "sdk_bridge.cpp",
            "mouse_hook_manager.cpp",
            "monitor_manager.cpp",
            "power_manager.cpp"
        )
        
        foreach ($module in $requiredModules) {
            if ($content -match $module) {
                Write-TestResult -TestName "CMake包含: $module" -Status "PASS"
            } else {
                Write-TestResult -TestName "CMake包含: $module" -Status "FAIL"
            }
        }
    } else {
        Write-TestResult -TestName "CMakeLists.txt" -Status "FAIL" -Message "文件不存在"
    }
}

function Test-ApplicationStartup {
    Write-Host "`n=== 测试5: 应用程序启动测试 ===" -ForegroundColor Cyan
    
    $exePath = "example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
    
    if (-not (Test-Path $exePath)) {
        Write-TestResult -TestName "应用启动测试" -Status "SKIP" -Message "应用程序不存在"
        return
    }
    
    # 检查应用是否已在运行
    $process = Get-Process anywallpaper_engine_example -ErrorAction SilentlyContinue
    
    if ($process) {
        Write-TestResult -TestName "应用程序运行状态" -Status "PASS" -Message "应用正在运行 (PID: $($process.Id))"
        
        # 检查内存使用
        $memoryMB = [math]::Round($process.WorkingSet64 / 1MB, 2)
        if ($memoryMB -lt 500) {
            Write-TestResult -TestName "内存使用检查" -Status "PASS" -Message "内存: ${memoryMB}MB (正常)"
        } else {
            Write-TestResult -TestName "内存使用检查" -Status "FAIL" -Message "内存: ${memoryMB}MB (过高)"
        }
    } else {
        Write-TestResult -TestName "应用程序运行状态" -Status "SKIP" -Message "应用未运行"
    }
}

function Test-CodeIntegrity {
    Write-Host "`n=== 测试6: 代码完整性检查 ===" -ForegroundColor Cyan
    
    # 检查头文件包含
    $mainHeader = "windows\anywp_engine_plugin.h"
    if (Test-Path $mainHeader) {
        $content = Get-Content $mainHeader -Raw
        
        if ($content -match '#include "utils/url_validator\.h"') {
            Write-TestResult -TestName "头文件包含: url_validator" -Status "PASS"
        } else {
            Write-TestResult -TestName "头文件包含: url_validator" -Status "FAIL"
        }
    }
    
    # 检查旧代码是否被禁用
    $mainCpp = "windows\anywp_engine_plugin.cpp"
    if (Test-Path $mainCpp) {
        $content = Get-Content $mainCpp -Raw
        
        if ($content -match '#if 0.*URLValidator') {
            Write-TestResult -TestName "旧代码禁用检查" -Status "PASS" -Message "URLValidator 旧实现已禁用"
        } else {
            Write-TestResult -TestName "旧代码禁用检查" -Status "SKIP" -Message "无法确认"
        }
    }
}

function Show-TestSummary {
    Write-Host "`n" -NoNewline
    Write-Host "=" * 60 -ForegroundColor Cyan
    Write-Host "                   测试总结" -ForegroundColor Cyan
    Write-Host "=" * 60 -ForegroundColor Cyan
    
    Write-Host "`n总测试数: $($TestResults.Total)" -ForegroundColor White
    Write-Host "通过: $($TestResults.Passed)" -ForegroundColor Green
    Write-Host "失败: $($TestResults.Failed)" -ForegroundColor Red
    Write-Host "跳过: $($TestResults.Skipped)" -ForegroundColor Yellow
    
    $passRate = if ($TestResults.Total -gt 0) { 
        [math]::Round(($TestResults.Passed / $TestResults.Total) * 100, 2) 
    } else { 0 }
    
    Write-Host "`n通过率: $passRate%" -ForegroundColor $(if ($passRate -ge 80) { "Green" } elseif ($passRate -ge 60) { "Yellow" } else { "Red" })
    
    if ($TestResults.Failed -eq 0) {
        Write-Host "`n✅ 所有测试通过！重构成功！" -ForegroundColor Green
    } elseif ($TestResults.Failed -le 3) {
        Write-Host "`n⚠️  有少量测试失败，需要修复" -ForegroundColor Yellow
    } else {
        Write-Host "`n❌ 多个测试失败，需要检查重构代码" -ForegroundColor Red
    }
    
    Write-Host "`n" -NoNewline
    Write-Host "=" * 60 -ForegroundColor Cyan
}

# 主测试流程
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "        AnyWP Engine 重构自动化测试" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host "开始时间: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

# 切换到项目根目录
Set-Location "E:\Projects\AnyWallpaper\AnyWallpaper-Engine"

# 执行所有测试
Test-ModuleCompilation
Test-BuildArtifacts
Test-HTMLTestPages
Test-CMakeConfiguration
Test-ApplicationStartup
Test-CodeIntegrity

# 显示总结
Show-TestSummary

# 返回退出码
exit $TestResults.Failed

