# AnyWP Engine v2.1.2

## ⚡ Performance Optimization

### WebMessage Polling Interval Optimization

**Optimization**: Changed webmessage pending messages polling interval from 100ms to 1 second.

**Benefits**:
- ✅ Reduced CPU usage (10x lower polling frequency)
- ✅ Reduced method call overhead
- ✅ Messages still processed promptly (batched processing, delay < 1 second)

**Technical Details**:
- `lib/anywp_engine.dart`: Updated `_startMessagePolling()` method
- Polling interval: `Duration(milliseconds: 100)` → `Duration(seconds: 1)`

## 📦 Packages

This release includes three packages:

### 1. Precompiled Package (Recommended)
**File**: `anywp_engine_v2.1.2_precompiled.zip`

**Contents**:
- ✅ Precompiled DLL files (`bin/`)
- ✅ LIB files (`lib/`)
- ✅ Pure C API headers (`include/anywp_engine/`)
- ✅ Dart API (`lib/dart/`)
- ✅ CMake configuration (`windows/CMakeLists.txt`)
- ✅ Documentation and license

**Best for**: Flutter developers who want minimal integration without WebView2 SDK.

### 2. Source Package
**File**: `anywp_engine_v2.1.2_source.zip`

**Contents**: Everything in precompiled package plus:
- ✅ Complete C++ source code (`windows/anywp_engine_plugin.cpp/h`)
- ✅ All modules (`windows/modules/`)
- ✅ All utilities (`windows/utils/`)
- ✅ TypeScript SDK source (`windows/sdk/`)
- ✅ WebView2 packages (`windows/packages/`)
- ✅ C++ unit tests (`windows/test/`)

**Best for**: Developers who need to modify or rebuild from source.

### 3. Web SDK Package
**File**: `anywp_web_sdk_v2.1.2.zip`

**Contents**:
- ✅ JavaScript SDK (`sdk/anywp_sdk.js`)
- ✅ Example HTML files (`examples/`)
- ✅ Documentation

**Best for**: Web wallpaper developers (HTML/CSS/JS).

## 📝 Code Changes

- `lib/anywp_engine.dart`: Updated message polling interval and log messages

## 🔗 Links

- [Integration Guide](docs/PRECOMPILED_DLL_INTEGRATION.md)
- [Developer API Reference](docs/DEVELOPER_API_REFERENCE.md)
- [Web Developer Guide](docs/WEB_DEVELOPER_GUIDE.md)

## 📄 Full Changelog

See [CHANGELOG_CN.md](../CHANGELOG_CN.md) for complete changelog.

