# AnyWP Engine v2.1.3

## 🔧 Code Quality Improvements

### Unified Log Format Specification

**New**: Added complete log format specification documentation in `windows/utils/logger.h`.

**Format**: `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [COMPONENT] message`

**Example**: `[2025-01-15 10:30:45.123] [INFO] [Plugin] Plugin initialized`

**Guidelines**:
- Component names use PascalCase (e.g., "WebViewManager", "FlutterBridge")
- All log messages use English, no emoji or special symbols
- Appropriate log levels: DEBUG for detailed info, INFO for normal operations, WARNING for recoverable issues, ERROR for failures

### Internationalization Improvements

**Removed**: All emoji symbols from log output (✅ ❌ etc.)

**Unified**: All Chinese comments changed to English

**Standardized**: All log messages use English, conforming to internationalization standards

## 📝 Code Changes

- `windows/utils/logger.h`: Added log format specification documentation
- `windows/utils/logger.cpp`: Unified log format implementation
- `windows/modules/sdk_bridge.cpp`: Removed emoji, unified to English logs
- `windows/modules/mouse_hook_manager.cpp`: Removed emoji, unified to English logs
- `windows/modules/flutter_bridge.cpp`: Changed Chinese comments to English
- `windows/anywp_engine_plugin.cpp/h`: Changed Chinese comments to English
- `windows/modules/sdk_bridge.h`: Changed Chinese comments to English
- `windows/modules/power_manager.h`: Changed Chinese comments to English
- `windows/modules/memory_optimizer.h`: Changed Chinese comments to English
- `windows/CMakeLists.txt`: Added precompiled DLL detection logic
- `windows/CMakeLists.precompiled.txt`: New dedicated CMakeLists.txt for precompiled packages
- `scripts/release.bat`: Modified to use precompiled CMakeLists.txt

## 🐛 Bug Fixes

### Precompiled Package WebView2 Dependency Issue

**Problem**: CMakeLists.txt in precompiled package was still looking for WebView2 packages directory, causing build failures.

**Solution**:
- Created `windows/CMakeLists.precompiled.txt` - dedicated CMakeLists.txt for precompiled packages
- Uses `IMPORTED` library to directly link precompiled DLL, skipping WebView2 packages check
- Modified `scripts/release.bat` to use dedicated CMakeLists.txt for precompiled packages

**Impact**:
- ✅ Precompiled packages can now build successfully without WebView2 SDK
- ✅ Simplified integration process, truly zero-dependency integration
- ✅ Fixed "WebView2 package not found" error

## 💡 Impact

- ✅ Unified log format for easier parsing and analysis
- ✅ Internationalization-friendly, supporting multi-language environments
- ✅ Improved code readability with unified English comments
- ✅ Conforms to international development standards
- ✅ Precompiled package integration issues completely resolved

## 📦 Packages

This release includes three packages:

### 1. Precompiled Package (Recommended)
**File**: `anywp_engine_v2.1.3_precompiled.zip`

**Contents**:
- ✅ Precompiled DLL files (`bin/`)
- ✅ LIB files (`lib/`)
- ✅ Pure C API headers (`include/anywp_engine/`)
- ✅ Dart API (`lib/dart/`)
- ✅ CMake configuration (`windows/CMakeLists.txt`)
- ✅ Documentation and license

**Best for**: Flutter developers who want minimal integration without WebView2 SDK.

### 2. Source Package
**File**: `anywp_engine_v2.1.3_source.zip`

**Contents**: Everything in precompiled package plus:
- ✅ Complete C++ source code (`windows/anywp_engine_plugin.cpp/h`)
- ✅ All modules (`windows/modules/`)
- ✅ All utilities (`windows/utils/`)
- ✅ TypeScript SDK source (`windows/sdk/`)
- ✅ WebView2 packages (`windows/packages/`)
- ✅ C++ unit tests (`windows/test/`)

**Best for**: Developers who need to modify or rebuild from source.

### 3. Web SDK Package
**File**: `anywp_web_sdk_v2.1.3.zip`

**Contents**:
- ✅ JavaScript SDK (`sdk/anywp_sdk.js`)
- ✅ Example HTML files (`examples/`)
- ✅ Documentation

**Best for**: Web wallpaper developers (HTML/CSS/JS).

## 🔗 Links

- [Integration Guide](docs/PRECOMPILED_DLL_INTEGRATION.md)
- [Developer API Reference](docs/DEVELOPER_API_REFERENCE.md)
- [Web Developer Guide](docs/WEB_DEVELOPER_GUIDE.md)

## 📄 Full Changelog

See [CHANGELOG_CN.md](../CHANGELOG_CN.md) for complete changelog.

