# AnyWP Engine v2.6.0 - Cross-Platform Release 🎉

**Release Date**: December 8, 2025  
**Engine Version**: 2.6.0  
**SDK Version**: 2.5.0

---

## 🌍 Major Update: macOS Platform Support

We are excited to announce that **AnyWP Engine** now supports **macOS**! This release merges the macOS support branch (v2.2.0) into the main codebase, making AnyWP Engine a truly cross-platform desktop wallpaper engine.

### Highlights

- ✅ **Native macOS Plugin**: Built with Objective-C + AppKit + WKWebView
- ✅ **Unified Dart API**: Same API for Windows and macOS
- ✅ **Cross-Platform JavaScript SDK**: Automatically adapts to platform-specific messaging (WebView2 on Windows, WKWebView on macOS)
- ✅ **Multi-Monitor Support**: Full support for multiple displays on macOS via NSScreen API
- ✅ **Power Management**: Screen sleep, session lock, and idle detection via NSWorkspace notifications
- ✅ **State Persistence**: Application-scoped data storage using macOS Application Support directory
- ✅ **Memory Optimization**: Optimized thresholds for WKWebView (200MB default)
- ✅ **File Encryption/Decryption**: XOR-based file encryption support on macOS

---

## 📦 Download Packages

### For Flutter Developers

**Windows:**
- **`anywp_engine_v2.6.0_precompiled.zip`** ⭐ Recommended
  - Precompiled DLL + LIB + Pure C API headers
  - No WebView2 development environment required
  - Easiest integration for Flutter apps

**macOS:**
- **`anywp_engine_macos_v2.6.0_precompiled.zip`** ⭐ Recommended
  - Precompiled framework + CocoaPods integration
  - Only requires Xcode Command Line Tools
  - Simplest integration for Flutter apps

### For Advanced Developers

**Windows:**
- **`anywp_engine_v2.6.0_source.zip`**
  - Complete C++ source code
  - All modules and utility classes
  - WebView2 SDK included
  - For developers who need to customize or debug

**macOS:**
- **`anywp_engine_macos_v2.6.0_source.zip`**
  - Complete Objective-C source code
  - All modules and utility classes
  - For developers who need to customize or debug

### For Wallpaper Developers

- **`anywp_web_sdk_v2.5.0.zip`** (Cross-platform)
  - JavaScript SDK for HTML wallpapers
  - Works on both Windows and macOS
  - Includes examples and documentation

---

## 🚀 Platform Comparison

| Feature | Windows | macOS |
|---------|---------|-------|
| **WebView Engine** | WebView2 (Chromium) | WKWebView (WebKit) |
| **Native Implementation** | C++17 + Win32 API | Objective-C + AppKit |
| **Multi-Monitor** | ✅ Full support | ✅ Full support |
| **Power Management** | ✅ Full support | ✅ Full support |
| **Interactive Mode** | ✅ Full support | ⚠️ Not yet (requires Accessibility permissions) |
| **File Access** | ✅ Local files | ⚠️ Sandboxed |
| **Memory Usage** | ~100-150MB | ~150-200MB |
| **JavaScript SDK** | ✅ Unified API | ✅ Unified API |

---

## 📚 Documentation

### Integration Guides
- **Windows**: `docs/PRECOMPILED_DLL_INTEGRATION.md`
- **macOS**: `docs/PRECOMPILED_MACOS_INTEGRATION.md`
- **Cross-Platform**: `docs/CROSS_PLATFORM_INTEGRATION.md`

### Developer Guides
- **Flutter Developers**: `docs/FOR_FLUTTER_DEVELOPERS.md`
- **Web Developers**: `docs/WEB_DEVELOPER_GUIDE_CN.md` / `docs/WEB_DEVELOPER_GUIDE.md`
- **API Reference**: `docs/DEVELOPER_API_REFERENCE.md`
- **macOS Development**: `docs/MACOS_DEVELOPER_GUIDE.md`

### Architecture & Technical
- **Multi-Platform Architecture**: `docs/MULTIPLATFORM_ARCHITECTURE.md`
- **Technical Notes**: `docs/TECHNICAL_NOTES.md`
- **API Bridge**: `docs/API_BRIDGE.md`

---

## 🔧 Technical Details

### macOS Modular Architecture
- **MonitorManager**: Multi-display management via NSScreen API
- **WallpaperManager**: Wallpaper window lifecycle and positioning
- **PowerManager**: Power event monitoring (sleep, lock, idle)
- **MessageBridge**: JavaScript ↔ Native communication bridge

### Shared Components
- **Unified Dart API Layer**: Same API for both platforms
- **Cross-Platform TypeScript SDK**: Automatic platform detection and adaptation
- **Unified Message Protocol**: Consistent communication between web and native layers

### Build Scripts
- **`scripts/release_macos.sh`**: Build macOS release packages
- **`scripts/build_sdk.sh`**: Build cross-platform Web SDK
- **`scripts/verify_precompiled_macos.sh`**: Verify package integrity

---

## ⚠️ Known Limitations (macOS)

1. **Interactive Mode**: Not yet implemented (requires Accessibility permissions for global input interception)
2. **File Access**: Subject to macOS sandbox restrictions; prefer `https://` over `file://` URLs
3. **Memory Usage**: WKWebView typically uses more memory than WebView2 (150-200MB vs 100-150MB)

---

## 📖 Quick Start

### Windows Integration

```yaml
dependencies:
  anywp_engine:
    path: ./packages/anywp_engine
```

See `docs/PRECOMPILED_DLL_INTEGRATION.md` for detailed instructions.

### macOS Integration

```yaml
dependencies:
  anywp_engine:
    path: ./packages/anywp_engine_macos
```

See `docs/PRECOMPILED_MACOS_INTEGRATION.md` for detailed instructions.

### Web Wallpaper Development

```html
<script src="sdk/anywp_sdk.js"></script>
<script>
  AnyWP.onClick(element, () => {
    console.log('Clicked!');
  });
</script>
```

See `docs/WEB_DEVELOPER_GUIDE.md` for API documentation.

---

## 🎯 System Requirements

### Windows
- Windows 10 version 1809 (build 17763) or later
- WebView2 Runtime installed
- Flutter 3.0+
- Visual Studio 2019+ (for source builds)

### macOS
- macOS 10.14+
- Flutter 3.0+
- Xcode 12+ / Xcode Command Line Tools

---

## 🙏 Feedback & Support

- **Issues**: [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- **Discussions**: [GitHub Discussions](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- **Documentation**: [Complete Documentation Index](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/DOCUMENTATION_INDEX.md)

---

## 📝 Full Changelog

See [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md) for complete version history.

---

**License**: MIT  
**Repository**: https://github.com/zhaibin/AnyWallpaper-Engine

