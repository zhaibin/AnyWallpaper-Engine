# AnyWP Engine v1.3.2 - 🚀 Stable Release

## 📦 Pre-compiled Version - Quick Integration, No Compilation Required

**For**: Flutter developers who want to quickly integrate AnyWP Engine without installing WebView2 SDK or Visual Studio.

---

## 🎯 Major Updates

### ✨ New Features

#### C++ Modular Architecture
- 🏗️ **Modular Design**: Core plugin refactored from 4000+ lines to independent functional modules
- 📦 **5 Function Modules**: IframeDetector, SDKBridge, MouseHookManager, MonitorManager, PowerManager
- 🛠️ **3 Utility Classes**: StatePersistence, URLValidator, Logger
- 🧪 **Unit Testing Framework**: Lightweight C++ testing framework with auto-registration and rich assertions
- 🛡️ **Complete Error Handling**: All modules protected with try-catch and detailed logging

#### TypeScript SDK Rewrite
- ✅ **100% TypeScript**: Complete type-safe SDK implementation
- 📦 **Modular Architecture**: Independent modules for core, events, drag, click, storage, SPA, animations
- 🧪 **Unit Test Coverage**: 118 test cases, 96.6% pass rate, ~71% code coverage
- 🚀 **Modern Build Process**: Built with Rollup, supports ES Module and UMD

#### Documentation Standardization
- ✅ **Doc Validation System**: Automated validation of documentation accuracy and completeness
- 🔍 **7 Validation Types**: Language compliance, file references, version consistency, links, code examples, scripts, release packages
- 🌐 **English Standards**: Root directory documents in English, technical docs in bilingual format
- 📝 **Script Standards**: All scripts written in English, no special characters

### 🔧 Technical Improvements

#### Logging System
- 📊 **Multi-level Logging**: DEBUG, INFO, WARNING, ERROR
- 🔒 **Thread-Safe**: Protected concurrent writes using std::mutex
- 📤 **Dual Output**: Console and file output simultaneously
- ⏱️ **Millisecond Timestamps**: Precise time recording

#### Error Handling
- 🛡️ **Comprehensive Exception Catching**: Critical operations wrapped in try-catch
- 🔄 **Callback Protection**: Callback functions with additional exception protection layer
- ↩️ **State Rollback**: Automatic state recovery on operation failure
- 📝 **Detailed Error Info**: Complete error stack and Windows API error codes recorded

#### Testing System
- 🧪 **C++ Unit Tests**: Lightweight testing framework for rapid verification
- ✅ **TypeScript Tests**: Jest testing framework, complete coverage of all modules
- 🤖 **Integration Test Scripts**: Automated testing process with performance monitoring

---

## 📥 Download

### Pre-compiled DLL Package (Recommended)

**Main Package**:
- **Filename**: `anywp_engine_v1.3.2.zip`
- **Size**: ~40 MB
- **Contents**:
  - ✅ Pre-compiled DLL (`anywp_engine_plugin.dll`, `WebView2Loader.dll`)
  - ✅ Dart source code
  - ✅ C++ source code (modules, utils, tests)
  - ✅ JavaScript SDK (built from TypeScript)
  - ✅ TypeScript SDK source code
  - ✅ Header files (C++ API)
  - ✅ CMake configuration
  - ✅ WebView2 packages
  - ✅ Documentation

**Web SDK Package** (for web developers):
- **Filename**: `anywp_web_sdk_v1.3.2.zip`
- **Size**: ~69 KB
- **Contents**:
  - ✅ JavaScript SDK (`anywp_sdk.js`)
  - ✅ 8 HTML examples
  - ✅ Web developer guides
  - ✅ API documentation

---

## 🚀 Quick Start

### Method 1: Using Pre-compiled DLL (Recommended)

**1. Download and extract**
```bash
# Download anywp_engine_v1.3.2.zip and extract to project root
```

**2. Add to pubspec.yaml**
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.3.2
```

**3. Get dependencies and build**
```bash
flutter pub get
flutter build windows
```

**4. Start using**
```dart
import 'package:anywp_engine/anywp_engine.dart';

