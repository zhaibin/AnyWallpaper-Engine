# Build Web SDK Independent Package
# For web developers who only need the JavaScript SDK

param(
    [string]$Version = "1.3.1"
)

$PACKAGE_NAME = "anywp_web_sdk_v$Version"
$PACKAGE_PATH = "release\$PACKAGE_NAME"

Write-Host "=========================================="
Write-Host "Building Web SDK Package v$Version"
Write-Host "=========================================="
Write-Host ""

# Step 1: Clean old package
Write-Host "[1/8] Cleaning old package..."
if (Test-Path $PACKAGE_PATH) {
    Remove-Item $PACKAGE_PATH -Recurse -Force
}
if (Test-Path "release\$PACKAGE_NAME.zip") {
    Remove-Item "release\$PACKAGE_NAME.zip" -Force
}
Write-Host "✓ Cleaned"

# Step 2: Create directory structure
Write-Host "[2/8] Creating directory structure..."
New-Item -ItemType Directory -Path "$PACKAGE_PATH\sdk" -Force | Out-Null
New-Item -ItemType Directory -Path "$PACKAGE_PATH\examples" -Force | Out-Null
New-Item -ItemType Directory -Path "$PACKAGE_PATH\docs" -Force | Out-Null
Write-Host "✓ Directories created"

# Step 3: Copy SDK file
Write-Host "[3/8] Copying SDK file..."
if (!(Test-Path "windows\anywp_sdk.js")) {
    Write-Host "[ERROR] SDK file not found: windows\anywp_sdk.js"
    exit 1
}
Copy-Item "windows\anywp_sdk.js" "$PACKAGE_PATH\sdk\" -Force
Write-Host "✓ SDK file copied"

# Step 4: Copy example HTML files
Write-Host "[4/8] Copying example files..."
if (!(Test-Path "examples\*.html")) {
    Write-Host "[WARNING] No example HTML files found in examples/"
} else {
    Copy-Item "examples\*.html" "$PACKAGE_PATH\examples\" -Force
    $exampleCount = (Get-ChildItem "$PACKAGE_PATH\examples\*.html").Count
    Write-Host "✓ $exampleCount example files copied"
}

# Step 5: Copy documentation
Write-Host "[5/8] Copying documentation..."
$docs = @{
    "docs\WEB_DEVELOPER_GUIDE_CN.md" = "$PACKAGE_PATH\docs\WEB_DEVELOPER_GUIDE_CN.md"
    "docs\WEB_DEVELOPER_GUIDE.md" = "$PACKAGE_PATH\docs\WEB_DEVELOPER_GUIDE.md"
    "docs\API_USAGE_EXAMPLES.md" = "$PACKAGE_PATH\docs\API_USAGE_EXAMPLES.md"
}
foreach ($src in $docs.Keys) {
    if (Test-Path $src) {
        Copy-Item $src $docs[$src] -Force
    } else {
        Write-Host "[WARNING] Document not found: $src"
    }
}
Write-Host "✓ Documentation copied"

# Step 6: Copy LICENSE
Write-Host "[6/8] Copying LICENSE..."
if (Test-Path "LICENSE") {
    Copy-Item "LICENSE" "$PACKAGE_PATH\" -Force
    Write-Host "✓ LICENSE copied"
} else {
    Write-Host "[WARNING] LICENSE file not found"
}

# Step 7: Create README.md for Web SDK
Write-Host "[7/8] Creating README.md..."
$readmeContent = @"
# AnyWP Web SDK v$Version

JavaScript SDK for creating interactive desktop wallpapers with AnyWP Engine.

## 📦 What's Included

- **SDK**: `sdk/anywp_sdk.js` - Core JavaScript SDK
- **Examples**: 8 sample HTML pages demonstrating SDK features
- **Documentation**: Complete guides in Chinese and English

## 🚀 Quick Start

### 1. Include the SDK

````html
<!DOCTYPE html>
<html>
<head>
    <script src="anywp_sdk.js"></script>
</head>
<body>
    <!-- Your content here -->
</body>
</html>
````

### 2. Use SDK Features

