param(
    [string]$Version,
    [string]$ProjectRoot
)

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ProjectRoot) {
    $ProjectRoot = Resolve-Path -Path (Join-Path $scriptRoot '..')
}

Import-Module (Join-Path $scriptRoot 'release_utils.psm1') -Force

$pubspecPath = Join-Path $ProjectRoot 'pubspec.yaml'
if (-not $Version -or $Version.Trim().Length -eq 0) {
    $Version = Get-PubspecVersion -PubspecPath $pubspecPath
}

$errors = @()

# Check CHANGELOG
$changelogPath = Join-Path $ProjectRoot 'CHANGELOG_CN.md'
try {
    $section = Get-ChangelogSection -ChangelogPath $changelogPath -Version $Version
    $allLines = Get-Content -Path $changelogPath -Encoding UTF8
    $firstHeading = $allLines | Where-Object { $_ -match '^## \[' } | Select-Object -First 1
    if ($firstHeading) {
        $match = [Regex]::Match($firstHeading, '^## \[(?<ver>[^\]]+)\]')
        if (-not $match.Success -or $match.Groups['ver'].Value -ne $Version) {
            $errors += "CHANGELOG_CN.md 顶部版本为 [$($match.Groups['ver'].Value)]，需更新为 [$Version]"
        }
    } else {
        $errors += "CHANGELOG_CN.md 中未找到任何版本段落"
    }
} catch {
    $errors += $_.Exception.Message
}

# Check .cursorrules
$cursorRulesPath = Join-Path $ProjectRoot '.cursorrules'
if (Test-Path -Path $cursorRulesPath) {
    $cursorMatch = Select-String -Path $cursorRulesPath -Pattern '\*\*版本\*\*:\s*(?<ver>\d+\.\d+\.\d+)' -AllMatches | Select-Object -First 1
    if ($cursorMatch) {
        $cursorVersion = $cursorMatch.Matches[0].Groups['ver'].Value
        if ($cursorVersion -ne $Version) {
            $errors += ".cursorrules 标记版本为 [$cursorVersion]，需更新为 [$Version]"
        }
    } else {
        $errors += ".cursorrules 中缺少版本标记（**版本**: x.x.x）"
    }
} else {
    $errors += ".cursorrules 文件缺失"
}

# Check PRECOMPILED integration guide
$integrationPath = Join-Path $ProjectRoot 'docs\PRECOMPILED_DLL_INTEGRATION.md'
if (Test-Path -Path $integrationPath) {
    $precompiledToken = ("anywp_engine_v{0}_precompiled" -f $Version)
    if (-not (Select-String -Path $integrationPath -Pattern [Regex]::Escape($precompiledToken) -Quiet)) {
        $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md 未更新包名标记 ($precompiledToken)"
    }

    $versionLinePattern = "\*\*版本\*\*:\s*$([Regex]::Escape($Version))"
    if (-not (Select-String -Path $integrationPath -Pattern $versionLinePattern -Quiet)) {
        $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md 未更新版本标签 (**版本**: $Version)"
    }
} else {
    $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md 文件缺失"
}

if ($errors.Count -eq 0) {
    Write-Host "Version consistency check passed for $Version"
    exit 0
}

Write-Host "Version consistency check failed:" -ForegroundColor Red
foreach ($error in $errors) {
    Write-Host " - $error"
}
exit 1

