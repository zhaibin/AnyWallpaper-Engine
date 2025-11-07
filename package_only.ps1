# Package only script (assumes Release is already built)
$VERSION = "1.3.1"
$RELEASE_NAME = "anywp_engine_v$VERSION"
$RELEASE_PATH = "release\$RELEASE_NAME"

Write-Host "=========================================="
Write-Host "Packaging Release v$VERSION"
Write-Host "=========================================="
Write-Host ""

# Verify build exists
if (!(Test-Path "example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll")) {
    Write-Host "[ERROR] DLL not found! Please build first."
    exit 1
}

# Step 1: Clean old release
Write-Host "[1/11] Cleaning old release..."
if (Test-Path "release\$RELEASE_NAME") {
    Remove-Item "release\$RELEASE_NAME" -Recurse -Force
}
if (Test-Path "release\$RELEASE_NAME.zip") {
    Remove-Item "release\$RELEASE_NAME.zip" -Force
}
Write-Host "✓ Cleaned"

# Step 2: Create directory structure
Write-Host "[2/11] Creating directory structure..."
New-Item -ItemType Directory -Path "$RELEASE_PATH\bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\lib" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\lib\dart" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\include\anywp_engine" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\sdk" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\windows\src" -Force | Out-Null
New-Item -ItemType Directory -Path "$RELEASE_PATH\windows\packages" -Force | Out-Null
Write-Host "✓ Directories created"

# Step 3: Copy DLL and LIB files
Write-Host "[3/11] Copying DLL and LIB files..."
Copy-Item "example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.dll" "$RELEASE_PATH\bin\" -Force
Copy-Item "example\build\windows\x64\plugins\anywp_engine\Release\anywp_engine_plugin.lib" "$RELEASE_PATH\lib\" -Force
Copy-Item "windows\packages\Microsoft.Web.WebView2.1.0.2651.64\build\native\x64\WebView2Loader.dll" "$RELEASE_PATH\bin\" -Force
Write-Host "✓ DLL and LIB copied"

# Step 4: Copy header files
Write-Host "[4/11] Copying header files..."
Copy-Item "windows\include\anywp_engine\any_w_p_engine_plugin.h" "$RELEASE_PATH\include\anywp_engine\" -Force
Write-Host "✓ Headers copied"

# Step 5: Copy source files
Write-Host "[5/11] Copying source files..."
Copy-Item "windows\anywp_engine_plugin.cpp" "$RELEASE_PATH\windows\src\" -Force
Copy-Item "windows\anywp_engine_plugin.h" "$RELEASE_PATH\windows\src\" -Force
Write-Host "✓ Source files copied"

# Step 6: Copy CMakeLists and build files
Write-Host "[6/11] Copying CMakeLists and packages..."
Copy-Item "windows\CMakeLists.txt" "$RELEASE_PATH\windows\" -Force
Copy-Item "windows\packages.config" "$RELEASE_PATH\windows\" -Force
Copy-Item "windows\packages\*" "$RELEASE_PATH\windows\packages\" -Recurse -Force
Write-Host "✓ CMake files copied"

# Step 7: Copy SDK
Write-Host "[7/11] Copying SDK files..."
Copy-Item "windows\anywp_sdk.js" "$RELEASE_PATH\sdk\" -Force
Write-Host "✓ SDK copied"

# Step 8: Copy Dart files
Write-Host "[8/11] Copying Dart files..."
Copy-Item "lib\anywp_engine.dart" "$RELEASE_PATH\lib\" -Force
Copy-Item "lib\anywp_engine.dart" "$RELEASE_PATH\lib\dart\" -Force
Write-Host "✓ Dart files copied"

# Step 9: Copy documentation
Write-Host "[9/11] Copying documentation..."
Copy-Item "README.md" "$RELEASE_PATH\" -Force
Copy-Item "CHANGELOG_CN.md" "$RELEASE_PATH\" -Force
Copy-Item "LICENSE" "$RELEASE_PATH\" -Force
Copy-Item "pubspec.yaml" "$RELEASE_PATH\" -Force
Copy-Item "docs\PRECOMPILED_DLL_INTEGRATION.md" "$RELEASE_PATH\PRECOMPILED_README.md" -Force
Write-Host "✓ Documentation copied"

# Step 10: Copy helper scripts
Write-Host "[10/11] Copying helper scripts..."
Copy-Item "templates\precompiled\*.bat" "$RELEASE_PATH\" -Force
Write-Host "✓ Helper scripts copied"

# Step 11: Create ZIP
Write-Host "[11/11] Creating ZIP archive..."
Compress-Archive -Path "$RELEASE_PATH\*" -DestinationPath "release\$RELEASE_NAME.zip" -Force
Write-Host "✓ ZIP created"

Write-Host ""
Write-Host "=========================================="
Write-Host "✅ Packaging Complete!"
Write-Host "=========================================="
Write-Host ""
Write-Host "Release package: release\$RELEASE_NAME.zip"
$zipFile = Get-Item "release\$RELEASE_NAME.zip"
Write-Host "Size: $([math]::Round($zipFile.Length/1MB,2)) MB"
Write-Host ""
Write-Host "Verifying critical files in package..."
Write-Host ""

# Verify by extracting to temp
$tempDir = "release\temp_verify_$VERSION"
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}
Expand-Archive -Path "release\$RELEASE_NAME.zip" -DestinationPath $tempDir -Force

$criticalFiles = @(
    "$tempDir\bin\anywp_engine_plugin.dll",
    "$tempDir\bin\WebView2Loader.dll",
    "$tempDir\lib\anywp_engine_plugin.lib",
    "$tempDir\lib\anywp_engine.dart",
    "$tempDir\lib\dart\anywp_engine.dart",
    "$tempDir\windows\src\anywp_engine_plugin.cpp",
    "$tempDir\windows\CMakeLists.txt",
    "$tempDir\sdk\anywp_sdk.js"
)

$allOk = $true
foreach ($file in $criticalFiles) {
    $exists = Test-Path $file
    $fileName = Split-Path $file -Leaf
    if ($exists) {
        Write-Host "  ✓ $fileName"
    } else {
        Write-Host "  ✗ $fileName - MISSING!"
        $allOk = $false
    }
}

Remove-Item $tempDir -Recurse -Force

Write-Host ""
if ($allOk) {
    Write-Host "✅ All critical files verified!"
} else {
    Write-Host "❌ Some files are missing!"
    exit 1
}