````javascript
// Make elements draggable
window.AnyWP.makeDraggable('my-element');

// Save state to Windows Registry
window.AnyWP.saveState('my-key', { x: 100, y: 200 });

// Load saved state
const state = window.AnyWP.loadState('my-key');

// Handle clicks
window.AnyWP.onClick('my-button', () => {
    console.log('Button clicked!');
});

// Detect visibility changes
window.AnyWP.onVisibilityChange((isVisible) => {
    console.log('Wallpaper visible:', isVisible);
});
````

## 📚 Documentation

- **Chinese Guide**: `docs/WEB_DEVELOPER_GUIDE_CN.md`
- **English Guide**: `docs/WEB_DEVELOPER_GUIDE.md`
- **API Examples**: `docs/API_USAGE_EXAMPLES.md`

## 🎨 Example Files

1. **test_simple.html** - Basic wallpaper
2. **test_draggable.html** - Draggable elements demo
3. **test_api.html** - Complete API showcase
4. **test_click.html** - Click event handling
5. **test_visibility.html** - Power saving demo
6. **test_react.html** - React integration
7. **test_vue.html** - Vue.js integration
8. **test_iframe_ads.html** - iFrame handling

## 🔧 Core API

### Element Dragging
````javascript
window.AnyWP.makeDraggable(elementId, options)
````

### State Persistence
````javascript
window.AnyWP.saveState(key, data)
window.AnyWP.loadState(key)
window.AnyWP.clearState(key)
````

### Event Handling
````javascript
window.AnyWP.onClick(elementId, callback)
window.AnyWP.onVisibilityChange(callback)
````

### Utility
````javascript
window.AnyWP.getStoragePath()
window.AnyWP.log(message)
````

## ⚠️ Requirements

- **AnyWP Engine**: v1.3.0 or higher
- **Operating System**: Windows 10/11
- **WebView2 Runtime**: Automatically included

## 📖 Full Documentation

For complete documentation, please refer to:
- [Flutter Developer Guide](https://github.com/zhaibin/AnyWallpaper-Engine)
- [Web Developer Guide](docs/WEB_DEVELOPER_GUIDE_CN.md)

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🔗 Links

- **GitHub**: https://github.com/zhaibin/AnyWallpaper-Engine
- **Releases**: https://github.com/zhaibin/AnyWallpaper-Engine/releases

---

**Version**: $Version | **Release Date**: $(Get-Date -Format 'yyyy-MM-dd')
"@

Set-Content -Path "$PACKAGE_PATH\README.md" -Value $readmeContent -Encoding UTF8
Write-Host "✓ README.md created"

# Step 8: Create ZIP archive
Write-Host "[8/8] Creating ZIP archive..."
if (Test-Path "release\$PACKAGE_NAME.zip") {
    Remove-Item "release\$PACKAGE_NAME.zip" -Force
}
Compress-Archive -Path "$PACKAGE_PATH\*" -DestinationPath "release\$PACKAGE_NAME.zip" -Force
Write-Host "✓ ZIP archive created"

# Verify
Write-Host ""
Write-Host "=========================================="
Write-Host "✅ Web SDK Package Complete!"
Write-Host "=========================================="
Write-Host ""

$zipFile = Get-Item "release\$PACKAGE_NAME.zip"
Write-Host "Package: release\$PACKAGE_NAME.zip"
Write-Host "Size: $([math]::Round($zipFile.Length/1KB,2)) KB"
Write-Host ""

Write-Host "Contents:"
Write-Host "  • SDK: anywp_sdk.js"
Write-Host "  • Examples: $(if (Test-Path "$PACKAGE_PATH\examples\*.html") { (Get-ChildItem "$PACKAGE_PATH\examples\*.html").Count } else { 0 }) HTML files"
Write-Host "  • Documentation: $(if (Test-Path "$PACKAGE_PATH\docs\*.md") { (Get-ChildItem "$PACKAGE_PATH\docs\*.md").Count } else { 0 }) guides"
Write-Host "  • README.md"
Write-Host "  • LICENSE"
Write-Host ""

