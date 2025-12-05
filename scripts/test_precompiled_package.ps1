# ========================================
# AnyWP Engine - Precompiled Package Testing
# ========================================
# 自动化测试预编译包的完整性和功能
# 用法: .\scripts\test_precompiled_package.ps1 -Version "2.4.1" [-TestLevel Full|Quick|Basic]
# ========================================

param(
    [Parameter(Mandatory=$true)]
    [string]$Version,
    
    [Parameter(Mandatory=$false)]
    [ValidateSet("Basic", "Quick", "Full")]
    [string]$TestLevel = "Quick",
    
    [Parameter(Mandatory=$false)]
    [string]$PackagePath = ""
)

$ErrorActionPreference = "Continue"
$TestsPassed = 0
$TestsFailed = 0
$TestsSkipped = 0

# 颜色输出函数
function Write-Success { param($msg) Write-Host "  [✓] $msg" -ForegroundColor Green }
function Write-Failure { param($msg) Write-Host "  [✗] $msg" -ForegroundColor Red; $script:TestsFailed++ }
function Write-Info { param($msg) Write-Host "  [i] $msg" -ForegroundColor Cyan }
function Write-Skip { param($msg) Write-Host "  [~] $msg" -ForegroundColor Yellow; $script:TestsSkipped++ }

# 确定包路径
if ($PackagePath -eq "") {
    $PackagePath = Join-Path $PSScriptRoot "..\release\anywp_engine_v${Version}_precompiled"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " AnyWP Engine Package Test Suite" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Version    : $Version"
Write-Host "Test Level : $TestLevel"
Write-Host "Package    : $PackagePath"
Write-Host ""

# ========================================
# 第1层: 基础完整性测试
# ========================================
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "第1层: 基础完整性测试" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

# 测试1.1: 包目录存在
Write-Host "[Test 1.1] 检查包目录..."
if (Test-Path $PackagePath) {
    Write-Success "包目录存在: $PackagePath"
    $TestsPassed++
} else {
    Write-Failure "包目录不存在: $PackagePath"
    exit 1
}

# 测试1.2: 关键文件存在性
Write-Host "`n[Test 1.2] 检查关键文件..."
$RequiredFiles = @(
    "bin\anywp_engine_plugin.dll",
    "bin\WebView2Loader.dll",
    "lib\anywp_engine_plugin.lib",
    "lib\anywp_engine.dart",
    "lib\dart\anywp_engine.dart",
    "include\anywp_engine\anywp_engine_plugin_c_api.h",
    "windows\CMakeLists.txt",
    "README.md",
    "CHANGELOG_CN.md",
    "LICENSE",
    "pubspec.yaml"
)

foreach ($file in $RequiredFiles) {
    $fullPath = Join-Path $PackagePath $file
    if (Test-Path $fullPath) {
        Write-Success "$file"
        $TestsPassed++
    } else {
        Write-Failure "$file 缺失"
    }
}

# 测试1.3: DLL文件大小合理性
Write-Host "`n[Test 1.3] 检查DLL文件大小..."
$dllPath = Join-Path $PackagePath "bin\anywp_engine_plugin.dll"
$dllInfo = Get-Item $dllPath -ErrorAction SilentlyContinue
if ($dllInfo) {
    $sizeKB = [math]::Round($dllInfo.Length / 1KB, 2)
    if ($sizeKB -gt 500 -and $sizeKB -lt 5000) {
        Write-Success "DLL大小正常: $sizeKB KB"
        $TestsPassed++
    } elseif ($sizeKB -lt 500) {
        Write-Failure "DLL文件过小 ($sizeKB KB)，可能不完整"
    } else {
        Write-Failure "DLL文件过大 ($sizeKB KB)，可能包含调试信息"
    }
} else {
    Write-Failure "无法读取DLL文件信息"
}

# 测试1.4: 版本一致性检查
Write-Host "`n[Test 1.4] 检查版本一致性..."
$pubspecPath = Join-Path $PackagePath "pubspec.yaml"
if (Test-Path $pubspecPath) {
    $content = Get-Content $pubspecPath -Raw
    if ($content -match "version:\s*$Version") {
        Write-Success "pubspec.yaml 版本号正确: $Version"
        $TestsPassed++
    } else {
        Write-Failure "pubspec.yaml 版本号不匹配"
    }
} else {
    Write-Failure "pubspec.yaml 不存在"
}

# 测试1.5: DLL编译时间检查
Write-Host "`n[Test 1.5] 检查DLL编译时间..."
if ($dllInfo) {
    $compileTime = $dllInfo.LastWriteTime
    $timeDiff = (Get-Date) - $compileTime
    Write-Info "DLL编译时间: $($compileTime.ToString('yyyy-MM-dd HH:mm:ss'))"
    
    if ($timeDiff.TotalHours -lt 24) {
        Write-Success "DLL是最近24小时内编译的"
        $TestsPassed++
    } elseif ($timeDiff.TotalDays -lt 7) {
        Write-Info "DLL编译于 $([math]::Round($timeDiff.TotalDays, 1)) 天前"
        $TestsPassed++
    } else {
        Write-Failure "DLL编译时间过旧 ($([math]::Round($timeDiff.TotalDays, 0)) 天前)，可能是旧版本"
    }
}

if ($TestLevel -eq "Basic") {
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
    Write-Host "基础测试完成" -ForegroundColor Green
    Write-Host "通过: $TestsPassed | 失败: $TestsFailed | 跳过: $TestsSkipped" -ForegroundColor Green
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
    exit $(if ($TestsFailed -eq 0) { 0 } else { 1 })
}

# ========================================
# 第2层: DLL功能验证
# ========================================
Write-Host ""
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "第2层: DLL功能验证" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

# 测试2.1: 检查DLL导出符号（需要dumpbin工具）
Write-Host "[Test 2.1] 检查DLL导出符号..."

# 查找dumpbin（Visual Studio工具）
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$dumpbinPath = $null

if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -property installationPath 2>$null
    if ($vsPath) {
        $possiblePaths = @(
            "$vsPath\VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe",
            "$vsPath\VC\Tools\MSVC\*\bin\Hostx86\x64\dumpbin.exe"
        )
        foreach ($pattern in $possiblePaths) {
            $found = Get-Item $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($found) {
                $dumpbinPath = $found.FullName
                break
            }
        }
    }
}

