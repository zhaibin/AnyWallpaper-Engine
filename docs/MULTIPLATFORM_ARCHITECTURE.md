# AnyWP Engine - Multi-Platform Architecture Design

## Overview

This document describes the architecture for supporting multiple platforms (Windows, macOS) in AnyWP Engine.

## Architecture Layers

```
┌─────────────────────────────────────────┐
│         Dart API Layer (lib/)           │
│  - Unified interface for all platforms  │
│  - Platform detection & routing         │
└────────────────┬────────────────────────┘
                 │
    ┌────────────┴──────────────┐
    │                           │
┌───▼──────────────┐  ┌────────▼─────────┐
│  Windows Plugin  │  │   macOS Plugin   │
│  (windows/)      │  │   (macos/)       │
│                  │  │                  │
│  - WebView2      │  │  - WKWebView     │
│  - Win32 API     │  │  - AppKit        │
│  - C++17         │  │  - Objective-C   │
└──────────────────┘  └──────────────────┘
```

## Platform-Specific Components

### Windows (Existing)
- **WebView Engine**: WebView2 (Chromium-based)
- **Window System**: Win32 API
- **Graphics**: DirectX/GDI
- **Storage**: Windows Registry + LocalAppData
- **Language**: C++17
- **Build System**: CMake

### macOS (New)
- **WebView Engine**: WKWebView (WebKit-based)
- **Window System**: AppKit (NSWindow, NSView)
- **Graphics**: Core Graphics/Quartz
- **Storage**: UserDefaults + Application Support
- **Language**: Objective-C / Swift
- **Build System**: CMake / CocoaPods

## Shared Components

### 1. Dart API Layer
- Unified method channel interface
- Platform-agnostic data models (MonitorInfo, etc.)
- Automatic platform detection

### 2. JavaScript SDK
- **Fully reusable across platforms**
- Browser-based APIs (no platform-specific code)
- TypeScript SDK (`sdk/src/` - platform-independent)
- Compiled output (`sdk/dist/anywp_sdk.js` - single bundle for all platforms)

### 3. Design Patterns
- Modular architecture
- Error handling strategies
- Logging framework patterns
- State persistence patterns

## Feature Mapping

| Feature                    | Windows                  | macOS                        |
|---------------------------|--------------------------|------------------------------|
| WebView                   | WebView2                 | WKWebView                    |
| Desktop Wallpaper Layer   | Progman/WorkerW          | NSWindow (Level Below Icons) |
| Multi-Monitor Support     | EnumDisplayMonitors      | NSScreen API                 |
| Mouse Transparency        | WS_EX_TRANSPARENT        | NSWindow.ignoresMouseEvents  |
| Mouse Hook (Interactive)  | SetWindowsHookEx         | NSEvent.addGlobalMonitor     |
| Power Management          | WM_POWERBROADCAST        | NSWorkspace notifications    |
| Display Change Detection  | WM_DISPLAYCHANGE         | NSScreen notifications       |
| Storage                   | Registry + JSON files    | UserDefaults + JSON files    |
| Process Memory            | PROCESS_MEMORY_COUNTERS  | task_info                    |
| Fullscreen Detection      | GetForegroundWindow      | NSApplication API            |

## Implementation Strategy

### Phase 1: Platform Abstraction ✅
1. Update `pubspec.yaml` to declare both platforms
2. Refactor Dart API for platform detection
3. Extract platform-agnostic interfaces

### Phase 2: macOS Platform Setup ✅
1. Create `macos/` directory structure
2. Setup CMakeLists.txt for macOS
3. Create plugin registration files

### Phase 3: macOS Core Implementation 🚧
1. Wallpaper initialization (NSWindow below desktop)
2. WKWebView integration
3. Multi-monitor support (NSScreen)
4. Mouse transparency and interaction

### Phase 4: macOS Feature Parity 📋
1. Power management (NSWorkspace)
2. Display change detection
3. State persistence (UserDefaults)
4. Memory optimization

### Phase 5: Testing & Documentation 📋
1. Update example app for macOS
2. Platform-specific documentation
3. Build and release scripts

## Code Organization

