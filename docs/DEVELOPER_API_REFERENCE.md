# AnyWP Engine - Developer API Reference

Complete API reference for integrating AnyWP Engine into your Flutter application.

## Table of Contents

- [Basic Usage](#basic-usage)
- [Wallpaper Management](#wallpaper-management)
- [Multi-Monitor Support](#multi-monitor-support)
- [Power Saving & Optimization](#power-saving--optimization)
- [Configuration](#configuration)
- [Callbacks](#callbacks)
- [Data Types](#data-types)

---

## Basic Usage

### Initialize Wallpaper

```dart
// Initialize wallpaper on primary monitor
bool success = await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com/wallpaper.html',
  enableMouseTransparent: true,  // Allow clicks to pass through
);
```

**Parameters:**
- `url` (required): URL to display (supports `http://`, `https://`, `file:///`)
- `enableMouseTransparent` (optional, default: `true`): Enable click-through behavior

**Returns:** `Future<bool>` - `true` if successful

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
  enableMouseTransparent: true,
);
```

**Parameters:**
- `url` (required): URL to display
- `monitorIndex` (required): Monitor index (0-based)
- `enableMouseTransparent` (optional, default: `true`): Enable click-through

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
  enableMouseTransparent: true,
);

// Stop on all monitors
bool success = await AnyWPEngine.stopWallpaperOnAllMonitors();
```

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
- ✅ WebView2 stops rendering (saves CPU/GPU)
- ✅ Preserves DOM state and memory (instant resume)
- ✅ Notifies web content via Page Visibility API
- ✅ Auto-pauses videos and audio
- ✅ Skips mouse hook processing
- ✅ Light memory trim (no cache clearing)

**Resume Performance:**
- ⚡ **Instant recovery** (<50ms)
- 🎯 No reloading or DOM reconstruction
- 🎨 Animations continue from where they stopped
- 💾 All state preserved

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
- Example: 200MB+ → ~100MB

**Auto-Triggered When:**
- ✅ After page load completes (3 seconds delay)
- ✅ After URL navigation (3 seconds delay)  
- ✅ When memory exceeds threshold (default: 150MB)
- ✅ Every cleanup interval (default: 15 minutes)
- ✅ When wallpaper is paused

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

💡 **See [README.md](../README.md#-storage-isolation-v120) for complete storage isolation guide, including uninstall cleanup and migration.**

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
      enableMouseTransparent: true,
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

## Best Practices

1. **Always check return values** for error handling
2. **Use callbacks** to respond to system events
3. **Configure before initializing** for better control
4. **Enable auto power saving** for better battery life
5. **Monitor memory usage** in long-running applications
6. **Handle monitor changes** gracefully
7. **Test on multiple monitors** if supporting multi-monitor setups

---

## See Also

- [Quick Start Guide](QUICK_START.md)
- [Usage Examples](USAGE_EXAMPLES.md)
- [Web Developer Guide](WEB_DEVELOPER_GUIDE.md)
- [Troubleshooting](TROUBLESHOOTING.md)