await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);
```

**Done!** No need to install WebView2 SDK or Visual Studio.

---

## 📋 System Requirements

- **OS**: Windows 10 or Windows 11
- **Flutter**: 3.0.0 or higher
- **Dart**: 3.0.0 or higher
- **WebView2 Runtime**: Built-in on Windows 11, Windows 10 users need to [download](https://developer.microsoft.com/microsoft-edge/webview2/)

---

## 📚 Complete Documentation

### Quick Start
- [Pre-compiled DLL Integration Guide](docs/PRECOMPILED_DLL_INTEGRATION.md) ⭐
- [Quick Start Guide](docs/QUICK_START.md)
- [Package Usage Guide](docs/PACKAGE_USAGE_GUIDE_CN.md)

### API Documentation
- [Developer API Reference](docs/DEVELOPER_API_REFERENCE.md)
- [API Usage Examples](docs/API_USAGE_EXAMPLES.md)

### Advanced Guides
- [Best Practices](docs/BEST_PRACTICES.md)
- [Web Developer Guide](docs/WEB_DEVELOPER_GUIDE_CN.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

---

## 🎓 Core API

### Dart API (Flutter)

```dart
// Initialize wallpaper
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// Stop wallpaper
await AnyWPEngine.stopWallpaper();

// Navigate to new URL
await AnyWPEngine.navigateToUrl('https://new-url.com');

// Get monitor list
List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();

// Power saving control
await AnyWPEngine.pauseWallpaper();        // Pause
await AnyWPEngine.resumeWallpaper();       // Resume
await AnyWPEngine.setAutoPowerSaving(true); // Auto power saving

// State management (Application-level isolation v1.2.0+)
await AnyWPEngine.setApplicationName('MyApp');
await AnyWPEngine.saveState('key', 'value');
String value = await AnyWPEngine.loadState('key');
```

### JavaScript SDK (Wallpaper Web Page)

```html
<!-- Include SDK -->
<script src="../windows/anywp_sdk.js"></script>

<script>
// Set interaction mode
AnyWP.setInteractionMode('interactive');  // or 'transparent'

// Element dragging
AnyWP.makeDraggable('#draggable-element', {
  persistKey: 'element_pos',
  boundary: { left: 0, top: 0, right: 800, bottom: 600 }
});

// Reset position
AnyWP.resetPosition('#element', { left: 100, top: 100 });

// Save state
AnyWP.saveState('myKey', 'myValue');

// Load state
AnyWP.loadState('myKey').then(value => {
  console.log('Loaded:', value);
});
</script>
```

---

## 🔄 Upgrading from Previous Versions

### From v1.3.1

**1. Download new version**
```bash
# Download anywp_engine_v1.3.2.zip
```

**2. Remove old version**
```bash
rmdir /s /q anywp_engine_v1.3.1
```

**3. Extract new version and update reference**
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.3.2  # Update version number
```

**4. Rebuild**
```bash
flutter clean
flutter pub get
flutter build windows
```

### Breaking Changes

- ✅ **No Breaking Changes**: v1.3.2 is fully compatible with v1.3.1

---

## 🎯 Example Code

### Complete Example Project

See the `example/` directory for a complete example application.

### Test HTML Files

The project includes multiple test HTML files:
- `examples/test_simple.html` - Simple wallpaper
- `examples/test_draggable.html` - Drag functionality test
- `examples/test_api.html` - Complete API test
- `examples/test_react.html` - React integration example
- `examples/test_vue.html` - Vue integration example
- `examples/test_iframe_ads.html` - iframe ads detection
- `examples/test_visibility.html` - Power saving test
- `examples/test_basic_click.html` - Click detection test

---

## 🐛 Known Issues

No major known issues. If you encounter any problems, please visit [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues).

---

## 🤝 Contributing

Welcome to submit Issues and Pull Requests!

- **Report Issues**: [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- **Feature Requests**: [GitHub Discussions](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- **Contribute Code**: Fork the project and submit PR

---

## 📜 License

This project is open-sourced under [MIT License](LICENSE).

---

## 🙏 Acknowledgments

Thanks to all contributors and users for their support!

---

**Release Date**: 2025-11-08  
**Version**: v1.3.2  
**Downloads**: [Auto-tracked by GitHub]

---

## 📞 Get Help

- 📖 View [Complete Documentation](https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs)
- 🐛 Submit [Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 Join [Discussion](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- 📧 Contact author: [GitHub @zhaibin](https://github.com/zhaibin)

