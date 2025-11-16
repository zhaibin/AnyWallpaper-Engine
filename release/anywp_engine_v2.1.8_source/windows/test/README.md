# AnyWP Engine - Test Suite

## 📂 Test Files Overview

### Core Test Framework
- **`test_framework.h`** (4 KB)
  - Lightweight C++ testing framework
  - Macros: `TEST_SUITE`, `TEST_CASE`, `ASSERT_*`
  - No external dependencies

### Test Suites

#### 1. Comprehensive Test (推荐)
- **`comprehensive_test.cpp`** (9 KB)
  - Tests all independent modules
  - **13 test suites**, covering:
    - Logger
    - PowerManager
    - URLValidator
    - StatePersistence
    - IframeDetector
    - MouseHookManager
    - MonitorManager
    - SDKBridge
    - MemoryProfiler (Phase 3)
    - CPUProfiler (Phase 3)
    - StartupOptimizer (Phase 3)
    - DesktopWallpaperHelper
    - ResourceTracker
  - Fast compilation (~10s)
  - No Flutter dependencies

- **`run_comprehensive_test.bat`** (2.5 KB)
  - Compiles and runs comprehensive tests
  - Auto-detects VS environment
  - Usage: Run from Developer Command Prompt

#### 2. Full Unit Tests
- **`unit_tests.cpp`** (40 KB)
  - Complete test suite with all modules
  - Includes Phase 2 modules (disabled by default)
  - **209 test cases** total
  - Requires Flutter headers for Phase 2 tests

- **`run_tests.bat`** (4 KB)
  - Original test runner
  - Compiles standalone modules only
  - Includes WebViewManager tests

#### 3. WebView Manager Tests
- **`webview_manager_tests.cpp`** (11 KB)
  - Dedicated WebView2 integration tests
  - Tests asynchronous operations
  - Requires WebView2Loader

### Build Configuration
- **`CMakeLists.txt`** (2 KB)
  - CMake build configuration
  - Links WebView2, Win32 libraries

## 🚀 Quick Start

### Run Comprehensive Tests (Recommended)
```batch
cd windows\test
# From Developer Command Prompt:
run_comprehensive_test.bat
```

### Run Full Unit Tests
```batch
cd windows\test
# From Developer Command Prompt:
run_tests.bat
```

## 📊 Test Results

### Latest Run (v2.0)
```
Comprehensive Test:
  - Total: 13 test suites
  - Passed: ✅ All tests
  - Time: ~1 second
  - Memory: ~6 MB

Performance Metrics:
  - Startup: 16 ms
  - CPU: 1%
  - Memory: 5.8 MB working set
```

## 🔧 Compilation

### Requirements
- Visual Studio 2022 Build Tools
- Windows SDK
- WebView2 SDK (1.0.2592.51)

### Libraries
- `psapi.lib` - Process API
- `User32.lib` - Windows User API
- `Ole32.lib` - COM
- `OleAut32.lib` - COM Automation
- `Shlwapi.lib` - Shell Lightweight API
- `WebView2LoaderStatic.lib` - WebView2

## 📝 Test Coverage

### Phase 3 Modules (v2.0) - ✅ 100%
- MemoryProfiler: 4 tests
- CPUProfiler: 4 tests
- StartupOptimizer: 4 tests

### Phase 2 Modules (v2.0) - Integration Tests
- InstanceManager: 10 tests (disabled in unit tests)
- WindowManager: 6 tests (disabled in unit tests)
- DisplayChangeCoordinator: 4 tests (disabled in unit tests)

### Phase 1 & Utilities - ✅ Covered
- Logger: 2 tests
- PowerManager: 2 tests
- URLValidator: 3 tests
- StatePersistence: 2 tests
- IframeDetector: 3 tests
- MouseHookManager: 2 tests
- MonitorManager: 3 tests
- SDKBridge: 2 tests
- DesktopWallpaperHelper: 2 tests
- ResourceTracker: 1 test

## 🎯 Testing Strategy

### Unit Tests
- Independent modules only
- No Flutter dependencies
- Fast compilation and execution
- Use `comprehensive_test.cpp`

### Integration Tests
- Phase 2 modules (require Flutter)
- Full plugin functionality
- Use `unit_tests.cpp` with Flutter build

### Performance Tests
- Memory profiling
- CPU profiling
- Startup optimization
- Included in comprehensive tests

## 📚 Documentation

- Test Framework: See `test_framework.h`
- Module Tests: See `comprehensive_test.cpp`
- Full Suite: See `unit_tests.cpp`
- Build System: See `CMakeLists.txt`

## 🔍 Troubleshooting

### Compiler Not Found
```
ERROR: Visual Studio compiler not found
```
**Solution**: Run from "Developer Command Prompt for VS"

### WebView2 Package Not Found
```
ERROR: WebView2 package not found
```
**Solution**: Check `windows/packages/` for WebView2 SDK

### Link Errors
```
LNK2019: unresolved external symbol
```
**Solution**: Ensure all required .cpp files are in compilation command

## 📄 Version History

- **v2.0** (2025-11-10): Phase 3 performance optimization complete
- **v1.3.3**: Phase 2 modularity enhancement
- **v1.3.2**: Initial modular architecture

---

**Last Updated**: 2025-11-10  
**Status**: ✅ All tests passing  
**Coverage**: 13 test suites, 100% independent modules


