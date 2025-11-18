# PowerShell Script: Migrate std::cout to Logger (Intelligent Version)
# v2.3.2 - AnyWP Engine
# 智能版：自动类型推断，正确处理变量

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

# 从文件中提取变量类型映射
function Build-TypeMap {
    param([string]$Content)
    
    $typeMap = @{}
    
    # Pattern 1: std::string variable
    $stringVars = [regex]::Matches($Content, 'std::string\s+(\w+)')
    foreach ($match in $stringVars) {
        $typeMap[$match.Groups[1].Value] = 'string'
    }
    
    # Pattern 2: const char* / wchar_t*
    $charPtrVars = [regex]::Matches($Content, '(?:const\s+)?(?:w)?char\s*\*\s*(\w+)')
    foreach ($match in $charPtrVars) {
        $typeMap[$match.Groups[1].Value] = 'string'
    }
    
    # Pattern 3: int, size_t, DWORD, etc.
    $intVars = [regex]::Matches($Content, '(?:int|size_t|DWORD|unsigned|long|short|int32_t|int64_t|uint32_t|uint64_t)\s+(\w+)')
    foreach ($match in $intVars) {
        $typeMap[$match.Groups[1].Value] = 'number'
    }
    
    # Pattern 4: bool
    $boolVars = [regex]::Matches($Content, 'bool\s+(\w+)')
    foreach ($match in $boolVars) {
        $typeMap[$match.Groups[1].Value] = 'bool'
    }
    
    # Pattern 5: HWND, HANDLE (pointers)
    $ptrVars = [regex]::Matches($Content, '(?:HWND|HANDLE|HINSTANCE|HMODULE)\s+(\w+)')
    foreach ($match in $ptrVars) {
        $typeMap[$match.Groups[1].Value] = 'pointer'
    }
    
    # Pattern 6: auto variables (try to infer from assignment)
    $autoVars = [regex]::Matches($Content, 'auto\s+(\w+)\s*=\s*"')
    foreach ($match in $autoVars) {
        $typeMap[$match.Groups[1].Value] = 'string'
    }
    
    return $typeMap
}

# 提取变量名和推断类型
function Get-VariableType {
    param([string]$VarName, [hashtable]$TypeMap, [string]$Context)
    
    # Check type map first
    if ($TypeMap.ContainsKey($VarName)) {
        return $TypeMap[$VarName]
    }
    
    # Heuristic: if it ends with _name, _url, _path, _message, _str -> string
    if ($VarName -match '_(name|url|path|message|str|text|id)$') {
        return 'string'
    }
    
    # Heuristic: if it ends with _count, _index, _size, _width, _height -> number
    if ($VarName -match '_(count|index|size|width|height|length|num)$') {
        return 'number'
    }
    
    # Heuristic: if it ends with _window, _handle, _hwnd -> pointer
    if ($VarName -match '_(window|handle|hwnd|ptr)$') {
        return 'pointer'
    }
    
    # Heuristic: check if it's a member access with known suffix
    if ($VarName -match '->(\w+)$') {
        $memberName = $Matches[1]
        if ($memberName -match '(name|url|path|message)') {
            return 'string'
        }
        if ($memberName -match '(width|height|size|count)') {
            return 'number'
        }
    }
    
    # Default: unknown (need manual handling)
    return 'unknown'
}

# 转换变量表达式为 Logger 格式
function Convert-Variable {
    param([string]$Variable, [string]$Type)
    
    $Variable = $Variable.Trim()
    
    switch ($Type) {
        'string' {
            # Already a string, no conversion needed
            return $Variable
        }
        'number' {
            # Need std::to_string()
            return "std::to_string($Variable)"
        }
        'pointer' {
            # Cast to uintptr_t then to_string
            return "std::to_string(reinterpret_cast<uintptr_t>($Variable))"
        }
        'bool' {
            # Convert bool to string
            return "($Variable ? `"true`" : `"false`")"
        }
        default {
            # Unknown type, return null to skip
            return $null
        }
    }
}