if ($dumpbinPath -and (Test-Path $dumpbinPath)) {
    $exports = & $dumpbinPath /EXPORTS $dllPath 2>&1 | Out-String
    
    # 检查关键导出函数（Flutter插件标准接口）
    if ($exports -match "FlutterDesktopPluginRegister") {
        Write-Success "DLL包含Flutter插件注册函数"
        $TestsPassed++
    } else {
        Write-Failure "DLL缺少Flutter插件注册函数"
    }
    
    # 统计导出函数数量
    $exportCount = ($exports -split "`n" | Where-Object { $_ -match '^\s+\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+\w+' }).Count
    if ($exportCount -gt 0) {
        Write-Info "找到 $exportCount 个导出函数"
        $TestsPassed++
    }
} else {
    Write-Skip "dumpbin工具未找到，跳过导出符号检查"
    Write-Info "（需要安装 Visual Studio 或 Build Tools）"
}

# 测试2.2: 检查CHANGELOG是否包含当前版本的更新说明
Write-Host "`n[Test 2.2] 检查CHANGELOG更新说明..."
$changelogPath = Join-Path $PackagePath "CHANGELOG_CN.md"
if (Test-Path $changelogPath) {
    $changelog = Get-Content $changelogPath -Raw
    if ($changelog -match "\[?$Version\]?" -or $changelog -match "## $Version") {
        Write-Success "CHANGELOG包含 v$Version 的更新说明"
        $TestsPassed++
    } else {
        Write-Failure "CHANGELOG未找到 v$Version 的更新说明"
    }
} else {
    Write-Failure "CHANGELOG_CN.md 不存在"
}

# 测试2.3: 检查Dart API完整性
Write-Host "`n[Test 2.3] 检查Dart API完整性..."
$dartApiPath = Join-Path $PackagePath "lib\anywp_engine.dart"
if (Test-Path $dartApiPath) {
    $dartContent = Get-Content $dartApiPath -Raw
    
    # v2.4.1 应该包含的关键API（核心功能）
    $requiredAPIs = @(
        "initializeWallpaperOnMonitor",
        "enableAutoRecovery",
        "isAutoRecoveryEnabled",
        "saveCurrentWallpaperConfiguration",
        "sendMessage"
    )
    
    foreach ($api in $requiredAPIs) {
        if ($dartContent -match "Future.*$api") {
            Write-Success "API存在: $api"
            $TestsPassed++
        } else {
            Write-Failure "API缺失: $api"
        }
    }
} else {
    Write-Failure "Dart API文件不存在"
}

if ($TestLevel -eq "Quick") {
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
    Write-Host "快速测试完成" -ForegroundColor Green
    Write-Host "通过: $TestsPassed | 失败: $TestsFailed | 跳过: $TestsSkipped" -ForegroundColor Green
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Green
    exit $(if ($TestsFailed -eq 0) { 0 } else { 1 })
}

# ========================================
# 第3层: 完整功能测试
# ========================================
Write-Host ""
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "第3层: 完整功能测试" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

Write-Host "[Test 3.1] 准备集成测试环境..."
$testProjectPath = Join-Path $PSScriptRoot "..\test_integration"

if (-not (Test-Path $testProjectPath)) {
    Write-Info "创建集成测试项目..."
    New-Item -ItemType Directory -Path $testProjectPath -Force | Out-Null
}

# 创建测试Flutter项目配置（如果不存在）
$testPubspecPath = Join-Path $testProjectPath "pubspec.yaml"
if (-not (Test-Path $testPubspecPath)) {
    Write-Info "创建测试项目配置..."
    # 这里需要创建一个完整的测试项目
    Write-Skip "集成测试项目需要手动创建"
    Write-Info "请参考: scripts\create_integration_test.ps1"
} else {
    Write-Host "`n[Test 3.2] 运行集成测试..."
    Write-Info "启动测试应用..."
    
    # TODO: 实现完整的集成测试
    Write-Skip "完整集成测试需要运行Flutter应用（实现中）"
}

# ========================================
# 测试总结
# ========================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 测试总结" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "测试级别: $TestLevel"
Write-Host "通过: $TestsPassed" -ForegroundColor Green
Write-Host "失败: $TestsFailed" -ForegroundColor $(if ($TestsFailed -eq 0) { "Green" } else { "Red" })
Write-Host "跳过: $TestsSkipped" -ForegroundColor Yellow
Write-Host ""

if ($TestsFailed -eq 0) {
    Write-Host "✓ 所有测试通过！" -ForegroundColor Green
    Write-Host "预编译包可以安全使用" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ 发现 $TestsFailed 个问题" -ForegroundColor Red
    Write-Host "请修复问题后重新生成预编译包" -ForegroundColor Red
    exit 1
}

