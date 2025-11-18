# PowerShell Script: Migrate std::cout to Logger
# v2.3.2 - AnyWP Engine

param(
    [string]$FilePath,
    [switch]$DryRun = $false,
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"

function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Analyze-StdCout {
    param([string]$File)
    
    $content = Get-Content $File -Raw
    $matches = [regex]::Matches($content, 'std::cout\s*<<\s*"?\[AnyWP\]([^;]+);')
    
    return @{
        File = $File
        Count = $matches.Count
        Matches = $matches
    }
}

function Extract-Component {
    param([string]$Line)
    
    # Try to extract component from [AnyWP] [Component] pattern
    if ($Line -match '\[AnyWP\]\s*\[([^\]]+)\]') {
        return $matches[1]
    }
    
    # Fallback: guess from context
    $file = Split-Path $Line -Leaf
    $component = $file -replace '\.cpp$','' -replace '\.h$',''
    $component = $component -replace '_','

' -replace '^(.)',{ $_.Value.ToUpper() }
    return $component
}

function Convert-StdCoutToLogger {
    param([string]$Line)
    
    # Pattern 1: std::cout << "[AnyWP] [Module] Message" << std::endl;
    if ($Line -match 'std::cout\s*<<\s*"\[AnyWP\]\s*\[([^\]]+)\]\s*([^"]+)"\s*<<\s*std::endl') {
        $component = $matches[1]
        $message = $matches[2].Trim()
        return "Logger::Instance().Info(`"$component`", `"$message`");"
    }
    
    # Pattern 2: std::cout << "[AnyWP] Message" << std::endl;
    if ($Line -match 'std::cout\s*<<\s*"\[AnyWP\]\s*([^"]+)"\s*<<\s*std::endl') {
        $message = $matches[1].Trim()
        return "Logger::Instance().Info(`"Plugin`", `"$message`");"
    }
    
    # Pattern 3: std::cout << "[AnyWP] [Module] " << variable << std::endl;
    if ($Line -match 'std::cout\s*<<\s*"\[AnyWP\]\s*\[([^\]]+)\]\s*([^"]+)"\s*<<\s*([^<]+)\s*<<\s*std::endl') {
        $component = $matches[1]
        $prefix = $matches[2].Trim()
        $variable = $matches[3].Trim()
        return "Logger::Instance().Info(`"$component`", `"$prefix`" + $variable);"
    }
    
    return $null
}

# Main
if (-not (Test-Path $FilePath)) {
    Write-ColorOutput "Error: File not found: $FilePath" "Red"
    exit 1
}

Write-ColorOutput "`n========== Logger Migration Tool ==========`n" "Cyan"
Write-ColorOutput "File: $FilePath" "Yellow"
Write-ColorOutput "Mode: $(if ($DryRun) { 'DRY RUN (no changes)' } else { 'LIVE (will modify file)' })`n" "Yellow"

$analysis = Analyze-StdCout -File $FilePath
Write-ColorOutput "Found $($analysis.Count) std::cout statements`n" "Green"

if ($analysis.Count -eq 0) {
    Write-ColorOutput "No std::cout statements found. Nothing to migrate.`n" "Green"
    exit 0
}

$conversions = @()
foreach ($match in $analysis.Matches) {
    $original = $match.Value
    $converted = Convert-StdCoutToLogger -Line $original
    
    if ($converted) {
        $conversions += @{
            Original = $original
            Converted = $converted
            Line = $match.Index
        }
        
        if ($Verbose) {
            Write-ColorOutput "  [-] $original" "Red"
            Write-ColorOutput "  [+] $converted`n" "Green"
        }
    }
}

Write-ColorOutput "Successfully converted: $($conversions.Count) / $($analysis.Count)`n" "Cyan"

if (-not $DryRun -and $conversions.Count -gt 0) {
    $content = Get-Content $FilePath -Raw
    
    foreach ($conversion in $conversions) {
        $content = $content.Replace($conversion.Original, $conversion.Converted)
    }
    
    # Backup original
    $backupPath = "$FilePath.bak"
    Copy-Item $FilePath $backupPath -Force
    Write-ColorOutput "Backup created: $backupPath" "Yellow"
    
    # Write modified content
    Set-Content $FilePath $content -NoNewline
    Write-ColorOutput "File updated: $FilePath`n" "Green"
}

Write-ColorOutput "========== Migration Complete ==========`n" "Cyan"

# Summary
Write-ColorOutput "Summary:" "Cyan"
Write-ColorOutput "  Total std::cout found: $($analysis.Count)" "White"
Write-ColorOutput "  Successfully converted: $($conversions.Count)" "Green"
Write-ColorOutput "  Manual review needed: $($analysis.Count - $conversions.Count)" "Yellow"

if ($DryRun) {
    Write-ColorOutput "`nRun without -DryRun to apply changes." "Yellow"
}