function Analyze-StdCout {
    param([string]$File)
    
    $content = Get-Content $File -Raw
    $pattern = 'std::cout\s*<<[^;]+<<\s*std::endl\s*;'
    $matches = [regex]::Matches($content, $pattern)
    
    return @{
        File = $File
        Content = $content
        Count = $matches.Count
        Matches = $matches
        TypeMap = (Build-TypeMap -Content $content)
    }
}

function Convert-StdCoutToLogger {
    param([string]$Line, [hashtable]$TypeMap, [string]$Context)
    
    $Line = $Line.Trim()
    
    # Pattern 1: Banner
    if ($Line -match 'std::cout\s*<<\s*"\[AnyWP\](\s*\[([^\]]+)\])?\s*=+\s*([^=]+?)\s*=+"\s*<<\s*std::endl') {
        $component = if ($matches[2]) { $matches[2] } else { "Plugin" }
        $title = $matches[3].Trim()
        return "Logger::Instance().Banner(`"$component`", `"$title`");"
    }
    
    # Pattern 2: Pure string message (no variables)
    if ($Line -match '^std::cout\s*<<\s*"\[([^\]]+)\](\s*\[([^\]]+)\])?\s*([^"]+)"\s*<<\s*std::endl\s*;$') {
        $prefix = $matches[1]
        $component = if ($matches[3]) { $matches[3] } else { $prefix }
        $message = $matches[4]
        
        if ($prefix -eq "AnyWP") {
            return "Logger::Instance().Info(`"$component`", `"$message`");"
        } else {
            return "Logger::Instance().Info(`"$prefix`", `"$message`");"
        }
    }
    
    # Pattern 3: Message with single variable
    # Format: std::cout << "[Component] Prefix " << variable << std::endl;
    if ($Line -match 'std::cout\s*<<\s*"\[([^\]]+)\](\s*\[([^\]]+)\])?\s*([^"]+)"\s*<<\s*([^<]+?)\s*<<\s*std::endl') {
        $prefix = $matches[1]
        $component = if ($matches[3]) { $matches[3] } else { $prefix }
        $messagePrefix = $matches[4]
        $variable = $matches[5].Trim()
        
        # Determine variable type
        $varType = Get-VariableType -VarName $variable -TypeMap $TypeMap -Context $Context
        $convertedVar = Convert-Variable -Variable $variable -Type $varType
        
        if (-not $convertedVar) {
            return $null  # Skip if we can't determine type
        }
        
        if ($prefix -eq "AnyWP") {
            return "Logger::Instance().Info(`"$component`", `"$messagePrefix`" + $convertedVar);"
        } else {
            return "Logger::Instance().Info(`"$prefix`", `"$messagePrefix`" + $convertedVar);"
        }
    }
    
    # Pattern 4: Message with multiple parts
    # Format: std::cout << "[Component] Part1 " << var1 << " Part2 " << var2 << std::endl;
    if ($Line -match 'std::cout\s*<<\s*"\[([^\]]+)\](\s*\[([^\]]+)\])?\s*([^"]+)"') {
        $prefix = $matches[1]
        $component = if ($matches[3]) { $matches[3] } else { $prefix }
        $firstPart = $matches[4]
        
        # Extract all parts after the first string
        $remaining = $Line.Substring($matches[0].Length)
        
        # Try to extract variables and strings
        $parts = @("`"$firstPart`"")
        $varMatches = [regex]::Matches($remaining, '<<\s*([^<"]+?)(?:\s*<<|;)')
        $stringMatches = [regex]::Matches($remaining, '<<\s*"([^"]+)"')
        
        # Simple case: alternate between variables and strings
        $hasUnknown = $false
        for ($i = 0; $i -lt $varMatches.Count - 1; $i++) {
            $var = $varMatches[$i].Groups[1].Value.Trim()
            if ($var -ne "std::endl") {
                $varType = Get-VariableType -VarName $var -TypeMap $TypeMap -Context $Context
                $convertedVar = Convert-Variable -Variable $var -Type $varType
                
                if (-not $convertedVar) {
                    $hasUnknown = $true
                    break
                }
                
                $parts += $convertedVar
                
                # Add next string if exists
                if ($i -lt $stringMatches.Count) {
                    $parts += "`"$($stringMatches[$i].Groups[1].Value)`""
                }
            }
        }
        
        if ($hasUnknown) {
            return $null  # Skip if any variable type is unknown
        }
        
        if ($parts.Count -gt 1) {
            $combined = $parts -join " + "
            if ($prefix -eq "AnyWP") {
                return "Logger::Instance().Info(`"$component`", $combined);"
            } else {
                return "Logger::Instance().Info(`"$prefix`", $combined);"
            }
        }
    }
    
    return $null
}

