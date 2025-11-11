# AnyWP Engine v2.0.0 - 🚀 Modular Architecture Refactoring

**Release Date:** November 11, 2025

## 🎉 Release Highlights

This is a **major architectural upgrade** that transforms AnyWP Engine from a monolithic plugin into a modern, modular system. The refactoring achieves:

- **42.9% code reduction** in main plugin (4,448 → 2,540 lines)
- **78% modularity** through 27 specialized modules
- **209+ unit tests** achieving 95%+ coverage
- **Zero performance loss** with 55% faster compilation
- **100% backward compatibility** - no API changes

---

## ✨ Key Features

### 🏗️ Modular Architecture

**Core Modules** (13 modules in `windows/modules/`):
- **FlutterBridge** - Flutter method channel communication
- **DisplayChangeCoordinator** - Display change detection
- **InstanceManager** - Instance lifecycle management
- **WindowManager** - Window creation and management
- **InitializationCoordinator** - Initialization flow coordination
- **WebViewManager** - WebView2 management
- **WebViewConfigurator** - WebView2 security configuration
- **PowerManager** - Power saving optimization
- **IframeDetector** - iFrame detection
- **SDKBridge** - SDK bridging
- **MouseHookManager** - Mouse hook management
- **MonitorManager** - Monitor enumeration
- **EventDispatcher** ⭐ - High-performance event routing (NEW)

**Utility Classes** (14 utilities in `windows/utils/`):
- StatePersistence, StartupOptimizer, CPUProfiler, MemoryProfiler
- InputValidator, ConflictDetector, DesktopWallpaperHelper
- **Logger** ⭐ - Enhanced with buffering, rotation, statistics
- URLValidator, ResourceTracker
- **ErrorHandler** ⭐ - Unified error handling system (NEW)
- **PerformanceBenchmark** ⭐ - Performance testing framework (NEW)
- **PermissionManager** ⭐ - Permission management system (NEW)
- **EventBus** ⭐ - Event bus system (NEW)
- **ConfigManager** ⭐ - Configuration management (NEW)
- **ServiceLocator** ⭐ - Dependency injection container (NEW)
- CircuitBreaker, RetryHandler, SafetyMacros (header-only)

### 🧪 Comprehensive Testing

**C++ Unit Tests** (`windows/test/`):
- 209+ test cases across 4 test files
- Lightweight custom test framework
- 95%+ code coverage
- Automated test execution via `run_tests.bat`

Test files:
- `unit_tests.cpp` (1,387 lines) - Core module tests
- `webview_manager_tests.cpp` (372 lines) - WebView integration tests
- `comprehensive_test.cpp` (324 lines) - End-to-end tests
- `test_framework.h` - Custom test framework

### 🔒 Enhanced Reliability

- **Error Recovery**: RetryHandler and CircuitBreaker patterns
- **Exception Safety**: try-catch protection on all critical operations
- **Callback Protection**: Extra exception guards for callbacks
- **Unified Logging**: Thread-safe Logger with rotation and buffering
- **Resource Tracking**: Automatic memory leak detection

### 📊 Performance Improvements

- **Compilation Speed**: 55% faster (modular builds)
- **Memory Efficiency**: Optimized resource management
- **Event Routing**: High-performance EventDispatcher
- **Startup Time**: Improved through StartupOptimizer

---

## 🎯 Usage Examples

### Basic Wallpaper Setup

```dart
import 'package:anywp_engine/anywp_engine.dart';

// Start wallpaper on primary monitor
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
);

// Stop wallpaper
await AnyWPEngine.stopWallpaper();
```

### Multi-Monitor Setup

```dart
// Get all monitors
final monitors = await AnyWPEngine.getMonitors();

// Start wallpaper on specific monitor
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  monitorIndex: 0,
);

// Start on all monitors
await AnyWPEngine.initializeWallpaperOnAllMonitors(
  url: 'https://example.com',
);
```

### Monitor Change Detection

```dart
// Set up monitor change callback
AnyWPEngine.setOnMonitorChangeCallback(() {
  print('Monitor configuration changed!');
  // Reload wallpapers
});
```

### Application Isolation

```dart
// Set application name for isolated storage
await AnyWPEngine.setApplicationName('MyAwesomeApp');

// Storage location: %LOCALAPPDATA%\AnyWPEngine\MyAwesomeApp\
```

---

## 📦 Download Options

### 1️⃣ Flutter Plugin Package (~38 MB)
**For Flutter developers** - Complete plugin with DLL, LIB, source code, and documentation

**Contents:**
- Precompiled DLLs (`bin/`)
- Link library (`lib/anywp_engine_plugin.lib`)
- C++ source code (`windows/src/`, `windows/modules/`, `windows/utils/`)
- Dart API (`lib/anywp_engine.dart`)
- Web SDK (`windows/anywp_sdk.js`)
- TypeScript SDK source (`windows/sdk/`)
- CMake configuration
- Complete documentation

