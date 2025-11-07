# Automated Release Script for AnyWP Engine
# 自动化发版脚本 - 执行完整的发版流程

param(
    [Parameter(Mandatory=$true)]
    [string]$Version,
    
    [Parameter(Mandatory=$true)]
    [string]$ReleaseTitle,
    
    [switch]$SkipBuild,
    [switch]$SkipGitPush,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

Write-Host "=========================================="
Write-Host "AnyWP Engine - Automated Release v$Version"
Write-Host "=========================================="
Write-Host ""
Write-Host "Release Title: $ReleaseTitle"
if ($DryRun) {
    Write-Host "[DRY RUN MODE] - No actual changes will be made"
}
Write-Host ""

# ============================================================
# Step 1: Pre-flight checks
# ============================================================
Write-Host "[1/9] Pre-flight checks..."

# Check if we're in the project root
if (!(Test-Path "scripts\auto_release.ps1")) {
    Write-Host "[ERROR] Please run this script from project root directory"
    exit 1
}

# Check if version files exist
$versionFiles = @(
    "pubspec.yaml",
    "scripts\build_release_v2.bat",
    ".cursorrules"
)

foreach ($file in $versionFiles) {
    if (!(Test-Path $file)) {
        Write-Host "[ERROR] Required file not found: $file"
        exit 1
    }
}

# Check if git working directory is clean
$gitStatus = git status --porcelain
if ($gitStatus -and !$DryRun) {
    Write-Host "[WARNING] Git working directory is not clean:"
    Write-Host $gitStatus
    Write-Host ""
    $continue = Read-Host "Continue anyway? (y/N)"
    if ($continue -ne "y") {
        Write-Host "Aborted by user"
        exit 1
    }
}

Write-Host "✓ Pre-flight checks passed"
Write-Host ""

# ============================================================
# Step 2: Update version numbers
# ============================================================
Write-Host "[2/9] Updating version numbers..."

if (!$DryRun) {
    # Update pubspec.yaml
    $pubspec = Get-Content "pubspec.yaml" -Raw
    $pubspec = $pubspec -replace 'version:\s*\d+\.\d+\.\d+', "version: $Version"
    Set-Content "pubspec.yaml" $pubspec -Encoding UTF8 -NoNewline
    
    # Update build_release_v2.bat
    $buildScript = Get-Content "scripts\build_release_v2.bat" -Raw
    $buildScript = $buildScript -replace 'set "VERSION=\d+\.\d+\.\d+"', "set `"VERSION=$Version`""
    Set-Content "scripts\build_release_v2.bat" $buildScript -Encoding UTF8 -NoNewline
    
    # Update .cursorrules (only the version line at the bottom)
    $cursorrules = Get-Content ".cursorrules" -Raw
    $cursorrules = $cursorrules -replace '\*\*版本\*\*:\s*\d+\.\d+\.\d+', "**版本**: $Version"
    Set-Content ".cursorrules" $cursorrules -Encoding UTF8 -NoNewline
    
    Write-Host "✓ Version numbers updated to $Version"
} else {
    Write-Host "[DRY RUN] Would update version numbers to $Version"
}
Write-Host ""

# ============================================================
# Step 3: Build release packages
# ============================================================
if (!$SkipBuild) {
    Write-Host "[3/9] Building release packages..."
    Write-Host "This will take 3-5 minutes..."
    Write-Host ""
    
    if (!$DryRun) {
        # Run build script
        $buildResult = cmd /c "set NO_PAUSE=1 && scripts\build_release_v2.bat"
        
        # Check if build succeeded
        if (!(Test-Path "release\anywp_engine_v$Version.zip")) {
            Write-Host "[ERROR] Flutter Plugin package not found!"
            exit 1
        }
        
        if (!(Test-Path "release\anywp_web_sdk_v$Version.zip")) {
            Write-Host "[ERROR] Web SDK package not found!"
            exit 1
        }
        
        $pluginSize = [math]::Round((Get-Item "release\anywp_engine_v$Version.zip").Length / 1MB, 2)
        $webSdkSize = [math]::Round((Get-Item "release\anywp_web_sdk_v$Version.zip").Length / 1KB, 2)
        
        Write-Host "✓ Flutter Plugin: $pluginSize MB"
        Write-Host "✓ Web SDK: $webSdkSize KB"
    } else {
        Write-Host "[DRY RUN] Would build release packages"
    }
} else {
    Write-Host "[3/9] Skipping build (--SkipBuild)"
}
Write-Host ""

# ============================================================
# Step 4: Verify critical files
# ============================================================
Write-Host "[4/9] Verifying critical files..."

$criticalFiles = @{
    "release\anywp_engine_v$Version\bin\anywp_engine_plugin.dll" = "Plugin DLL"
    "release\anywp_engine_v$Version\lib\anywp_engine_plugin.lib" = "Link Library"
    "release\anywp_engine_v$Version\lib\anywp_engine.dart" = "Dart Source"
    "release\anywp_engine_v$Version\windows\src\anywp_engine_plugin.cpp" = "C++ Source"
    "release\anywp_web_sdk_v$Version.zip" = "Web SDK Package"
}

if (!$SkipBuild -and !$DryRun) {
    $allOk = $true
    foreach ($file in $criticalFiles.Keys) {
        if (Test-Path $file) {
            Write-Host "  ✓ $($criticalFiles[$file])"
        } else {
            Write-Host "  ✗ $($criticalFiles[$file]) - MISSING!" -ForegroundColor Red
            $allOk = $false
        }
    }
    
    if (!$allOk) {
        Write-Host ""
        Write-Host "[ERROR] Some critical files are missing!"
        exit 1
    }
} else {
    Write-Host "[DRY RUN] Would verify critical files"
}
Write-Host ""

# ============================================================
# Step 5: Create Release Notes
# ============================================================
Write-Host "[5/9] Creating Release Notes..."

$releaseNotesPath = "release\GITHUB_RELEASE_NOTES_v$Version.md"
if (!(Test-Path $releaseNotesPath) -and !$DryRun) {
    Write-Host "[WARNING] Release notes not found: $releaseNotesPath"
    Write-Host "Please create it manually from docs/RELEASE_TEMPLATE.md"
    Write-Host ""
    $continue = Read-Host "Continue without Release Notes? (y/N)"
    if ($continue -ne "y") {
        Write-Host "Aborted by user"
        exit 1
    }
} else {
    Write-Host "✓ Release notes found: $releaseNotesPath"
}
Write-Host ""

# ============================================================
# Step 6: Git commit
# ============================================================
Write-Host "[6/9] Committing changes..."

if (!$DryRun) {
    # Add files
    git add pubspec.yaml scripts/build_release_v2.bat .cursorrules CHANGELOG_CN.md README.md release/
    
    # Create commit message
    $commitMsg = @"
release: 发布 v$Version - $ReleaseTitle

发布包：
- Flutter Plugin: anywp_engine_v$Version.zip
- Web SDK: anywp_web_sdk_v$Version.zip

自动生成于: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
"@
    
    Set-Content "commit_msg.txt" $commitMsg -Encoding UTF8
    
    # Commit
    git commit -F commit_msg.txt
    Remove-Item "commit_msg.txt"
    
    Write-Host "✓ Changes committed"
} else {
    Write-Host "[DRY RUN] Would commit changes"
}
Write-Host ""

# ============================================================
# Step 7: Git push
# ============================================================
if (!$SkipGitPush) {
    Write-Host "[7/9] Pushing to GitHub..."
    
    if (!$DryRun) {
        git push origin main
        Write-Host "✓ Pushed to main branch"
    } else {
        Write-Host "[DRY RUN] Would push to GitHub"
    }
} else {
    Write-Host "[7/9] Skipping Git push (--SkipGitPush)"
}
Write-Host ""

# ============================================================
# Step 8: Create and push Git tag
# ============================================================
Write-Host "[8/9] Creating and pushing Git tag..."

if (!$DryRun) {
    # Create tag
    git tag -a "v$Version" -m "AnyWP Engine v$Version - $ReleaseTitle"
    
    # Push tag
    if (!$SkipGitPush) {
        git push origin "v$Version"
        Write-Host "✓ Tag v$Version created and pushed"
    } else {
        Write-Host "✓ Tag v$Version created (not pushed due to --SkipGitPush)"
    }
} else {
    Write-Host "[DRY RUN] Would create and push tag v$Version"
}
Write-Host ""

# ============================================================
# Step 9: Summary and next steps
# ============================================================
Write-Host "[9/9] Release preparation complete!"
Write-Host ""
Write-Host "=========================================="
Write-Host "✅ Release v$Version Ready"
Write-Host "=========================================="
Write-Host ""

if (!$DryRun) {
    Write-Host "📦 Release Packages:"
    Write-Host "  1. release\anywp_engine_v$Version.zip"
    if (Test-Path "release\anywp_engine_v$Version.zip") {
        $size = [math]::Round((Get-Item "release\anywp_engine_v$Version.zip").Length / 1MB, 2)
        Write-Host "     Size: $size MB"
    }
    Write-Host "  2. release\anywp_web_sdk_v$Version.zip"
    if (Test-Path "release\anywp_web_sdk_v$Version.zip") {
        $size = [math]::Round((Get-Item "release\anywp_web_sdk_v$Version.zip").Length / 1KB, 2)
        Write-Host "     Size: $size KB"
    }
    Write-Host ""
    
    Write-Host "📝 Release Notes:"
    Write-Host "  release\GITHUB_RELEASE_NOTES_v$Version.md"
    Write-Host ""
    
    Write-Host "🔖 Git Tag:"
    Write-Host "  v$Version"
    if (!$SkipGitPush) {
        Write-Host "  Status: Pushed to GitHub ✓"
    } else {
        Write-Host "  Status: Created locally (not pushed)"
    }
    Write-Host ""
}

Write-Host "=========================================="
Write-Host "Next Steps:"
Write-Host "=========================================="
Write-Host ""
Write-Host "1. Visit: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new"
Write-Host "2. Select Tag: v$Version"
Write-Host "3. Title: AnyWP Engine v$Version - $ReleaseTitle"
Write-Host "4. Description: Copy from release\GITHUB_RELEASE_NOTES_v$Version.md"
Write-Host "5. Upload BOTH files:"
Write-Host "   - anywp_engine_v$Version.zip"
Write-Host "   - anywp_web_sdk_v$Version.zip"
Write-Host "6. Check: Set as the latest release"
Write-Host "7. Click: Publish release"
Write-Host ""

if ($DryRun) {
    Write-Host "[DRY RUN] No actual changes were made"
    Write-Host "Run without --DryRun to execute the release"
}

