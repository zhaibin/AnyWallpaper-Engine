# AnyWP Engine v2.6.1 - macOS Production Optimization 🚀

## 🎯 Overview

This release focuses on **production environment optimization for macOS**, achieving **95% feature parity** with Windows. All core features are now fully tested and production-ready.

## ✨ Key Improvements

### 🍎 macOS Bundle Resource Integration

- ✅ **Fixed Test Page Access**: Resolved 404 errors caused by macOS sandboxing
- ✅ **Automated Build Scripts**: Examples automatically copied to App Bundle Resources
- ✅ **Intelligent Path Detection**: macOS prioritizes Bundle path, Windows maintains original logic
- ✅ **HTTP Server Optimization**: All 17 test pages now accessible

### 🔗 SDK Embedding Alignment

- ✅ **EmbeddedSDK Implementation**: 95KB SDK compiled into macOS Framework
- ✅ **Three-Tier Loading Strategy**: Embedded String > Bundle Resources > Fallback SDK
- ✅ **Windows Parity**: macOS (ObjC) vs Windows (RC) architectures fully aligned

### 🌐 LocalFileServer Implementation

- ✅ **NSURLProtocol Implementation**: Custom `localfile://` protocol
- ✅ **CORS Support**: Resolves cross-origin issues
- ✅ **MIME Type Detection**: Supports HTML/CSS/JS/images/videos
- ✅ **Sandbox Compatible**: Perfectly adapts to macOS security policies

### 🖱️ Interactive Mode Optimization

- ✅ **Permission-Free Solution**: WKWebView native interaction mode (no Accessibility permission required)
- ✅ **Dynamic Window Level Adjustment**: `setIgnoresMouseEvents` + `CGWindowLevel`
- ✅ **Feature Completeness**: 85% → 95%

## 🛠️ New Tools

### Automation Scripts

- `example/build_macos.sh`: Automated build script (build + copy + verify)
- `example/macos/copy_examples.sh`: Xcode build phase script
- `scripts/test_macos_fileserver.sh`: File server testing tool
- `scripts/generate_embedded_sdk_macos.sh`: SDK embedding generator

## 📝 Documentation Updates

### New Documentation

- `docs/PLATFORM_COMPARISON.md` (455 lines): Comprehensive Windows vs macOS comparison
- `docs/MACOS_FEATURE_ROADMAP.md` (745 lines): Feature completeness roadmap
- `docs/MACOS_FILE_ACCESS_FIX.md` (184 lines): Sandbox issue solutions

### Updated Documentation

- Updated `scripts/release_macos.sh`: Added automatic examples copying step
- Updated `example/lib/main.dart`: Intelligent path detection logic

## 📊 Feature Completeness

- **Windows**: 100% ✅
- **macOS**: 95% ✅ (Only missing 5% global keyboard listening, requires Accessibility permission)

## 🎁 Test Verification

- ✅ HTTP server starts successfully
- ✅ All 17 test pages accessible
- ✅ Carousel test page bidirectional communication works
- ✅ Interactive mode available
- ✅ Multi-monitor support works

## 🚀 Production Ready

- ✅ Complete core functionality
- ✅ No special permissions required
- ✅ Sandbox compatible
- ✅ Automated builds
- ✅ Comprehensive documentation
- ✅ Complete test environment

## 📦 Download Packages

Three packages are available for different use cases:

### 1. **Precompiled Package** (Recommended for Flutter Developers) ⭐
- `anywp_engine_macos_v2.6.1_precompiled.zip` (1.5MB)
- Contains: Framework + Headers + Dart API + SDK + Integration Guide
- Easiest integration, no build environment required
- SDK embedded in framework (no extra JS files)

### 2. **Source Package** (For Advanced Users)
- `anywp_engine_macos_v2.6.1_source.zip` (24MB)
- Contains: Full Objective-C source code + Modules + Utils
- Suitable for developers who need custom modifications
- Includes complete build scripts

### 3. **Web SDK Package** (For Wallpaper Developers)
- `anywp_web_sdk_v2.5.0.zip` (116KB)
- Cross-platform JavaScript SDK
- For HTML wallpaper development
- Works on both Windows and macOS

## 📖 Documentation

- [macOS Integration Guide](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PRECOMPILED_MACOS_INTEGRATION.md)
- [Web Developer Guide](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/WEB_DEVELOPER_GUIDE.md)
- [API Reference](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/DEVELOPER_API_REFERENCE.md)
- [Platform Comparison](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PLATFORM_COMPARISON.md)

## 🔧 Integration Example

### Using Precompiled Package (macOS)

```yaml
# pubspec.yaml
dependencies:
  anywp_engine:
    path: ./plugins/anywp_engine
```

```dart
// Initialize wallpaper
await AnyWPEngine.initializeWallpaper(
  'localfile:///examples/test_carousel_control.html',
  monitorIndex: 0,
);

// Enable interactive mode
await AnyWPEngine.setInteractiveMode(true);
```

## 🐛 Bug Fixes

- Fixed macOS test page 404 errors due to sandboxing
- Fixed HTTP server path detection in sandboxed environments
- Improved example app compatibility on macOS

## 📈 What's Changed

**Full Changelog**: [v2.6.0...v2.6.1](https://github.com/zhaibin/AnyWallpaper-Engine/compare/v2.6.0...v2.6.1)

## 🙏 Credits

Thanks to all contributors and testers who helped make this release possible!

---

**Platform Support**:
- ✅ Windows 10/11 (x64)
- ✅ macOS 10.13+ (Intel & Apple Silicon)

**Flutter Version**: 3.0.0+