```
anywp_engine/
├── lib/
│   └── anywp_engine.dart           # Platform-agnostic Dart API
├── sdk/                            # Cross-platform TypeScript SDK (NEW ⭐)
│   ├── src/                        # TypeScript source code
│   │   ├── core/                   # Core SDK logic
│   │   ├── modules/                # Feature modules
│   │   ├── utils/                  # Utilities (including platform.ts)
│   │   └── package.json
│   ├── dist/                       # Compiled JavaScript SDK
│   │   └── anywp_sdk.js            # Final bundle (used by all platforms)
│   └── README.md                   # SDK documentation
├── windows/                         # Windows implementation
│   ├── modules/                     # Windows-specific modules
│   ├── utils/                       # Windows utilities
│   └── anywp_engine_plugin.cpp     # Windows plugin entry
├── macos/                           # macOS implementation (NEW)
│   ├── Classes/
│   │   ├── AnyWPEnginePlugin.h     # macOS plugin header
│   │   ├── AnyWPEnginePlugin.m     # macOS plugin implementation
│   │   ├── Modules/                # macOS-specific modules
│   │   │   ├── WallpaperManager.h/m
│   │   │   ├── MonitorManager.h/m
│   │   │   ├── PowerManager.h/m
│   │   │   └── MessageBridge.h/m
│   │   └── Utils/                  # macOS utilities
│   │       ├── Logger.h/m
│   │       ├── StatePersistence.h/m
│   │       └── ...
│   ├── Assets/                     # macOS assets
│   └── CMakeLists.txt              # macOS build config
└── docs/
    ├── MACOS_DEVELOPER_GUIDE.md    # macOS-specific guide
    └── MULTIPLATFORM_ARCHITECTURE.md
```

## API Consistency

All APIs maintain the same interface across platforms:

```dart
// Works on both Windows and macOS
await AnyWPEngine.initializeWallpaper(url: 'https://example.com');
await AnyWPEngine.setAutoPowerSaving(true);
final monitors = await AnyWPEngine.getMonitors();
```

Platform-specific behavior is handled internally:
- Windows: Uses Progman/WorkerW trick
- macOS: Creates NSWindow at desktop level

## Platform Detection

```dart
// Automatic detection in Dart API
static const _platform = MethodChannel('anywp_engine');

Future<bool> initializeWallpaper({required String url}) async {
  if (Platform.isWindows) {
    // Routed to Windows plugin
  } else if (Platform.isMacOS) {
    // Routed to macOS plugin
  } else {
    throw UnsupportedError('Platform not supported');
  }
  
  return await _channel.invokeMethod('initializeWallpaper', {'url': url});
}
```

## Testing Strategy

### Unit Tests
- Platform-independent Dart API tests
- Windows-specific module tests (existing)
- macOS-specific module tests (new)

### Integration Tests
- Cross-platform example app
- Feature parity validation
- Performance benchmarks

### Manual Testing
- Windows 10/11 (existing)
- macOS 12+ (Monterey and later)
- Multi-monitor setups on both platforms

## Known Platform Differences

### 1. Window Layering
- **Windows**: Uses Progman/WorkerW trick to place behind desktop icons
- **macOS**: Uses NSWindow with appropriate level (CGWindowLevelForKey)

### 2. Mouse Events
- **Windows**: Low-level mouse hook (SetWindowsHookEx)
- **macOS**: Global event monitor (NSEvent.addGlobalMonitorForEventsMatchingMask)

### 3. Security Permissions
- **Windows**: No special permissions needed
- **macOS**: May require Accessibility permissions for global mouse hooks

### 4. Fullscreen Detection
- **Windows**: GetForegroundWindow + extended frame bounds
- **macOS**: NSApplication.currentSystemPresentationOptions

## Performance Considerations

### Memory Usage
- **Windows**: Target < 150MB per instance
- **macOS**: Target < 200MB per instance (WKWebView typically uses more)

### CPU Usage
- Both platforms: Auto-pause when idle/locked
- Both platforms: Configurable cleanup intervals

## Future Extensions

- Linux support (X11/Wayland)
- Mobile platforms (iOS/Android) as live wallpapers
- Web platform (browser extension)

## References

- [Flutter Platform Channels](https://docs.flutter.dev/platform-integration/platform-channels)
- [WKWebView Documentation](https://developer.apple.com/documentation/webkit/wkwebview)
- [AppKit Window Management](https://developer.apple.com/documentation/appkit/nswindow)
- [macOS Event Handling](https://developer.apple.com/documentation/appkit/nsevent)

---

**Version**: 2.2.0 (Multi-platform)  
**Last Updated**: 2025-11-17

