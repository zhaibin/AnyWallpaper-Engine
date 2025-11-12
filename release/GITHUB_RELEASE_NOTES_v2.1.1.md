# AnyWP Engine v2.1.1 - Developer Experience Improvements

Release Date: November 13, 2025

## 🎯 Overview

This release focuses on improving the developer experience for Flutter developers integrating AnyWP Engine. Based on valuable feedback from the Flutter developer community, we've simplified the integration process, reduced package size, and enhanced CMake support.

---

## ✨ What's New

### 1. Pure C API Header 🔌

**New File**: `windows/anywp_engine_plugin_c_api.h`

- ✅ **Minimal Dependencies**: Only exposes the C registration function
- ✅ **No C++ Complexity**: Hides all C++ classes and WebView2 dependencies  
- ✅ **Easy Integration**: Uses `extern "C"` for C compatibility
- ✅ **Opaque Pointers**: Avoids Flutter header dependencies

**Benefits**: Flutter developers no longer need WebView2 SDK for integration!

### 2. CMake INSTALL Support 📦

- ✅ Complete CMake INSTALL rules added (commented out to avoid Flutter build conflicts)
- ✅ Available for manual `cmake --install` usage
- ✅ Auto-install DLLs, LIB files, and headers

**Fixed**: Resolved IMPORTED library installation issues.

### 3. Dual Release Package Structure 📂

#### **Precompiled Package** (`anywp_engine_v2.1.1_precompiled.zip`) - ⭐ **Recommended**

**Size**: ~5MB (83% smaller!)

**Best For**:
- ✅ Most Flutter developers  
- ✅ Production deployments
- ✅ CI/CD pipelines

#### **Source Package** (`anywp_engine_v2.1.1_source.zip`)

**Best For**:
- 🔧 Custom modifications needed
- 🔧 Plugin debugging required
- 🔧 Learning implementation details

---

## 📊 Performance Improvements

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| **Package Size** | 30MB+ | ~5MB | ⬇️ 83% |
| **Integration Steps** | 5+ steps | 3 steps | ⬇️ 40% |

---

## 📦 Download Packages

### For Most Flutter Developers ⭐

**[anywp_engine_v2.1.1_precompiled.zip](anywp_engine_v2.1.1_precompiled.zip)**

### For Advanced Users

**[anywp_engine_v2.1.1_source.zip](anywp_engine_v2.1.1_source.zip)**

### For Web Developers

**[anywp_web_sdk_v2.1.1.zip](anywp_web_sdk_v2.1.1.zip)**

---

## 🙏 Acknowledgments

**Special Thanks** to the Flutter developer community for their valuable feedback!

---

## 📝 Full Changelog

See [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md#211) for the complete list of changes.

**License**: MIT
