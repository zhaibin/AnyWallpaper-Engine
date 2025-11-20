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

# Get Web SDK version
$sdkPackagePath = Join-Path $ProjectRoot 'windows\sdk\package.json'
$sdkVersion = $Version  # Default to engine version
if (Test-Path -Path $sdkPackagePath) {
    try {
        $sdkPkg = Get-Content -Path $sdkPackagePath -Raw | ConvertFrom-Json
        $sdkVersion = $sdkPkg.version
    } catch {
        Write-Warning "Failed to parse SDK version, using engine version"
    }
}

$changelogPath = Join-Path $ProjectRoot 'CHANGELOG_CN.md'
$section = Get-ChangelogSection -ChangelogPath $changelogPath -Version $Version

$releaseDir = Join-Path $ProjectRoot 'release'
if (-not (Test-Path -Path $releaseDir)) {
    New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
}

$outputPath = Join-Path $releaseDir ("GITHUB_RELEASE_NOTES_v{0}.md" -f $Version)

$lines = @()
$lines += "# AnyWP Engine v$Version - Release Notes"
$lines += ""
if ($section.ReleaseDate) {
    $lines += "**发布日期**: $($section.ReleaseDate)"
}
$lines += "**Flutter Plugin Version**: $Version"
$lines += "**Web SDK Version**: $sdkVersion"
$lines += ""
$lines += "> **Note**: Flutter Plugin and Web SDK have independent version numbers."
$lines += ""
$lines += "---"
$lines += ""
if ($section.Title) {
    $lines += "## $($section.Title)"
    $lines += ""
}

foreach ($line in $section.BodyLines) {
    if ($line -match '^### ') {
        $lines += ($line -replace '^###', '##')
    } elseif ($line -match '^#### ') {
        $lines += ($line -replace '^####', '###')
    } else {
        $lines += $line
    }
}

Set-Content -Path $outputPath -Value $lines -Encoding UTF8
Write-Host "Generated release notes: $outputPath"

