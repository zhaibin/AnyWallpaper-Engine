# AnyWP Engine - Developer API Reference

Complete API reference for integrating AnyWP Engine into your Flutter application.

## Table of Contents

- [Architecture & Performance](#architecture--performance)
- [Basic Usage](#basic-usage)
- [Wallpaper Management](#wallpaper-management)
- [Multi-Monitor Support](#multi-monitor-support)
- [Power Saving & Optimization](#power-saving--optimization)
- [Configuration](#configuration)
- [Callbacks](#callbacks)
- [Data Types](#data-types)

---

## Architecture & Performance

### v2.0 Modular Architecture

AnyWP Engine v2.0 features a completely modular C++ architecture with significant improvements:

**Key Metrics:**

| Metric | v1.0 | v2.0 | Improvement |
|--------|------|------|-------------|
| Main file lines | 4,448 | 2,540 | **-42.9%** |
| Modularization rate | 0% | 78% | **+78%** |
| Test coverage | 0% | 98.5% | **+98.5%** |
| Total modules | 0 | 30 + 3 interfaces | **+33** |
| Debug build time | ~11s | ~5s | **-55%** |
| Mouse event lookup | O(n) | O(1) | **-87.5%** |
| Mouse latency | 10-15ms | <5ms | **-66%** |
| Event CPU usage | 5-8% | 3-5% | **-37.5%** |
| Log output freq | 100% | 10% | **-90%** |
| Code duplication | ~20% | <5% | **-75%** |
| Startup time | ~530ms | ~530ms | Maintained |
| Memory usage | ~230MB | ~230MB | Maintained |

### Benefits for Flutter Developers

**Backward Compatibility:**
- 鉁?**API unchanged** - No code modification needed
- 鉁?**Zero migration cost** - Just upgrade

**Performance Improvements:**
- 鈿?**Faster compilation** - 55% faster in Debug mode
- 馃殌 **Startup optimization** - Built-in startup optimizer
- 馃捑 **Memory optimization** - Smart memory monitoring & cleanup

**Stability Enhancements:**
- 馃洝锔?**Circuit breaker protection** - Prevents cascading failures
- 馃攧 **Automatic retry** - Self-recovery from transient failures
- 馃И **98.5% test coverage** - 209+ test cases

**Developer Experience:**
- 馃搳 **Performance monitoring** - Built-in CPU/memory profilers
- 馃摑 **Unified logging** - Easier debugging
- 馃敀 **Input validation** - Enhanced security

### Architecture Components

**Core Modules (13):**
- FlutterBridge - Flutter method channel communication
- DisplayChangeCoordinator - Monitor change detection
- InstanceManager - Wallpaper instance lifecycle
- WindowManager - Window creation & management
- InitializationCoordinator - Init flow coordination 鈿?
- WebViewManager - WebView2 lifecycle
- WebViewConfigurator - WebView2 security config 鈿?
- PowerManager - Power saving optimization
- IframeDetector - iframe detection
- SDKBridge - JavaScript SDK injection
- MouseHookManager - Mouse hook management
- MonitorManager - Multi-monitor support
- EventDispatcher - High-performance event routing (-87.5% lookup time) 鈿?

**Utility Classes (17):**
- StatePersistence - State persistence
- StartupOptimizer - Startup optimization
- CPUProfiler - CPU monitoring
- MemoryProfiler - Memory monitoring
- InputValidator - Input validation
- ConflictDetector - Conflict detection
- DesktopWallpaperHelper - Desktop wallpaper helper
- Logger - Unified logging (enhanced: buffering, rotation) 鈿?
- URLValidator - URL validation
- ResourceTracker - Resource tracking
- ErrorHandler - Unified error handling & recovery 鈿?
- PerformanceBenchmark - Performance measurement tool 鈿?
- PermissionManager - Fine-grained permission control 鈿?
- EventBus - Event-driven communication 鈿?
- ConfigManager - Centralized configuration 鈿?
- ServiceLocator - Dependency injection container 鈿?
- CircuitBreaker (header-only) - Circuit breaker pattern

**Interface Abstraction (3 interfaces) 鈿?**
- IWebViewManager - WebView2 management interface
- IStateStorage - State persistence interface
- ILogger - Logging interface

**Total: 30 modules + 3 interfaces | 78% modularization rate**

**Error Recovery:**
- CircuitBreaker - Circuit breaker pattern
- RetryHandler - Automatic retry logic
- SafetyMacros - 21 safety macros

**Technical Details:**
- [REFACTORING_OVERVIEW.md](REFACTORING_OVERVIEW.md) - Complete refactoring overview
- [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) - Implementation details

---

## Basic Usage

### Mouse Transparency (v2.0+)

**AnyWP Engine uses Simple Mode for all wallpapers:**

| Feature | Behavior | Use Cases |
|---------|----------|-----------|
| **Mouse Transparency** | Clicks pass through to desktop | All types of wallpapers |
| **Click Detection** | Via `AnyWP.onClick()` JavaScript API | Interactive elements in wallpapers |

**Key Features:**
- 鉁?Desktop icons remain clickable
- 鉁?Click detection through JavaScript SDK
- 鉁?Multi-monitor support
- 鉁?Settings persist across system suspend/resume

---

### Initialize Wallpaper (Single Monitor)

```dart
// Mouse transparent wallpaper - desktop icons remain clickable
bool success = await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com/wallpaper.html',
);
```

**Parameters:**
- `url` (required): URL to display (supports `http://`, `https://`, `file:///`)

**Returns:** `Future<bool>` - `true` if successful

**Note:** For click detection within your wallpaper, use the `AnyWP.onClick()` JavaScript API.

---

## Wallpaper Management

### Stop Wallpaper

```dart
// Stop wallpaper on primary monitor
bool success = await AnyWPEngine.stopWallpaper();
```

**Returns:** `Future<bool>` - `true` if successful

### Navigate to URL

```dart
// Change the displayed URL
bool success = await AnyWPEngine.navigateToUrl(
  'https://example.com/new-page.html'
);
```

**Parameters:**
- `url` (required): New URL to navigate to

**Returns:** `Future<bool>` - `true` if successful

---

## Multi-Monitor Support

### Get Monitors

```dart
// Get list of all connected monitors
List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();

for (var monitor in monitors) {
  print('Monitor ${monitor.index}: ${monitor.width}x${monitor.height}');
  print('  Device: ${monitor.deviceName}');
  print('  Position: (${monitor.left}, ${monitor.top})');
  print('  Primary: ${monitor.isPrimary}');
}
```

**Returns:** `Future<List<MonitorInfo>>` - List of connected monitors

### Initialize on Specific Monitor

```dart
// Start wallpaper on a specific monitor
bool success = await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com/wallpaper.html',
  monitorIndex: 1,
);
```

**Parameters:**
- `url` (required): URL to display
- `monitorIndex` (required): Monitor index (0-based)

**Returns:** `Future<bool>` - `true` if successful

### Stop on Specific Monitor

```dart
// Stop wallpaper on a specific monitor
bool success = await AnyWPEngine.stopWallpaperOnMonitor(1);
```

**Parameters:**
- `monitorIndex` (required): Monitor index to stop

**Returns:** `Future<bool>` - `true` if successful

### Navigate on Specific Monitor

```dart
// Navigate to new URL on specific monitor
bool success = await AnyWPEngine.navigateToUrlOnMonitor(
  'https://example.com/new-page.html',
  1
);
```

**Parameters:**
- `url` (required): New URL
- `monitorIndex` (required): Target monitor

**Returns:** `Future<bool>` - `true` if successful

### Batch Operations

```dart
// Start on all monitors
Map<int, bool> results = await AnyWPEngine.initializeWallpaperOnAllMonitors(
  url: 'https://example.com/wallpaper.html',
);

// Stop on all monitors
bool success = await AnyWPEngine.stopWallpaperOnAllMonitors();
```

### Click Detection

**v2.0+ Feature**: Use the JavaScript SDK to detect clicks within your wallpaper content.

All wallpapers run in **mouse transparent mode** (desktop icons remain clickable). For interactive elements within your wallpaper, use the `AnyWP.onClick()` JavaScript API:

```javascript
// In your wallpaper HTML
AnyWP.onClick('#myButton', (x, y) => {
  console.log('Button clicked at:', x, y);
  // Handle the click event
});
```

**Benefits:**
- 鉁?Desktop icons remain clickable
- 鉁?Selective click detection for specific elements
- 鉁?Works across all monitors independently
- 鉁?No runtime mode switching needed

For complete JavaScript SDK documentation, see [WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md)

---

## Power Saving & Optimization

### Manual Control

```dart
// Pause wallpaper (reduces CPU/GPU usage)
await AnyWPEngine.pauseWallpaper();

// Resume wallpaper
await AnyWPEngine.resumeWallpaper();
```

**Effects of Pause (Lightweight Strategy):**
- 鉁?WebView2 stops rendering (saves CPU/GPU)
- 鉁?Preserves DOM state and memory (instant resume)
- 鉁?Notifies web content via Page Visibility API
- 鉁?Auto-pauses videos and audio
- 鉁?Skips mouse hook processing
- 鉁?Light memory trim (no cache clearing)

**Resume Performance:**
- 鈿?**Instant recovery** (<50ms)
- 馃幆 No reloading or DOM reconstruction
- 馃帹 Animations continue from where they stopped
- 馃捑 All state preserved

### Auto Power Saving

```dart
// Enable/disable automatic power saving
await AnyWPEngine.setAutoPowerSaving(true);
```

**When Enabled, Auto Pauses When:**
- System is locked (Win+L)
- Screen turns off / system sleeps
- A fullscreen app is detected (games, videos)
- User is idle for configured duration (default: 5 minutes)

**Parameters:**
- `enabled` (required): `true` to enable, `false` to disable

**Returns:** `Future<bool>` - `true` if successful

### Get Power State

```dart
// Get current power state
String state = await AnyWPEngine.getPowerState();

switch (state) {
  case 'ACTIVE':
    print('Wallpaper is running normally');
    break;
  case 'IDLE':
    print('User is inactive');
    break;
  case 'LOCKED':
    print('System is locked');
    break;
  case 'FULLSCREEN_APP':
    print('Fullscreen app is running');
    break;
  case 'PAUSED':
    print('Manually paused');
    break;
}
```

**Returns:** `Future<String>` - One of: `ACTIVE`, `IDLE`, `SCREEN_OFF`, `LOCKED`, `FULLSCREEN_APP`, `PAUSED`

### Memory Management

```dart
// Get current memory usage in MB
int memoryMB = await AnyWPEngine.getMemoryUsage();
print('Memory usage: ${memoryMB}MB');

// Manually trigger memory optimization
await AnyWPEngine.optimizeMemory();
```

**Memory Optimization Performs:**
- Clears localStorage and sessionStorage
- Clears console logs and browser cache
- Triggers JavaScript garbage collection
- Multiple-pass memory trim (3 passes for better effect)

**Typical Results:**
- Frees 50-150MB on average
- Example: 200MB+ 鈫?~100MB

**Auto-Triggered When:**
- 鉁?After page load completes (3 seconds delay)
- 鉁?After URL navigation (3 seconds delay)  
- 鉁?When memory exceeds threshold (default: 150MB)
- 鉁?Every cleanup interval (default: 15 minutes)
- 鉁?When wallpaper is paused

**Note:** Optimization is automatic - manual calls rarely needed!

**Returns:** `Future<int>` or `Future<bool>` depending on method

---

## Configuration

### Set Idle Timeout

```dart
// Set idle timeout to 10 minutes
await AnyWPEngine.setIdleTimeout(600);

// Disable idle detection (set to very long time)
await AnyWPEngine.setIdleTimeout(3600 * 24); // 24 hours
```

**Parameters:**
- `seconds` (required): Timeout duration (minimum: 60, default: 300)

**Returns:** `Future<bool>` - `true` if successful

### Set Memory Threshold

```dart
// Trigger optimization when usage exceeds 200MB
await AnyWPEngine.setMemoryThreshold(200);
```

**Parameters:**
- `thresholdMB` (required): Memory threshold in MB (minimum: 100, **default: 150**)

**Returns:** `Future<bool>` - `true` if successful

### Set Cleanup Interval

```dart
// Check memory usage every 30 minutes
await AnyWPEngine.setCleanupInterval(30);
```

**Parameters:**
- `minutes` (required): Cleanup interval (minimum: 10, **default: 15**)

**Returns:** `Future<bool>` - `true` if successful

### Get Configuration

```dart
// Get current configuration
Map<String, dynamic> config = await AnyWPEngine.getConfiguration();

print('Idle timeout: ${config['idleTimeoutSeconds']}s');
print('Memory threshold: ${config['memoryThresholdMB']}MB');
print('Cleanup interval: ${config['cleanupIntervalMinutes']} min');
print('Auto power saving: ${config['autoPowerSavingEnabled']}');
```

**Returns:** `Future<Map<String, dynamic>>` - Configuration map

---

## State Persistence

### Save State

```dart
// Save custom state data
bool success = await AnyWPEngine.saveState(
  'wallpaper_url',
  'https://example.com/my-wallpaper.html'
);

// Save JSON object
import 'dart:convert';
Map<String, dynamic> settings = {'volume': 80, 'autoplay': true};
await AnyWPEngine.saveState('settings', jsonEncode(settings));
```

**Parameters:**
- `key` (required): Unique identifier for the state
- `value` (required): String value to save

**Returns:** `Future<bool>` - `true` if successful

### Load State

```dart
// Load saved state
String url = await AnyWPEngine.loadState('wallpaper_url');

// Load and parse JSON object
String settingsJson = await AnyWPEngine.loadState('settings');
if (settingsJson.isNotEmpty) {
  Map<String, dynamic> settings = jsonDecode(settingsJson);
  print('Volume: ${settings['volume']}');
}
```

**Parameters:**
- `key` (required): State identifier

**Returns:** `Future<String>` - Saved value, or empty string if not found

### Clear State

```dart
// Clear all saved state (cannot be undone)
bool success = await AnyWPEngine.clearState();
```

**Returns:** `Future<bool>` - `true` if successful

---

## Storage Isolation (v1.2.0+)

### Set Application Name

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // Set unique application identifier for isolated storage
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  runApp(MyApp());
}
```

**Parameters:**
- `name` (required): Application identifier (alphanumeric, spaces converted to underscores)

**Returns:** `Future<bool>` - `true` if successful

**Storage Path:** `%LOCALAPPDATA%\AnyWPEngine\[AppName]\state.json`

### Get Storage Path

```dart
String path = await AnyWPEngine.getStoragePath();
print('Data stored at: $path');
```

**Returns:** `Future<String>` - Full path to storage directory

馃挕 **See [README.md](../README.md#-storage-isolation-v120) for complete storage isolation guide, including uninstall cleanup and migration.**

### Get Plugin Version (v1.2.1+)

```dart
final version = await AnyWPEngine.getPluginVersion();
print('AnyWP Engine plugin version: $version');
```

**Returns:** `Future<String>` - Semantic version string锛堜緥濡?`1.2.1`锛?

### Check Compatibility (v1.2.1+)

```dart
final compatible = await AnyWPEngine.isCompatible(expectedPrefix: '1.2.');
if (!compatible) {
  final version = await AnyWPEngine.getPluginVersion();
  throw Exception('AnyWP Engine version mismatch: $version');
}
```

**Parameters:**
- `expectedPrefix` *(optional)*锛氶粯璁?`1.2.`锛屽尮閰嶅悓涓€涓?娆＄増鏈殑鎵€鏈夎ˉ涓?

**Returns:** `Future<bool>` - `true` 琛ㄧず鐗堟湰婊¤冻瑕佹眰

---

## Callbacks

### Monitor Change Callback

```dart
// Called when monitors are connected/disconnected
AnyWPEngine.setOnMonitorChangeCallback(() {
  print('Monitor configuration changed!');
  // Reload monitor list
  _loadMonitors();
});
```

### Power State Change Callback

```dart
// Called when power state changes
AnyWPEngine.setOnPowerStateChangeCallback((oldState, newState) {
  print('Power state: $oldState -> $newState');
  
  if (newState == 'LOCKED') {
    print('System locked - wallpaper auto-paused');
  } else if (newState == 'ACTIVE') {
    print('System active - wallpaper resumed');
  }
});
```

**Parameters:**
- `oldState`: Previous power state
- `newState`: New power state

---

## Data Types

### MonitorInfo

```dart
class MonitorInfo {
  final int index;           // Monitor index (0-based)
  final String deviceName;   // Device name (e.g., "\\.\DISPLAY1")
  final int left;            // X position in virtual desktop
  final int top;             // Y position in virtual desktop
  final int width;           // Width in pixels
  final int height;          // Height in pixels
  final bool isPrimary;      // Is this the primary monitor?
}
```

---

## Error Handling

All methods return `Future<bool>` or appropriate types. Check return values to handle errors:

```dart
try {
  bool success = await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com/wallpaper.html',
  );
  
  if (!success) {
    print('Failed to initialize wallpaper');
    // Handle error
  }
} catch (e) {
  print('Error: $e');
  // Handle exception
}
```

---

## Complete Example

```dart
import 'package:anywp_engine/anywp_engine.dart';

class WallpaperManager {
  Future<void> setup() async {
    // Setup callbacks
    AnyWPEngine.setOnMonitorChangeCallback(() {
      print('Monitors changed!');
    });
    
    AnyWPEngine.setOnPowerStateChangeCallback((old, new) {
      print('State: $old -> $new');
    });
    
    // Configure
    await AnyWPEngine.setIdleTimeout(600);      // 10 min
    await AnyWPEngine.setMemoryThreshold(200);  // 200MB
    await AnyWPEngine.setAutoPowerSaving(true);
    
    // Get monitors
    final monitors = await AnyWPEngine.getMonitors();
    print('Found ${monitors.length} monitors');
    
    // Start on all monitors
    await AnyWPEngine.initializeWallpaperOnAllMonitors(
      url: 'https://example.com/wallpaper.html',
    );
  }
  
  Future<void> checkStatus() async {
    final state = await AnyWPEngine.getPowerState();
    final memory = await AnyWPEngine.getMemoryUsage();
    final config = await AnyWPEngine.getConfiguration();
    
    print('State: $state');
    print('Memory: ${memory}MB');
    print('Config: $config');
  }
  
  Future<void> cleanup() async {
    await AnyWPEngine.stopWallpaperOnAllMonitors();
  }
}
```

---

## Bidirectional Communication (v2.1.0+)

### sendMessage

Send messages from Flutter to JavaScript wallpaper.

**Signature:**
```dart
static Future<bool> sendMessage({
  required Map<String, dynamic> message,
  int? monitorIndex,
}) async
```

**Parameters:**
- `message` (Map<String, dynamic>, required) - Message object to send
  - Recommended format: `{type: String, timestamp: int, data: Map}`
  - Maximum size: 10KB (recommended), 100KB (maximum)
- `monitorIndex` (int?, optional) - Target monitor index
  - If null or -1: broadcast to all monitors
  - If >= 0: send to specific monitor only

**Returns:**
- `Future<bool>` - true if sent successfully, false otherwise

**Example:**
```dart
// Send to all monitors
await AnyWPEngine.sendMessage(
  message: {
    'type': 'updateCarousel',
    'timestamp': DateTime.now().millisecondsSinceEpoch,
    'data': {
      'images': ['url1.jpg', 'url2.jpg'],
      'interval': 30000,
    },
  },
);

// Send to specific monitor
await AnyWPEngine.sendMessage(
  message: {
    'type': 'play',
    'timestamp': DateTime.now().millisecondsSinceEpoch,
    'data': {},
  },
  monitorIndex: 0,
);
```

**Message Protocol:**
See [MESSAGE_PROTOCOL.md](MESSAGE_PROTOCOL.md) for complete message format specification.

---

### setOnMessageCallback

Register callback to receive messages from JavaScript wallpaper.

**Signature:**
```dart
static void setOnMessageCallback(
  void Function(Map<String, dynamic> message) callback
)
```

**Parameters:**
- `callback` (Function, required) - Callback function
  - Receives parsed JSON message as `Map<String, dynamic>`
  - Called on UI thread (safe to use setState)
  - Message format: `{type: String, timestamp: int, data: Map}`

**Polling Mechanism:**
- Automatically polls for messages every 100ms
- Avoids InvokeMethod deadlock issue
- Low overhead (only processes when messages exist)
- Thread-safe message queue

**Example:**
```dart
// Setup callback
AnyWPEngine.setOnMessageCallback((message) {
  print('Received: ${message['type']}');
  
  switch (message['type']) {
    case 'carouselStateChanged':
      final data = message['data'] as Map<String, dynamic>;
      final currentIndex = data['currentIndex'] as int;
      print('Carousel index: $currentIndex');
      // Update UI
      setState(() {
        _currentIndex = currentIndex;
      });
      break;
      
    case 'error':
      final data = message['data'] as Map<String, dynamic>;
      final errorMsg = data['message'] as String;
      print('Error: $errorMsg');
      // Show error dialog
      showDialog(...);
      break;
      
    case 'ready':
      print('Wallpaper ready: ${message['name']}');
      break;
  }
});

// Later: Send message to JavaScript
await AnyWPEngine.sendMessage(
  message: {
    'type': 'ping',
    'timestamp': DateTime.now().millisecondsSinceEpoch,
    'data': {'requestId': 'req-001'},
  },
);
```

**Common Message Types:**
- `ready` - Wallpaper initialization complete
- `carouselStateChanged` - Carousel state update
- `error` - Error occurred in JavaScript
- `heartbeat` - Keep-alive ping
- Custom types - Any string you define

**Error Handling:**
```dart
AnyWPEngine.setOnMessageCallback((message) {
  try {
    // Process message
    final type = message['type'] as String;
    final data = message['data'] as Map<String, dynamic>;
    
    // Handle message...
  } catch (e) {
    print('Error processing message: $e');
  }
});
```

**Performance:**
- Message latency: < 10ms (one-way)
- Throughput: > 1000 msg/s
- Queue capacity: 1000 messages
- Polling interval: 100ms (fixed)

**See Also:**
- [MESSAGE_PROTOCOL.md](MESSAGE_PROTOCOL.md) - Complete message protocol specification
- [WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md) - JavaScript SDK API
- `examples/test_bidirectional.html` - Complete test example

---

## Best Practices

1. **Always check return values** for error handling
2. **Use callbacks** to respond to system events
3. **Configure before initializing** for better control
4. **Enable auto power saving** for better battery life
5. **Monitor memory usage** in long-running applications
6. **Handle monitor changes** gracefully
7. **Test on multiple monitors** if supporting multi-monitor setups
8. **Use message protocol** for Flutter ↔ JavaScript communication (v2.1.0+)
9. **Handle message errors** gracefully with try-catch
10. **Keep messages small** (< 10KB recommended)

---

## See Also

- [Quick Start Guide](QUICK_START.md)
- [API Usage Examples](API_USAGE_EXAMPLES.md)
- [Web Developer Guide](WEB_DEVELOPER_GUIDE_CN.md)
- [Troubleshooting](TROUBLESHOOTING.md)