**Quick Integration:**
```bash
# Extract to your Flutter project
Expand-Archive anywp_engine_v2.0.0.zip -DestinationPath path/to/flutter_project/

# Follow integration guide
cd path/to/flutter_project
# See PRECOMPILED_README.md
```

### 2️⃣ Web SDK Package (~80 KB)
**For Web developers** - Standalone JavaScript SDK with examples and documentation

**Contents:**
- JavaScript SDK (`sdk/anywp_sdk.js`)
- 13 HTML test pages
- API documentation (Chinese & English)
- Usage examples

---

## 🔧 Technical Details

### Architecture Benefits

1. **Single Responsibility**: Each module handles one specific function
2. **Low Coupling**: Modules interact through interfaces
3. **Easy Testing**: Independent modules simplify unit testing
4. **Backward Compatible**: No changes to external APIs
5. **Complete Documentation**: All public interfaces documented
6. **Error Recovery**: RetryHandler and CircuitBreaker for resilience

### Module Organization

```
windows/
├── modules/          # 13 core modules (business logic)
├── utils/            # 14 utility classes (helpers)
├── test/             # Unit tests (209+ tests)
├── sdk/              # TypeScript SDK source
├── anywp_sdk.js      # Built SDK (from TypeScript)
├── anywp_engine_plugin.cpp/h  # Main plugin (now 2,540 lines)
└── CMakeLists.txt    # Build configuration
```

### Adding New Modules

```cpp
// 1. Create windows/modules/my_module.h/cpp
class MyModule {
public:
    bool Initialize();
    void Cleanup();
};

// 2. Add error handling
try {
    MyModule module;
    if (!module.Initialize()) {
        Logger::Instance().Error("MyModule", "Init failed");
        return false;
    }
} catch (const std::exception& e) {
    Logger::Instance().Error("MyModule", e.what());
    return false;
}

// 3. Add unit tests in windows/test/unit_tests.cpp
TEST_SUITE(MyModule) {
    TEST_CASE(basic_functionality) {
        MyModule module;
        ASSERT_TRUE(module.Initialize());
    }
}
```

---

## 📚 Documentation

- **[README.md](../README.md)** - Project overview and quick start
- **[FOR_FLUTTER_DEVELOPERS.md](../docs/FOR_FLUTTER_DEVELOPERS.md)** - Flutter developer guide
- **[DEVELOPER_API_REFERENCE.md](../docs/DEVELOPER_API_REFERENCE.md)** - Complete API reference
- **[WEB_DEVELOPER_GUIDE_CN.md](../docs/WEB_DEVELOPER_GUIDE_CN.md)** - Web SDK guide (Chinese)
- **[WEB_DEVELOPER_GUIDE.md](../docs/WEB_DEVELOPER_GUIDE.md)** - Web SDK guide (English)
- **[PRECOMPILED_DLL_INTEGRATION.md](../docs/PRECOMPILED_DLL_INTEGRATION.md)** - Integration guide
- **[TESTING_GUIDE.md](../docs/TESTING_GUIDE.md)** - Testing guide
- **[CHANGELOG_CN.md](../CHANGELOG_CN.md)** - Complete changelog (Chinese)

---

## 🧪 Testing Results

### C++ Unit Tests
- **Total Tests**: 209+
- **Pass Rate**: 96.6% (202 passed, 7 skipped)
- **Code Coverage**: 95%+
- **Test Files**: 4 (unit_tests.cpp, webview_manager_tests.cpp, comprehensive_test.cpp)

### TypeScript SDK Tests
- **Total Tests**: 118
- **Pass Rate**: 96.6% (114 passed, 4 skipped)
- **Code Coverage**: ~71%
- **Test Modules**: 9 (storage, animations, events, click, drag, spa, debug, bounds, coordinates)

### Integration Tests
- ✅ Multi-monitor support
- ✅ Drag and drop functionality
- ✅ State persistence
- ✅ Application isolation
- ✅ Power management
- ✅ Monitor change detection

---

## ⚠️ Known Issues

None for this release.

---

## 🆙 Upgrade Guide

### From v1.x.x to v2.0.0

**Good News:** 100% backward compatible! No code changes required.

```dart
// Your existing code works as-is
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
);
```

**Optional Enhancements:**
```dart
// NEW: Set application name for storage isolation
await AnyWPEngine.setApplicationName('MyApp');

// NEW: Monitor change callbacks (same API, improved reliability)
AnyWPEngine.setOnMonitorChangeCallback(() {
  // Your existing callback code
});
```

---

## 🙏 Acknowledgments

This release represents a complete architectural refactoring of the AnyWP Engine plugin. Special thanks to all contributors and users who provided feedback during development.

---

## 📝 License

MIT License - See [LICENSE](../LICENSE) file

---

## 🔗 Links

- **GitHub Repository**: https://github.com/zhaibin/AnyWallpaper-Engine
- **Issues**: https://github.com/zhaibin/AnyWallpaper-Engine/issues
- **Documentation**: https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs

---

**Full Changelog**: v1.3.3...v2.0.0

