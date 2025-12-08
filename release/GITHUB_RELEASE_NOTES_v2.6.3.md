# AnyWP Engine v2.6.3 - Multi-Monitor Fix Release 🖥️

## What's New

### 🎯 Critical Fix: macOS Multi-Monitor Wallpaper Rendering

**Root Cause Found**: WebView was using screen coordinates instead of window-relative coordinates.

| Monitor | Before (Bug) | After (Fixed) |
|---------|-------------|---------------|
| Primary (0,0) | frame = (0, 0, 2560, 1440) ✅ | frame = (0, 0, 2560, 1440) ✅ |
| Secondary (2560,0) | frame = (2560, 0, 2560, 1440) ❌ | frame = (0, 0, 2560, 1440) ✅ |

The WebView on secondary monitors was positioned **completely outside** the visible window area!

### 🔧 Additional Fixes

- **HiDPI Window Positioning**: Fixed window position doubling issue on HiDPI displays
- **URL Port Synchronization**: Fixed missing port number in secondary monitor URLs

## Downloads

| Package | Description | For |
|---------|-------------|-----|
| `anywp_engine_macos_v2.6.3_precompiled.zip` | **Recommended** - Ready-to-use macOS framework | Flutter Developers |
| `anywp_engine_macos_v2.6.3_source.zip` | Full source code for customization | Advanced Developers |
| `anywp_web_sdk_v2.5.0.zip` | JavaScript SDK for wallpaper creation | Web Developers |

## Technical Details

### WallpaperManager.m Changes

```objective-c
// Before (Bug)
WKWebView *webView = [[WKWebView alloc] initWithFrame:screenFrame ...];
// screenFrame.origin = (2560, 0) for secondary monitor → WebView outside window!

// After (Fix)
NSRect webViewFrame = NSMakeRect(0, 0, screenFrame.size.width, screenFrame.size.height);
WKWebView *webView = [[WKWebView alloc] initWithFrame:webViewFrame ...];
// webViewFrame.origin = (0, 0) → WebView correctly inside window
```

### Window Level Configuration

- Default: `kCGDesktopIconWindowLevel - 1` (below desktop icons)
- Interactive Mode: `kCGDesktopIconWindowLevel + 1` (above desktop icons)

## Verification

- ✅ Primary monitor: Wallpaper visible, icons visible
- ✅ Secondary monitor: Wallpaper visible, icons visible
- ✅ WebView content renders correctly on all monitors
- ✅ Interactive Mode toggle works correctly
- ✅ HiDPI display compatibility verified

## Debugging Journey

After 6 rounds of window level adjustments, we discovered the real culprit was the WebView frame coordinate system:

| Attempt | Level | Result | Real Cause |
|---------|-------|--------|------------|
| 1 | iconLevel - 1 | Black screen | WebView frame bug |
| 2 | desktopLevel | Black screen | WebView frame bug |
| 3 | iconLevel - 10 | Black screen | WebView frame bug |
| 4 | normalLevel - 1 | Covered icons | WebView frame bug |
| 5 | iconLevel + 1 | Visible but covered icons | Frame fixed but level too high |
| 6 | iconLevel - 1 | ✅ Perfect | Frame fixed + correct level |

## Upgrade Notes

This is a **recommended update** for all macOS users experiencing black screen issues on secondary monitors.

### Breaking Changes

None - fully backward compatible.

---

**Full Changelog**: [v2.6.2...v2.6.3](https://github.com/AnyWallpaper/anywp-engine/compare/v2.6.2...v2.6.3)

