param(
    [string]$EngineVersion,
    [string]$SdkVersion,
    [string]$ProjectRoot
)

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) {
    $ProjectRoot = Resolve-Path -Path (Join-Path $scriptRoot '..')
}

Import-Module (Join-Path $scriptRoot 'release_utils.psm1') -Force

$pubspecPath = Join-Path $ProjectRoot 'pubspec.yaml'
if (-not $EngineVersion -or $EngineVersion.Trim().Length -eq 0) {
    $EngineVersion = Get-PubspecVersion -PubspecPath $pubspecPath
}

if (-not $SdkVersion -or $SdkVersion.Trim().Length -eq 0) {
    $sdkPackagePath = Join-Path $ProjectRoot 'windows\sdk\package.json'
    if (Test-Path -Path $sdkPackagePath) {
        $sdkPkg = Get-Content -Path $sdkPackagePath -Raw | ConvertFrom-Json
        $SdkVersion = $sdkPkg.version
    }
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Documentation Consistency Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Engine Version: $EngineVersion"
Write-Host "  SDK Version:    $SdkVersion"
Write-Host ""

$errors = @()
$warnings = @()

# 检查 Flutter 开发者文档
$flutterDevDocs = @(
    'docs\FOR_FLUTTER_DEVELOPERS.md',
    'docs\DEVELOPER_API_REFERENCE.md',
    'docs\PRECOMPILED_DLL_INTEGRATION.md'
)

Write-Host "Checking Flutter Developer Documentation..." -ForegroundColor Yellow
foreach ($doc in $flutterDevDocs) {
    $docPath = Join-Path $ProjectRoot $doc
    if (-not (Test-Path -Path $docPath)) {
        $errors += "Missing Flutter developer documentation: $doc"
        continue
    }
    
    $content = Get-Content -Path $docPath -Raw -Encoding UTF8
    
    # 检查是否提及了版本号（至少应该有一个版本引用）
    if ($content -notmatch '\d+\.\d+\.\d+') {
        $warnings += "$doc does not contain any version references"
    }
    
    Write-Host "  [OK] $doc exists" -ForegroundColor Green
}

# 检查 Web 开发者文档
$webDevDocs = @(
    'docs\WEB_DEVELOPER_GUIDE_CN.md',
    'docs\WEB_DEVELOPER_GUIDE.md',
    'docs\API_USAGE_EXAMPLES.md'
)

Write-Host ""
Write-Host "Checking Web Developer Documentation..." -ForegroundColor Yellow
foreach ($doc in $webDevDocs) {
    $docPath = Join-Path $ProjectRoot $doc
    if (-not (Test-Path -Path $docPath)) {
        $errors += "Missing Web developer documentation: $doc"
        continue
    }
    
    $content = Get-Content -Path $docPath -Raw -Encoding UTF8
    
    # 检查是否提及了 SDK
    if ($content -notmatch 'AnyWP|SDK|anywp_sdk') {
        $warnings += "$doc does not contain SDK references"
    }
    
    Write-Host "  [OK] $doc exists" -ForegroundColor Green
}

# 检查 API 文档的接口一致性
Write-Host ""
Write-Host "Checking API Documentation Consistency..." -ForegroundColor Yellow

$dartApiPath = Join-Path $ProjectRoot 'lib\anywp_engine.dart'
$apiRefPath = Join-Path $ProjectRoot 'docs\DEVELOPER_API_REFERENCE.md'

if ((Test-Path -Path $dartApiPath) -and (Test-Path -Path $apiRefPath)) {
    # 提取 Dart API 中的公共方法
    $dartContent = Get-Content -Path $dartApiPath -Raw -Encoding UTF8
    $dartMethods = [regex]::Matches($dartContent, 'Future<[^>]+>\s+(\w+)\s*\(') | ForEach-Object { $_.Groups[1].Value }
    
    $apiRefContent = Get-Content -Path $apiRefPath -Raw -Encoding UTF8
    
    $missingInDocs = @()
    foreach ($method in $dartMethods) {
        if ($apiRefContent -notmatch [regex]::Escape($method)) {
            $missingInDocs += $method
        }
    }
    
    if ($missingInDocs.Count -gt 0) {
        $warnings += "DEVELOPER_API_REFERENCE.md may be missing documentation for: $($missingInDocs -join ', ')"
    } else {
        Write-Host "  [OK] API documentation appears complete" -ForegroundColor Green
    }
}

# 检查 README.md
Write-Host ""
Write-Host "Checking README.md..." -ForegroundColor Yellow
$readmePath = Join-Path $ProjectRoot 'README.md'
if (Test-Path -Path $readmePath) {
    $readmeContent = Get-Content -Path $readmePath -Raw -Encoding UTF8
    
    # 检查是否包含主要章节
    $requiredSections = @('Features', 'Installation', 'Usage', 'Documentation', 'License')
    foreach ($section in $requiredSections) {
        if ($readmeContent -notmatch "##\s+$section") {
            $warnings += "README.md missing section: $section"
        }
    }
    
    Write-Host "  [OK] README.md exists and contains main sections" -ForegroundColor Green
} else {
    $errors += "README.md is missing"
}

# 检查文档更新提示
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Documentation Update Reminders" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Please verify the following documentation has been updated:" -ForegroundColor Yellow
Write-Host ""
Write-Host "  Flutter Developers:" -ForegroundColor Cyan
Write-Host "    - docs/FOR_FLUTTER_DEVELOPERS.md" -ForegroundColor White
Write-Host "    - docs/DEVELOPER_API_REFERENCE.md" -ForegroundColor White
Write-Host "    - docs/PRECOMPILED_DLL_INTEGRATION.md (version $EngineVersion)" -ForegroundColor White
Write-Host ""
Write-Host "  Web Developers:" -ForegroundColor Cyan
Write-Host "    - docs/WEB_DEVELOPER_GUIDE_CN.md (SDK version $SdkVersion)" -ForegroundColor White
Write-Host "    - docs/WEB_DEVELOPER_GUIDE.md (SDK version $SdkVersion)" -ForegroundColor White
Write-Host "    - docs/API_USAGE_EXAMPLES.md" -ForegroundColor White
Write-Host ""
Write-Host "  Integration Guides:" -ForegroundColor Cyan
Write-Host "    - README.md (Features and Usage sections)" -ForegroundColor White
Write-Host "    - CHANGELOG_CN.md (version $EngineVersion entry)" -ForegroundColor White
Write-Host ""

# 汇总结果
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Check Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Errors:   $($errors.Count)" -ForegroundColor $(if ($errors.Count -eq 0) { 'Green' } else { 'Red' })
Write-Host "  Warnings: $($warnings.Count)" -ForegroundColor $(if ($warnings.Count -eq 0) { 'Green' } else { 'Yellow' })
Write-Host ""

if ($errors.Count -gt 0) {
    Write-Host "ERRORS:" -ForegroundColor Red
    foreach ($err in $errors) {
        Write-Host "  - $err" -ForegroundColor Red
    }
    Write-Host ""
}

if ($warnings.Count -gt 0) {
    Write-Host "WARNINGS:" -ForegroundColor Yellow
    foreach ($warn in $warnings) {
        Write-Host "  - $warn" -ForegroundColor Yellow
    }
    Write-Host ""
}

if ($errors.Count -eq 0) {
    Write-Host "[SUCCESS] Documentation consistency check passed!" -ForegroundColor Green
    Write-Host ""
    Write-Host "NOTE: This script only checks for file existence and basic structure." -ForegroundColor Yellow
    Write-Host "Please manually verify that documentation content is up-to-date." -ForegroundColor Yellow
    exit 0
} else {
    Write-Host "[FAILED] Documentation consistency check failed!" -ForegroundColor Red
    exit 1
}

