param(
    [string]$Version,
    [string]$ProjectRoot,
    [int]$MaxItems = 6
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

$changelogPath = Join-Path $ProjectRoot 'CHANGELOG_CN.md'
$section = Get-ChangelogSection -ChangelogPath $changelogPath -Version $Version

$releaseDir = Join-Path $ProjectRoot 'release'
if (-not (Test-Path -Path $releaseDir)) {
    New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
}

$outputPath = Join-Path $releaseDir ("commit_msg_v{0}.txt" -f $Version)

$items = @()
foreach ($line in $section.BodyLines) {
    if ($items.Count -ge $MaxItems) {
        break
    }

    if ($line -match '^### ') {
        $title = $line.Substring(4).Trim()
        if ($title.Length -gt 0) {
            $items += "- $title"
        }
        continue
    }

    if ($line -match '^#### ') {
        $subtitle = $line.Substring(5).Trim()
        if ($subtitle.Length -gt 0) {
            $items += "- $subtitle"
        }
    }
}

if ($items.Count -eq 0) {
    $items += "- 详见 release/GITHUB_RELEASE_NOTES_v$Version.md"
}

$lines = @()
$lines += "release: 发布 v$Version"
$lines += ""
$lines += $items

Set-Content -Path $outputPath -Value $lines -Encoding UTF8
Write-Host "Generated commit template: $outputPath"

