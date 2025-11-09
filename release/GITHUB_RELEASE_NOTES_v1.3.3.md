# AnyWP Engine v1.3.3 - SDK Injection Fix

## 🐛 Critical Bug Fixes

This release addresses critical issues with SDK injection and mouse event handling that affected v1.3.2.

---

## 🔧 Core Fixes

### 1. SDK Duplicate Injection Fix

**Problem:**
- SDK was being injected twice (C++ plugin auto-injection + manual HTML loading)
- Event handlers registered twice, causing callbacks to fire multiple times
- Click counters incremented/decremented by 2 instead of 1

**Solution:**
- Added global flag `_anywpEarlyMessageListenerRegistered` to prevent duplicate WebMessage listener registration
- Removed manual SDK loading scripts from all 13 test HTML pages
- SDK now relies solely on C++ plugin's automatic injection via `AddScriptToExecuteOnDocumentCreated`

**Code Changes:**
```typescript
// windows/sdk/index.ts
if (globalAny._anywpEarlyMessageListenerRegistered) {
  console.log('[AnyWP] WebMessage listener already registered (EARLY), skipping duplicate');
} else {
  globalAny._anywpEarlyMessageListenerRegistered = true;
  // Register WebMessage listener only once
}
```

### 2. Mouse Event Detection Improvements

**Problem:**
- After clicking within WebView, mouseover events stopped firing
- v1.3.2 had temporarily disabled window detection for mouseover as a workaround

**Solution:**
- Restored full window occlusion detection for all mouse events (mousemove, mousedown, mouseup)
- Added `IsOurWindow()` helper function to correctly identify wallpaper windows
- Fixed logic to prevent wallpaper's own windows from being treated as "occluding application windows"

**Code Changes:**
```cpp
// windows/anywp_engine_plugin.cpp
bool AnyWPEnginePlugin::IsOurWindow(HWND hwnd) {
  // Check if window belongs to wallpaper's WebView or WorkerW
  // Includes child windows and all instances
}
```

---

## 📝 Changed Files

### TypeScript SDK
- `windows/sdk/index.ts` - Added duplicate registration guard
- `windows/sdk/core/init.ts` - Moved WebMessage listener to index.ts

### C++ Plugin
- `windows/anywp_engine_plugin.cpp` - Restored window detection + added `IsOurWindow()`
- `windows/anywp_engine_plugin.h` - Added `IsOurWindow()` declaration

### Example HTML Pages (removed manual SDK loading)
- `examples/test_api.html`
- `examples/test_basic_click.html`
- `examples/test_drag_debug.html`
- `examples/test_draggable.html`
- `examples/test_vue.html`
- `examples/test_visibility.html`
- `examples/test_simple.html`
- `examples/test_sdk_browser.html`
- `examples/test_react.html`
- `examples/test_iframe_ads.html`
- `examples/test_refactoring.html`
- `examples/test_position_tracking.html`
- `examples/test_js_events_debug.html`
- `examples/test_webmessage_debug.html`

### Configuration
- `pubspec.yaml` - Version 1.3.3
- `.cursorrules` - Updated version info
- `example/pubspec.yaml` - Removed deleted asset directory reference

---

## 🎯 Correct Usage

### HTML Pages (No Manual SDK Loading Required)

```html
<!DOCTYPE html>
<html>
<head>
  <title>AnyWP Test Page</title>
  <!-- ❌ DO NOT manually load SDK -->
  <!-- <script src="../windows/anywp_sdk.js"></script> -->
</head>
<body>
  <div id="clickable">Click me</div>
  
  <script>
    // ✅ SDK is automatically injected by C++ plugin
    if (window.AnyWP) {
      AnyWP.onClick('#clickable', function() {
        console.log('Clicked!'); // Now fires once per click
      });
    }
  </script>
</body>
</html>
```

### SDK Injection Path

The SDK is injected by the C++ plugin at:
```cpp
webview_controller->AddScriptToExecuteOnDocumentCreated(
    L"windows/anywp_sdk.js",  // ← Injection path
    callback.Get()
);
```

**Full path:** `E:\Projects\AnyWallpaper\AnyWallpaper-Engine\windows\anywp_sdk.js`

---

## 🧪 Testing Results

### Functional Tests
- ✅ Click events fire exactly once (+1/-1, not +2/-2)
- ✅ Mouseover events continue working after clicking in WebView
- ✅ Drag and drop functionality works correctly
- ✅ State persistence works as expected
- ✅ All 14 test pages function properly

### Unit Tests
- ✅ TypeScript SDK: 118/118 tests passing (100%)
- ✅ C++ compilation: No errors, no warnings
- ✅ Flutter build: Debug and Release builds successful

---

## 📦 Downloads

### For Flutter Developers
Download `anywp_engine_v1.3.3.zip` (37.94 MB)
- Includes DLL, LIB, C++ source, Dart source, SDK, documentation
- Complete package for Flutter Windows integration

### For Web Developers
Download `anywp_web_sdk_v1.3.3.zip` (80.72 KB)
- Lightweight package with just the JavaScript SDK
- Includes examples and documentation
- Perfect for testing wallpaper HTML pages

---

## 🔄 Upgrade Guide

### From v1.3.2 to v1.3.3

1. **Update Plugin Package**
   - Replace `windows/` directory with new version
   - Copy updated `lib/anywp_engine.dart`

2. **Clean HTML Pages**
   Remove any manual SDK loading code:
   ```html
   <!-- DELETE THIS -->
   <script>
     if (!window.AnyWP) {
       document.write('<script src="../windows/anywp_sdk.js"><\/script>');
     }
   </script>
   ```

3. **Rebuild**
   ```bash
   # Rebuild TypeScript SDK (if modifying SDK source)
   cd windows\sdk
   npm run build
   
   # Rebuild Flutter app
   cd ..\..
   flutter clean
   flutter build windows --debug
   ```

4. **Test**
   - Verify click events fire once
   - Check mouseover events work after clicking
   - Test draggable elements
   - Verify state persistence

---

## ⚠️ Breaking Changes

**None.** This release is fully backward compatible with v1.3.2.

However, if you have HTML pages with manual SDK loading, you should remove those scripts to prevent potential duplicate injection in future versions.

---

## 📚 Documentation

- **Updated:** `.cursorrules` with v1.3.3 notes
- **Updated:** `CHANGELOG_CN.md` with detailed fix descriptions
- **Added:** SDK injection path documentation

---

## 🙏 Acknowledgments

Thank you to all testers who reported the duplicate event firing issue!

---

## 🔗 Links

- **GitHub Repository:** https://github.com/zhaibin/AnyWallpaper-Engine
- **Previous Release:** [v1.3.2](https://github.com/zhaibin/AnyWallpaper-Engine/releases/tag/v1.3.2)
- **Documentation:** See `docs/` directory in release package

---

## 📄 License

MIT License - See LICENSE file for details.

