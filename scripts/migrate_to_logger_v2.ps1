# PowerShell Script: Migrate std::cout to Logger (Fixed Version)
# v2.3.2 - AnyWP Engine
# 修复版：正确处理变量类型，避免不必要的 std::to_string()

param(
    [Parameter(Mandatory=$true)]
    [string]$FilePath,
    [switch]$DryRun = $false,
    [switch]$ShowDetails = $false
)

$ErrorActionPreference = "Stop"

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Analyze-StdCout {
    param([string]$File)
    
    $content = Get-Content $File -Raw
    # 匹配 std::cout << ... << std::endl;
    $pattern = 'std::cout\s*<<[^;]+<<\s*std::endl\s*;'
    $matches = [regex]::Matches($content, $pattern)
    
    return @{
        File = $File
        Count = $matches.Count
        Matches = $matches
    }
}

function Convert-StdCoutToLogger {
    param([string]$Line)
    
    $Line = $Line.Trim()
    
    # Pattern 1: Banner (========== Title ==========)
    if ($Line -match 'std::cout\s*<<\s*"\[AnyWP\](\s*\[([^\]]+)\])?\s*=+\s*([^=]+?)\s*=+"\s*<<\s*std::endl') {
        $component = if ($matches[2]) { $matches[2] } else { "Plugin" }
        $title = $matches[3].Trim()
        return "Logger::Instance().Banner(`"$component`", `"$title`");"
    }
    
    # Pattern 2: [AnyWP] [Component] Message (纯字符串，无变量)
    if ($Line -match '^std::cout\s*<<\s*"\[AnyWP\]\s*\[([^\]]+)\]\s*([^"]+)"\s*<<\s*std::endl\s*;$') {
        $component = $matches[1].Trim()
        $message = $matches[2]  # 保留原始空格
        return "Logger::Instance().Info(`"$component`", `"$message`");"
    }
    
    # Pattern 3: [AnyWP] Message (纯字符串，无变量)
    if ($Line -match '^std::cout\s*<<\s*"\[AnyWP\]\s*([^"]+)"\s*<<\s*std::endl\s*;$') {
        $message = $matches[1]  # 保留原始空格
        return "Logger::Instance().Info(`"Plugin`", `"$message`");"
    }
    
    # Pattern 4: [WindowManager] Message (其他组件格式)
    if ($Line -match '^std::cout\s*<<\s*"\[([^\]]+)\]\s*([^"]+)"\s*<<\s*std::endl\s*;$') {
        $component = $matches[1].Trim()
        $message = $matches[2]
        return "Logger::Instance().Info(`"$component`", `"$message`");"
    }
    
    # 对于包含变量的复杂模式，跳过自动转换
    # 这些需要手动处理，因为需要判断变量类型
    return $null
}

function Test-Conversion {
    param([string]$Original, [string]$Converted)
    
    if (-not $Converted) { return $false }
    
    # 验证基本格式
    if ($Converted -notmatch 'Logger::Instance\(\)\.(Info|Debug|Warning|Error|Banner|Section)') {
        return $false
    }
    
    # 检查括号匹配
    $openParen = ($Converted.ToCharArray() | Where-Object { $_ -eq '(' }).Count
    $closeParen = ($Converted.ToCharArray() | Where-Object { $_ -eq ')' }).Count
    if ($openParen -ne $closeParen) {
        return $false
    }
    
    # 检查引号匹配
    $unescapedQuotes = ([regex]::Matches($Converted, '(?<!\\)"')).Count
    if ($unescapedQuotes % 2 -ne 0) {
        return $false
    }
    
    # 检查是否以分号结尾
    if ($Converted -notmatch ';$') {
        return $false
    }
    
    return $true
}

# ========== Main ==========

if (-not (Test-Path $FilePath)) {
    Write-ColorOutput "Error: File not found: $FilePath" "Red"
    exit 1
}