function Test-Conversion {
    param([string]$Original, [string]$Converted)
    
    if (-not $Converted) { return $false }
    
    if ($Converted -notmatch 'Logger::Instance\(\)\.(Info|Debug|Warning|Error|Banner|Section)') {
        return $false
    }
    
    $openParen = ($Converted.ToCharArray() | Where-Object { $_ -eq '(' }).Count
    $closeParen = ($Converted.ToCharArray() | Where-Object { $_ -eq ')' }).Count
    if ($openParen -ne $closeParen) {
        return $false
    }
    
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

Write-ColorOutput "`n========== Logger Migration Tool v3 (Intelligent) ==========`n" "Cyan"
Write-ColorOutput "File: $FilePath" "Yellow"
Write-ColorOutput "Mode: $(if ($DryRun) { 'DRY RUN (no changes)' } else { 'LIVE (will modify file)' })`n" "Yellow"

$analysis = Analyze-StdCout -File $FilePath
Write-ColorOutput "Found $($analysis.Count) std::cout statements" "Green"
Write-ColorOutput "Built type map with $($analysis.TypeMap.Count) variables`n" "Green"

if ($analysis.Count -eq 0) {
    Write-ColorOutput "Nothing to migrate.`n" "Green"
    exit 0
}

$conversions = @()
$skipped = @()

foreach ($match in $analysis.Matches) {
    $original = $match.Value
    $converted = Convert-StdCoutToLogger -Line $original -TypeMap $analysis.TypeMap -Context $analysis.Content
    
    if ($converted -and (Test-Conversion -Original $original -Converted $converted)) {
        $conversions += @{
            Original = $original
            Converted = $converted
            Index = $match.Index
        }
        
        if ($ShowDetails) {
            Write-ColorOutput "✓ CONVERTED:" "Green"
            Write-ColorOutput "  [-] $($original.Substring(0, [Math]::Min(80, $original.Length)))" "DarkGray"
            Write-ColorOutput "  [+] $($converted.Substring(0, [Math]::Min(80, $converted.Length)))`n" "Green"
        }
    } else {
        $skipped += $original
        if ($ShowDetails) {
            Write-ColorOutput "⚠ SKIPPED (manual review needed):" "Yellow"
            Write-ColorOutput "  $($original.Substring(0, [Math]::Min(80, $original.Length)))`n" "DarkGray"
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
    $content = $analysis.Content
    
    # Sort by index descending to avoid offset issues
    $conversions = $conversions | Sort-Object -Property Index -Descending
    
    foreach ($conversion in $conversions) {
        $content = $content.Replace($conversion.Original, $conversion.Converted)
    }
    
    # Backup
    $backupPath = "$FilePath.bak2"
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

if ($DryRun) {
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "This was a DRY RUN. No files were modified." "Yellow"
    Write-ColorOutput "Run without -DryRun to apply changes.`n" "Yellow"
}

# Exit code
if ($conversions.Count -eq $analysis.Count) {
    exit 0
} else {
    exit 1
}


