# AnyWP Engine v2.6.2 - Security Patch Release 🔒

## 🚨 Security Update

This is a **critical security patch** that fixes **3 severe issues** in the macOS implementation. All macOS users are strongly recommended to update immediately.

---

## 🔒 Security Fixes (HIGH Priority)

### 1. Race Condition in Static Variable Access

**Severity**: HIGH  
**Impact**: Data race and undefined behavior from multi-threaded access

**Problem**: 
- Static variables `_rootDirectory` and `_isRunning` were accessed from multiple threads without synchronization
- Could lead to crashes, data corruption, or unpredictable behavior

**Fix**:
- Added `dispatch_queue_t` serial queue to protect all static variable access
- Used `dispatch_once` for initialization and `dispatch_sync` for all read/write operations
- Thread-safe access guaranteed across all methods

```objc
static dispatch_queue_t _syncQueue = nil;

+ (void)initialize {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _syncQueue = dispatch_queue_create("com.anywp.localfileserver.sync", DISPATCH_QUEUE_SERIAL);
    });
}

dispatch_sync(_syncQueue, ^{
    _isRunning = YES;
});
```

---

### 2. Path Traversal Security Vulnerability ⚠️ CRITICAL

**Severity**: CRITICAL  
**Impact**: Attackers could access arbitrary files on the system

**Problem**:
- URLs starting with `/` were treated as absolute paths
- Attackers could bypass `rootDirectory` restrictions
- Could be exploited to read sensitive system files

**Fix**:
- Force all paths to be treated as relative paths
- Strip leading `/` from all file paths
- Canonicalize and validate paths are within `rootDirectory`
- Return 403 Forbidden (not 404) for path traversal attempts

```objc
// Strip leading slash
if ([filePath hasPrefix:@"/"]) {
    filePath = [filePath substringFromIndex:1];
}

// Validate path is within root
NSString *canonicalRoot = [rootDir stringByStandardizingPath];
NSString *canonicalPath = [fullPath stringByStandardizingPath];

if (![canonicalPath hasPrefix:canonicalRoot]) {
    [self sendError:403 message:@"Access denied"];
    return;
}
```

**Example Attack** (Now Blocked):
```
localfile:///../../../etc/passwd  ❌ Blocked
localfile:///../../.ssh/id_rsa     ❌ Blocked
```

---

### 3. RangeError Crash in Path Extraction

**Severity**: HIGH  
**Impact**: Application crash when running outside standard app bundle

**Problem**:
- `substring(0, lastIndexOf('/Contents/MacOS'))` would crash if `/Contents/MacOS` not found
- `lastIndexOf` returns -1 when substring not found
- `substring(0, -1)` throws RangeError

**Fix**:
- Check `lastIndexOf` result before calling `substring`
- Added fallback logic for development/non-standard paths
- Enhanced error handling and logging

```dart
final macOSMarker = '/Contents/MacOS';
final markerIndex = executablePath.lastIndexOf(macOSMarker);

if (markerIndex != -1) {
  // Running from app bundle
  final appPath = executablePath.substring(0, markerIndex);
  projectRoot = '$appPath/Contents/Resources';
} else {
  // Development mode - use fallback logic
  final execDir = Directory(executablePath).parent.path;
  projectRoot = execDir;
}
```

---

## 📝 Code Quality Improvements

### Flutter Linter Warnings Fixed

- Fixed 3 linter warnings in `lib/anywp_engine.dart`:
  - 2 × `prefer_const_constructors` (lines 362, 397)
  - 1 × `prefer_final_locals` (line 367)
- Analysis result: 90 issues → 87 issues (only `avoid_print` debug logs remain)

---

## 📊 Impact Assessment

### Security Impact

| Issue | Before | After |
|-------|--------|-------|
| **Path Traversal** | 🔴 System files exposed | ✅ Access restricted |
| **Race Condition** | 🔴 Data races possible | ✅ Thread-safe |
| **Crash Risk** | 🔴 RangeError in edge cases | ✅ Robust handling |

### Affected Versions

- **v2.6.0**: ✅ Not affected (LocalFileServer not present)
- **v2.6.1**: ❌ Affected (all 3 issues present)
- **v2.6.2**: ✅ All issues fixed

---

## 🧪 Testing

All fixes have been validated:

- ✅ macOS Debug build successful
- ✅ macOS Release build successful
- ✅ No compiler warnings
- ✅ Path security validation passed
- ✅ Multi-threaded access tested
- ✅ Edge case path handling verified

---

## 📦 Download Packages

Three packages are available:

### 1. **Precompiled Package** (Recommended) ⭐
- `anywp_engine_macos_v2.6.2_precompiled.zip` (1.5MB)
- SDK embedded in framework
- Easiest integration for Flutter developers

### 2. **Source Package** (For Advanced Users)
- `anywp_engine_macos_v2.6.2_source.zip` (24MB)
- Complete Objective-C source code
- For developers who need custom modifications

### 3. **Web SDK Package** (For Wallpaper Developers)
- `anywp_web_sdk_v2.5.0.zip` (116KB)
- Cross-platform JavaScript SDK
- Works on both Windows and macOS

---

## 🚀 Upgrade Instructions

### For Existing Users (v2.6.1)

**⚠️ URGENT**: Please upgrade immediately to fix the security vulnerability.

```yaml
# pubspec.yaml
dependencies:
  anywp_engine:
    path: ./plugins/anywp_engine  # Update to v2.6.2
```

Then rebuild your application:
```bash
flutter clean
flutter pub get
flutter build macos --release
```

### For New Users

See [macOS Integration Guide](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PRECOMPILED_MACOS_INTEGRATION.md)

---

## 📚 Documentation

- [macOS Integration Guide](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PRECOMPILED_MACOS_INTEGRATION.md)
- [API Reference](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/DEVELOPER_API_REFERENCE.md)
- [Platform Comparison](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PLATFORM_COMPARISON.md)
- [Web Developer Guide](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/WEB_DEVELOPER_GUIDE.md)

---

## 🐛 Bug Reports

If you discover any security issues, please report them privately to the maintainers.

For general bugs: [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)

---

## 📈 What's Changed

**Full Changelog**: [v2.6.1...v2.6.2](https://github.com/zhaibin/AnyWallpaper-Engine/compare/v2.6.1...v2.6.2)

---

## 🙏 Credits

Thanks to all contributors and security researchers who help keep this project secure!

---

**Platform Support**:
- ✅ Windows 10/11 (x64)
- ✅ macOS 10.13+ (Intel & Apple Silicon)

**Flutter Version**: 3.0.0+

---

**License**: MIT  
**Repository**: https://github.com/zhaibin/AnyWallpaper-Engine

