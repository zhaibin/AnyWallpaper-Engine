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
            $errors += "CHANGELOG_CN.md top version is [$($match.Groups['ver'].Value)], need update to [$Version]"
        }
    } else {
        $errors += "CHANGELOG_CN.md does not contain any version section"
    }
} catch {
    $errors += $_.Exception.Message
}

# Check .cursorrules
$cursorRulesPath = Join-Path $ProjectRoot '.cursorrules'
if (Test-Path -Path $cursorRulesPath) {
    $cursorMatch = Select-String -Path $cursorRulesPath -Pattern '\*\*Version\*\*:\s*(?<ver>\d+\.\d+\.\d+)' -AllMatches | Select-Object -First 1
    if ($cursorMatch) {
        $cursorVersion = $cursorMatch.Matches[0].Groups['ver'].Value
        if ($cursorVersion -ne $Version) {
            $errors += ".cursorrules version is [$cursorVersion], need update to [$Version]"
        }
    } else {
        $errors += ".cursorrules missing version marker (**Version**: x.x.x)"
    }
} else {
    $errors += ".cursorrules file is missing"
}

# Check PRECOMPILED integration guide
$integrationPath = Join-Path $ProjectRoot 'docs\PRECOMPILED_DLL_INTEGRATION.md'
if (Test-Path -Path $integrationPath) {
    $precompiledToken = "anywp_engine_v{0}_precompiled" -f $Version
    $escapedToken = [Regex]::Escape($precompiledToken)
    if (-not (Select-String -Path $integrationPath -Pattern $escapedToken -Quiet)) {
        $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md need update package name ($precompiledToken)"
    }

    $versionLinePattern = "\*\*Version\*\*:\s*$([Regex]::Escape($Version))"
    if (-not (Select-String -Path $integrationPath -Pattern $versionLinePattern -Quiet)) {
        $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md need update version label (**Version**: $Version)"
    }
} else {
    $errors += "docs/PRECOMPILED_DLL_INTEGRATION.md file is missing"
}

if ($errors.Count -eq 0) {
    Write-Host "Version consistency check passed for $Version"
    exit 0
}

Write-Host "Version consistency check failed:" -ForegroundColor Red
foreach ($err in $errors) {
    Write-Host " - $err"
}
exit 1
