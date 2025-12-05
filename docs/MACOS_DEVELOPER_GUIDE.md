# AnyWP Engine - macOS Developer Guide

## Overview

AnyWP Engine now supports macOS, providing desktop wallpaper functionality using WKWebView and AppKit. This guide covers macOS-specific features, implementation details, and best practices.

## System Requirements

- **macOS Version**: 10.14 (Mojave) or later
- **Xcode**: 12.0 or later
- **Flutter**: 3.0.0 or later
- **CocoaPods**: 1.11.0 or later

## Quick Start

### 1. Add Dependency

```yaml
# pubspec.yaml
dependencies:
  anywp_engine: ^2.2.0
```

### 2. Initialize Wallpaper

```dart
import 'package:anywp_engine/anywp_engine.dart';

// On macOS, same API as Windows!
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com/wallpaper',
);
```

### 3. Run on macOS

```bash
flutter run -d macos
```

## Architecture

### Window Layering

Unlike Windows (which uses Progman/WorkerW), macOS uses:

```objective-c
// Set window level to desktop
[window setLevel:CGWindowLevelForKey(kCGDesktopWindowLevelKey)];

// Make window stationary across all spaces
[window setCollectionBehavior:
    NSWindowCollectionBehaviorCanJoinAllSpaces |
    NSWindowCollectionBehaviorStationary |
    NSWindowCollectionBehaviorIgnoresCycle];
```

This places the window:
- Below desktop icons (desktop level)
- Visible on all Spaces/Mission Control views
- Does not appear in window switcher (Cmd+Tab)

### WebView Integration

Uses WKWebView (WebKit) instead of WebView2:

```objective-c
WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
config.preferences.javaScriptEnabled = YES;

WKWebView *webView = [[WKWebView alloc] initWithFrame:frame
                                        configuration:config];
```

### Message Bridge

Communication uses WKWebView's script message handlers:

**JavaScript to Native:**
```javascript
window.webkit.messageHandlers.anywpMessage.postMessage({
  type: 'myMessage',
  data: { ... }
});
```

**Native to JavaScript:**
```objective-c
[webView evaluateJavaScript:@"window.postMessage(...)" 
          completionHandler:nil];
```

## Platform-Specific Features

### 1. Multi-Monitor Support

macOS uses `NSScreen` API:

```dart
// Get all monitors
final monitors = await AnyWPEngine.getMonitors();

for (final monitor in monitors) {
  print('Monitor ${monitor.index}: ${monitor.width}x${monitor.height}');
}

// Initialize on specific monitor
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com',
  monitorIndex: 1,
);
```

### 2. Power Management

macOS integration with NSWorkspace notifications:

```dart
// Auto power saving (default: enabled)
await AnyWPEngine.setAutoPowerSaving(true);

// Listen for power state changes
AnyWPEngine.setOnPowerStateChangeCallback((oldState, newState) {
  print('Power state: $oldState -> $newState');
  // ACTIVE, IDLE, SCREEN_OFF, LOCKED, PAUSED
});
```

**Triggers:**
- Screen sleep/wake (`NSWorkspaceScreensDidSleepNotification`)
- Session lock/unlock (`NSWorkspaceSessionDidResignActiveNotification`)
- User idle (detected via `CGEventSourceSecondsSinceLastEventType`)

### 3. Memory Management

macOS typically uses more memory than Windows due to WKWebView:

```dart
// Configure memory threshold (default: 200MB on macOS)
await AnyWPEngine.setMemoryThreshold(200);

// Get current memory usage
final memoryMB = await AnyWPEngine.getMemoryUsage();
print('Memory: $memoryMB MB');

// Manual optimization
await AnyWPEngine.optimizeMemory();
```

### 4. State Persistence

Uses Application Support directory:

```dart
// Set application name for isolated storage
await AnyWPEngine.setApplicationName('MyWallpaperApp');

// Storage location: ~/Library/Application Support/AnyWPEngine/MyWallpaperApp/

// Save/load state
await AnyWPEngine.saveState('lastUrl', 'https://example.com');
final url = await AnyWPEngine.loadState('lastUrl');
```

## JavaScript SDK

The JavaScript SDK is platform-independent, but communication differs:

```javascript
// Detect platform
if (window.AnyWP) {
  console.log('Platform:', window.AnyWP.platform); // 'macOS'
  console.log('Version:', window.AnyWP.version);   // '2.2.0'
}

// Send message (same API on all platforms)
window.AnyWP.sendMessage({
  type: 'userAction',
  data: { action: 'click', x: 100, y: 200 }
});

// Check if running in AnyWP
if (window.AnyWP.utils.isAnyWP()) {
  // Wallpaper-specific code
}
```

## Permissions

### Accessibility (Optional)

For global mouse hooks (if implementing interactive mode in the future):

```xml
<!-- Info.plist -->
<key>NSAppleEventsUsageDescription</key>
<string>This app needs accessibility permissions for mouse interaction.</string>
```

Users must grant permission in:
**System Preferences > Security & Privacy > Privacy > Accessibility**

### Screen Recording (Optional)

For screen capture or window detection:

```xml
<!-- Info.plist -->
<key>NSScreenCaptureUsageDescription</key>
<string>This app needs screen capture for display detection.</string>
```

## Performance Optimization

### 1. WKWebView Memory

