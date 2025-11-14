function Get-PubspecVersion {
    param(
        [Parameter(Mandatory = $true)]
        [string]$PubspecPath
    )

    if (-not (Test-Path -Path $PubspecPath)) {
        throw "pubspec.yaml not found: $PubspecPath"
    }

    $lines = Get-Content -Path $PubspecPath -Encoding UTF8
    foreach ($line in $lines) {
        if ($line -match '^\s*version:\s*(?<ver>\S+)') {
            return $Matches['ver']
        }
    }

    throw "Version field not found in pubspec.yaml: $PubspecPath"
}

function Get-ChangelogSection {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ChangelogPath,
        [Parameter(Mandatory = $true)]
        [string]$Version
    )

    if (-not (Test-Path -Path $ChangelogPath)) {
        throw "CHANGELOG file not found: $ChangelogPath"
    }

    $lines = Get-Content -Path $ChangelogPath -Encoding UTF8
    $escapedVersion = [Regex]::Escape($Version)

    $startIndex = -1
    for ($index = 0; $index -lt $lines.Count; $index++) {
        if ($lines[$index] -match "^## \[\s*$escapedVersion\s*\]") {
            $startIndex = $index
            break
        }
    }

    if ($startIndex -lt 0) {
        throw "Version [$Version] section not found in CHANGELOG: $ChangelogPath"
    }

    $endIndex = $lines.Count
    for ($index = $startIndex + 1; $index -lt $lines.Count; $index++) {
        if ($lines[$index] -match '^## \[') {
            $endIndex = $index
            break
        }
    }

    $sectionLines = @()
    for ($index = $startIndex; $index -lt $endIndex; $index++) {
        $sectionLines += $lines[$index]
    }

    $headerLine = $sectionLines[0]
    $bodyLines = @()
    if ($sectionLines.Count -gt 1) {
        $bodyLines = $sectionLines | Select-Object -Skip 1
    }

    $releaseDate = $null
    $title = $null
    $headerMatch = [Regex]::Match($headerLine, '^## \[(?<ver>[^\]]+)\]\s*-\s*(?<date>\d{4}-\d{2}-\d{2})(\s*-\s*(?<title>.+))?')
    if ($headerMatch.Success) {
        $releaseDate = $headerMatch.Groups['date'].Value
        if ($headerMatch.Groups['title'].Success) {
            $title = $headerMatch.Groups['title'].Value.Trim()
        }
    }

    return [PSCustomObject]@{
        HeaderLine  = $headerLine
        BodyLines   = $bodyLines
        ReleaseDate = $releaseDate
        Title       = $title
        StartIndex  = $startIndex
    }
}

Export-ModuleMember -Function Get-PubspecVersion, Get-ChangelogSection