Write-ColorOutput "`n========== Logger Migration Tool v2.3.2 (Fixed) ==========`n" "Cyan"
Write-ColorOutput "File: $FilePath" "Yellow"
Write-ColorOutput "Mode: $(if ($DryRun) { 'DRY RUN (no changes)' } else { 'LIVE (will modify file)' })`n" "Yellow"

$analysis = Analyze-StdCout -File $FilePath
Write-ColorOutput "Found $($analysis.Count) std::cout statements" "Green"

if ($analysis.Count -eq 0) {
    Write-ColorOutput "Nothing to migrate.`n" "Green"
    exit 0
}

Write-ColorOutput ""

$conversions = @()
$skipped = @()

foreach ($match in $analysis.Matches) {
    $original = $match.Value
    $converted = Convert-StdCoutToLogger -Line $original
    
    if ($converted -and (Test-Conversion -Original $original -Converted $converted)) {
        $conversions += @{
            Original = $original
            Converted = $converted
            Index = $match.Index
        }
        
        if ($ShowDetails) {
            Write-ColorOutput "✓ CONVERTED:" "Green"
            Write-ColorOutput "  [-] $original" "DarkGray"
            Write-ColorOutput "  [+] $converted`n" "Green"
        }
    } else {
        $skipped += $original
        if ($ShowDetails) {
            Write-ColorOutput "⚠ SKIPPED (manual review needed):" "Yellow"
            Write-ColorOutput "  $original`n" "DarkGray"
        }
    }
}

Write-ColorOutput "========================================" "Cyan"
Write-ColorOutput "Successfully converted: $($conversions.Count) / $($analysis.Count)" "Green"

if ($skipped.Count -gt 0) {
    Write-ColorOutput "Need manual review: $($skipped.Count)" "Yellow"
    Write-ColorOutput "========================================`n" "Cyan"
}

# Apply changes
if (-not $DryRun -and $conversions.Count -gt 0) {
    $content = Get-Content $FilePath -Raw
    
    # 从后往前替换
    $conversions = $conversions | Sort-Object -Property Index -Descending
    
    foreach ($conversion in $conversions) {
        $content = $content.Replace($conversion.Original, $conversion.Converted)
    }
    
    # Backup
    $backupPath = "$FilePath.bak"
    Copy-Item $FilePath $backupPath -Force
    Write-ColorOutput "✓ Backup created: $backupPath" "Yellow"
    
    # Write with UTF-8 (no BOM)
    $encoding = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($FilePath, $content, $encoding)
    Write-ColorOutput "✓ File updated: $FilePath`n" "Green"
}

# Final summary
Write-ColorOutput "========== Summary ==========`n" "Cyan"
Write-ColorOutput "  Total std::cout: $($analysis.Count)" "White"
Write-ColorOutput "  ✓ Converted: $($conversions.Count)" "Green"
Write-ColorOutput "  ⚠ Manual review: $($skipped.Count)" "Yellow"
Write-ColorOutput "  Success rate: $(if ($analysis.Count -gt 0) { [math]::Round($conversions.Count / $analysis.Count * 100, 1) } else { 0 })%`n" "Cyan"

if ($skipped.Count -gt 0 -and $ShowDetails) {
    Write-ColorOutput "Patterns needing manual review:" "Yellow"
    $skipped | Select-Object -First 5 | ForEach-Object {
        $truncated = if ($_.Length -gt 80) { $_.Substring(0, 80) + "..." } else { $_ }
        Write-ColorOutput "  • $truncated" "DarkGray"
    }
    if ($skipped.Count -gt 5) {
        Write-ColorOutput "  ... and $($skipped.Count - 5) more`n" "DarkGray"
    }
}

if ($DryRun) {
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "This was a DRY RUN. No files were modified." "Yellow"
    Write-ColorOutput "Run without -DryRun to apply changes.`n" "Yellow"
}

# Exit code: 0 = all converted, 1 = some need manual review
if ($conversions.Count -eq $analysis.Count) {
    exit 0
} else {
    exit 1
}