WKWebView can use more memory than WebView2. Best practices:

```dart
// Set conservative thresholds
await AnyWPEngine.setMemoryThreshold(200); // 200MB
await AnyWPEngine.setCleanupInterval(15);  // 15 minutes

// Enable auto power saving
await AnyWPEngine.setAutoPowerSaving(true);
```

### 2. Idle Detection

```dart
// Set idle timeout (default: 5 minutes)
await AnyWPEngine.setIdleTimeout(300);
```

### 3. Hardware Acceleration

WKWebView automatically uses GPU acceleration. No additional configuration needed.

## Debugging

### 1. Enable Web Inspector

In development, you can inspect WKWebView content:

```objective-c
// Development build only
#ifdef DEBUG
if (@available(macOS 13.3, *)) {
    [webView setInspectable:YES];
}
#endif
```

Then: **Right-click in wallpaper > Inspect Element**

### 2. Console Logs

```dart
// Dart logs
debugPrint('Wallpaper initialized');

// JavaScript logs (visible in Xcode console)
console.log('[Wallpaper] Ready');
```

### 3. Xcode Debugging

```bash
# Open Xcode workspace
open example/macos/Runner.xcworkspace

# Run with debugging
flutter run -d macos --debug
```

## Known Limitations

### 1. Window Level Behavior

- macOS may bring other windows above wallpaper on certain events
- Fullscreen apps will cover wallpaper (expected behavior)
- Dock and menu bar always visible

### 2. File Access

WKWebView has stricter file access restrictions:

```javascript
// Use file:// URLs with caution
// Prefer https:// or data: URIs
```

### 3. Mouse Transparency

Simple Mode (mouse transparent) is default:

```dart
// Desktop icons remain clickable
await AnyWPEngine.initializeWallpaper(url: url);
```

Interactive mode may require Accessibility permissions (not yet implemented).

## Platform Differences

| Feature | Windows | macOS |
|---------|---------|-------|
| WebView | WebView2 (Chromium) | WKWebView (WebKit) |
| Window Layer | Progman/WorkerW | CGWindowLevel |
| Memory (Typical) | 100-150MB | 150-200MB |
| File Access | Unrestricted | Sandboxed |
| Permissions | None required | Accessibility (optional) |
| Fullscreen Detection | GetForegroundWindow | NSApplication API |

## Migration Guide

### From Windows-Only

```dart
// No code changes needed!
// Same API works on both platforms

await AnyWPEngine.initializeWallpaper(url: 'https://...');
await AnyWPEngine.setAutoPowerSaving(true);
final monitors = await AnyWPEngine.getMonitors();
```

### Platform-Specific Code (if needed)

```dart
import 'dart:io';

if (Platform.isWindows) {
  // Windows-specific
} else if (Platform.isMacOS) {
  // macOS-specific
}
```

## Building for Release

### 1. Build App Bundle

```bash
flutter build macos --release
```

Output: `build/macos/Build/Products/Release/YourApp.app`

### 2. Code Signing

```bash
# Sign the app
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Your Name (TEAM_ID)" \
  build/macos/Build/Products/Release/YourApp.app
```

### 3. Notarization (for distribution)

```bash
# Create ZIP
ditto -c -k --keepParent YourApp.app YourApp.zip

# Upload for notarization
xcrun notarytool submit YourApp.zip \
  --apple-id "your@email.com" \
  --team-id "TEAM_ID" \
  --password "app-specific-password" \
  --wait

# Staple notarization ticket
xcrun stapler staple YourApp.app
```

## Examples

### Basic Wallpaper

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  await AnyWPEngine.setApplicationName('BasicWallpaper');
  await AnyWPEngine.initializeWallpaper(
    url: 'file:///path/to/wallpaper.html',
  );
  
  runApp(MyApp());
}
```

### Multi-Monitor Wallpaper

```dart
Future<void> initializeAllMonitors() async {
  final monitors = await AnyWPEngine.getMonitors();
  
  for (final monitor in monitors) {
    final url = 'file:///wallpapers/monitor${monitor.index}.html';
    await AnyWPEngine.initializeWallpaperOnMonitor(
      url: url,
      monitorIndex: monitor.index,
    );
  }
}
```

### Dynamic Wallpaper with Messages

```dart
// Setup message callback
AnyWPEngine.setOnMessageCallback((message) {
  print('From JavaScript: ${message['type']}');
  
  if (message['type'] == 'timeUpdate') {
    updateTimeDisplay(message['data']['time']);
  }
});

// Send message to wallpaper
await AnyWPEngine.sendMessage(
  message: {
    'type': 'setTheme',
    'data': {'theme': 'dark'}
  },
);
```

## Troubleshooting

### Wallpaper Not Showing

1. Check window level is set correctly
2. Verify URL is accessible
3. Check Console.app for errors

### High Memory Usage

1. Reduce memory threshold
2. Enable auto power saving
3. Optimize HTML/CSS (reduce DOM size)

### Permissions Issues

1. Check System Preferences > Security & Privacy
2. Grant required permissions
3. Restart application

## Support

- **GitHub Issues**: https://github.com/zhaibin/AnyWallpaper-Engine/issues
- **Documentation**: https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs
- **Examples**: https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/examples

---

**Version**: 2.2.0  
**Last Updated**: 2025-11-17

