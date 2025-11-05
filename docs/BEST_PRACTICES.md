# AnyWP Engine - Best Practices Guide

Guidelines and recommendations for building optimal wallpaper applications.

## Table of Contents

1. [Performance Optimization](#performance-optimization)
2. [Memory Management](#memory-management)
3. [Power Efficiency](#power-efficiency)
4. [Multi-Monitor Support](#multi-monitor-support)
5. [Error Handling](#error-handling)
6. [User Experience](#user-experience)
7. [Security](#security)

---

## Performance Optimization

### ✅ DO: Use Efficient Web Content

**Recommended:**
```html
<!-- Optimize images -->
<img src="background.webp" loading="lazy" />

<!-- Use CSS animations instead of JavaScript -->
<div class="animated-background"></div>

<style>
.animated-background {
  animation: slide 20s linear infinite;
}
</style>
```

**Avoid:**
```javascript
// Heavy JavaScript animations
setInterval(() => {
  // Expensive operations every frame
  updateComplexVisualization();
}, 16); // 60 FPS
```

### ✅ DO: Minimize DOM Complexity

Keep DOM tree shallow and simple:

```html
<!-- Good: Simple structure -->
<div id="wallpaper">
  <canvas id="background"></canvas>
  <div id="widgets"></div>
</div>

<!-- Avoid: Deep nesting -->
<div>
  <div>
    <div>
      <div>
        <!-- Many nested elements... -->
      </div>
    </div>
  </div>
</div>
```

### ✅ DO: Use `requestAnimationFrame` for Animations

```javascript
// Good: Efficient animation
function animate() {
  // Update animation
  updateElements();
  requestAnimationFrame(animate);
}
requestAnimationFrame(animate);

// Avoid: setInterval/setTimeout
setInterval(updateElements, 16);
```

### ✅ DO: Debounce Frequent Updates

```javascript
let resizeTimer;
window.addEventListener('resize', () => {
  clearTimeout(resizeTimer);
  resizeTimer = setTimeout(() => {
    // Handle resize
    adjustLayout();
  }, 250);
});
```

---

## Memory Management

### ✅ DO: Clean Up Event Listeners

```dart
class WallpaperWidget extends StatefulWidget {
  @override
  _WallpaperWidgetState createState() => _WallpaperWidgetState();
}

class _WallpaperWidgetState extends State<WallpaperWidget> {
  StreamSubscription? _monitorSub;

  @override
  void initState() {
    super.initState();
    // Setup listeners
    AnyWPEngine.setOnMonitorChangeCallback(_handleMonitorChange);
  }

  @override
  void dispose() {
    // Clean up
    _monitorSub?.cancel();
    super.dispose();
  }

  void _handleMonitorChange() {
    // Handle change
  }
}
```

### ✅ DO: Monitor Memory Usage

```dart
class MemoryMonitor {
  Timer? _timer;

  void startMonitoring() {
    _timer = Timer.periodic(Duration(minutes: 5), (_) async {
      final memory = await AnyWPEngine.getMemoryUsage();
      
      if (memory > 500) {  // 500MB threshold
        print('High memory usage: ${memory}MB');
        await AnyWPEngine.optimizeMemory();
      }
    });
  }

  void stopMonitoring() {
    _timer?.cancel();
  }
}
```

### ✅ DO: Set Appropriate Thresholds

```dart
// Configure based on available RAM
Future<void> configureMemorySettings() async {
  // Get system RAM (pseudo-code)
  final systemRAM = getSystemRAM();
  
  if (systemRAM < 8 * 1024) {  // Less than 8GB
    // Conservative settings for low-RAM systems
    await AnyWPEngine.setMemoryThreshold(150);
    await AnyWPEngine.setCleanupInterval(30);
  } else {
    // Normal settings
    await AnyWPEngine.setMemoryThreshold(300);
    await AnyWPEngine.setCleanupInterval(60);
  }
}
```

### ❌ DON'T: Load Large Assets Unnecessarily

```javascript
// Avoid: Loading all assets upfront
const images = [/* hundreds of images */];
images.forEach(src => {
  const img = new Image();
  img.src = src;
});

// Good: Lazy loading
function loadImageWhenNeeded(src) {
  const img = new Image();
  img.src = src;
  return img;
}
```

---

## Power Efficiency

### ✅ DO: Enable Auto Power Saving

```dart
Future<void> initializeWallpaper() async {
  // Always enable auto power saving
  await AnyWPEngine.setAutoPowerSaving(true);
  
  // Configure reasonable idle timeout
  await AnyWPEngine.setIdleTimeout(300);  // 5 minutes
  
  await AnyWPEngine.initializeWallpaper(url: url);
}
```

### ✅ DO: Adapt to Power Source

```dart
import 'package:battery_plus/battery_plus.dart';

class PowerAwareSettings {
  final _battery = Battery();

  Future<void> setup() async {
    _battery.onBatteryStateChanged.listen((state) {
      if (state == BatteryState.discharging) {
        _enableAggressivePowerSaving();
      } else {
        _enableNormalMode();
      }
    });
  }

  Future<void> _enableAggressivePowerSaving() async {
    await AnyWPEngine.setIdleTimeout(180);      // 3 min
    await AnyWPEngine.setMemoryThreshold(150);
    await AnyWPEngine.setCleanupInterval(30);
  }

  Future<void> _enableNormalMode() async {
    await AnyWPEngine.setIdleTimeout(600);      // 10 min
    await AnyWPEngine.setMemoryThreshold(300);
    await AnyWPEngine.setCleanupInterval(60);
  }
}
```

### ✅ DO: Handle Visibility Changes

```dart
class SmartWallpaperController {
  void setupPowerStateListener() {
    AnyWPEngine.setOnPowerStateChangeCallback((old, newState) {
      print('State: $old -> $newState');
      
      // State changes are automatic - no action needed
      // Content is notified via Page Visibility API
    });
  }
}
```

**Web Side - Auto-Pause Media:**
```javascript
// SDK automatically handles video/audio pause
// But you can add custom behavior:
AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    console.log('Wallpaper visible - resuming');
    resumeCustomAnimations();
  } else {
    console.log('Wallpaper hidden - pausing');
    pauseCustomAnimations();
  }
});
```

### ✅ DO: Use Page Visibility API

```javascript
// Good: Pause animations when wallpaper is hidden
let isPaused = false;
let lastFrameTime = 0;

// Listen to AnyWP visibility events
window.addEventListener('AnyWP:visibility', (e) => {
  isPaused = !e.detail.visible;
  if (isPaused) {
    lastFrameTime = Date.now();  // Save state
  }
});

function renderParticles() {
  if (!isPaused) {
    particles.forEach(p => {
      p.update();
      p.render();
    });
  }
  requestAnimationFrame(renderParticles);
}
```

### ✅ DO: Preserve State During Pause

```javascript
// Good: Save state and resume smoothly
let animationState = {
  progress: 0,
  timestamp: 0,
};

AnyWP.onVisibilityChange(function(visible) {
  if (!visible) {
    // Save current state
    animationState.timestamp = Date.now();
    pauseAnimation();
  } else {
    // Resume from saved state (no restart)
    const pausedDuration = Date.now() - animationState.timestamp;
    resumeAnimation(animationState.progress);
  }
});
```

### ❌ DON'T: Restart Animations on Resume

```javascript
// Avoid: Restarting from beginning
AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    startAnimationFromBeginning();  // Bad - jarring experience
  }
});

// Good: Continue from where it stopped
AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    continueAnimation();  // Smooth transition
  }
});
```

---

## Multi-Monitor Support

### ✅ DO: Handle Monitor Changes Gracefully

```dart
class MonitorManager {
  List<MonitorInfo> _monitors = [];
  Map<int, String> _monitorUrls = {};

  Future<void> initialize() async {
    AnyWPEngine.setOnMonitorChangeCallback(_handleMonitorChange);
    await _loadMonitors();
  }

  Future<void> _loadMonitors() async {
    final newMonitors = await AnyWPEngine.getMonitors();
    final oldIndices = _monitors.map((m) => m.index).toSet();
    final newIndices = newMonitors.map((m) => m.index).toSet();

    // Handle removed monitors
    final removed = oldIndices.difference(newIndices);
    for (final index in removed) {
      await _cleanupMonitor(index);
    }

    // Handle added monitors
    final added = newIndices.difference(oldIndices);
    for (final index in added) {
      await _setupNewMonitor(index);
    }

    _monitors = newMonitors;
  }

  Future<void> _handleMonitorChange() async {
    print('Monitor configuration changed');
    await _loadMonitors();
  }

  Future<void> _cleanupMonitor(int index) async {
    await AnyWPEngine.stopWallpaperOnMonitor(index);
    _monitorUrls.remove(index);
  }

  Future<void> _setupNewMonitor(int index) async {
    // Optionally auto-start on new monitors
    final defaultUrl = getDefaultUrl();
    if (defaultUrl != null) {
      await AnyWPEngine.initializeWallpaperOnMonitor(
        url: defaultUrl,
        monitorIndex: index,
        enableMouseTransparent: true,
      );
    }
  }
}
```

### ✅ DO: Test Different Configurations

```dart
// Test with:
// - Single monitor
// - Dual monitors (same resolution)
// - Dual monitors (different resolutions)
// - Triple+ monitors
// - Mixed DPI scaling
// - Portrait + Landscape

Future<void> runMultiMonitorTests() async {
  final monitors = await AnyWPEngine.getMonitors();
  
  print('Testing ${monitors.length} monitor(s)');
  
  for (var monitor in monitors) {
    print('Monitor ${monitor.index}:');
    print('  Resolution: ${monitor.width}x${monitor.height}');
    print('  Position: (${monitor.left}, ${monitor.top})');
    print('  Primary: ${monitor.isPrimary}');
    
    // Test individual control
    await testMonitor(monitor);
  }
}
```

### ❌ DON'T: Assume Primary Monitor Only

```dart
// Avoid: Only using primary monitor
await AnyWPEngine.initializeWallpaper(url: url);

// Good: Support all monitors
final monitors = await AnyWPEngine.getMonitors();
for (var monitor in monitors) {
  await AnyWPEngine.initializeWallpaperOnMonitor(
    url: getUrlForMonitor(monitor),
    monitorIndex: monitor.index,
  );
}
```

---

## Error Handling

### ✅ DO: Always Check Return Values

```dart
Future<void> startWallpaper(String url) async {
  try {
    final success = await AnyWPEngine.initializeWallpaper(url: url);
    
    if (!success) {
      // Handle failure
      print('Failed to initialize wallpaper');
      _showError('Could not start wallpaper');
      return;
    }
    
    print('Wallpaper started successfully');
  } catch (e, stack) {
    // Handle exception
    print('Error: $e');
    print('Stack: $stack');
    _showError('An error occurred: $e');
  }
}
```

### ✅ DO: Validate URLs

```dart
bool isValidUrl(String url) {
  try {
    final uri = Uri.parse(url);
    return uri.hasScheme && 
           (uri.scheme == 'http' || 
            uri.scheme == 'https' || 
            uri.scheme == 'file');
  } catch (e) {
    return false;
  }
}

Future<void> startWallpaper(String url) async {
  if (!isValidUrl(url)) {
    _showError('Invalid URL format');
    return;
  }
  
  await AnyWPEngine.initializeWallpaper(url: url);
}
```

### ✅ DO: Provide User Feedback

```dart
class WallpaperController {
  Future<void> startWithFeedback(BuildContext context, String url) async {
    // Show loading
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (context) => Center(child: CircularProgressIndicator()),
    );
    
    try {
      final success = await AnyWPEngine.initializeWallpaper(url: url);
      
      // Hide loading
      Navigator.pop(context);
      
      if (success) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Wallpaper started successfully'),
            backgroundColor: Colors.green,
          ),
        );
      } else {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to start wallpaper'),
            backgroundColor: Colors.red,
            action: SnackBarAction(
              label: 'Retry',
              onPressed: () => startWithFeedback(context, url),
            ),
          ),
        );
      }
    } catch (e) {
      Navigator.pop(context);
      _showErrorDialog(context, e.toString());
    }
  }
}
```

---

## User Experience

### ✅ DO: Save User Preferences

```dart
import 'package:shared_preferences/shared_preferences.dart';

class UserPreferences {
  static const String KEY_WALLPAPER_URL = 'wallpaper_url';
  static const String KEY_AUTO_POWER_SAVING = 'auto_power_saving';
  static const String KEY_IDLE_TIMEOUT = 'idle_timeout';

  Future<void> saveWallpaperUrl(String url) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(KEY_WALLPAPER_URL, url);
  }

  Future<String?> getWallpaperUrl() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString(KEY_WALLPAPER_URL);
  }

  Future<void> saveSettings({
    required bool autoPowerSaving,
    required int idleTimeout,
  }) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool(KEY_AUTO_POWER_SAVING, autoPowerSaving);
    await prefs.setInt(KEY_IDLE_TIMEOUT, idleTimeout);
  }

  Future<void> applySavedSettings() async {
    final prefs = await SharedPreferences.getInstance();
    
    final autoPowerSaving = prefs.getBool(KEY_AUTO_POWER_SAVING) ?? true;
    final idleTimeout = prefs.getInt(KEY_IDLE_TIMEOUT) ?? 300;
    
    await AnyWPEngine.setAutoPowerSaving(autoPowerSaving);
    await AnyWPEngine.setIdleTimeout(idleTimeout);
  }
}
```

### ✅ DO: Provide Visual Feedback

```dart
class StatusIndicator extends StatelessWidget {
  final String powerState;
  final int memoryUsage;

  const StatusIndicator({
    required this.powerState,
    required this.memoryUsage,
  });

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        _buildPowerStateIcon(),
        SizedBox(width: 8),
        Text('$memoryUsage MB'),
        SizedBox(width: 8),
        _buildMemoryIndicator(),
      ],
    );
  }

  Widget _buildPowerStateIcon() {
    IconData icon;
    Color color;

    switch (powerState) {
      case 'ACTIVE':
        icon = Icons.check_circle;
        color = Colors.green;
        break;
      case 'PAUSED':
        icon = Icons.pause_circle;
        color = Colors.orange;
        break;
      case 'LOCKED':
        icon = Icons.lock;
        color = Colors.red;
        break;
      default:
        icon = Icons.help;
        color = Colors.grey;
    }

    return Icon(icon, color: color, size: 20);
  }

  Widget _buildMemoryIndicator() {
    final color = memoryUsage > 300 ? Colors.red : Colors.green;
    return LinearProgressIndicator(
      value: memoryUsage / 500,
      color: color,
    );
  }
}
```

### ✅ DO: Handle First-Time Setup

```dart
class FirstTimeSetup {
  Future<bool> isFirstRun() async {
    final prefs = await SharedPreferences.getInstance();
    return !prefs.containsKey('setup_completed');
  }

  Future<void> runSetup(BuildContext context) async {
    // Show welcome dialog
    await showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: Text('Welcome to AnyWP Engine!'),
        content: Text(
          'This app allows you to set interactive web content as your desktop wallpaper.\n\n'
          'Tips:\n'
          '• Enable auto power saving for better battery life\n'
          '• Each monitor can display different content\n'
          '• Wallpaper automatically pauses when system is locked',
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: Text('Get Started'),
          ),
        ],
      ),
    );

    // Apply recommended settings
    await AnyWPEngine.setAutoPowerSaving(true);
    await AnyWPEngine.setIdleTimeout(300);
    await AnyWPEngine.setMemoryThreshold(300);

    // Mark setup as complete
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool('setup_completed', true);
  }
}
```

---

## Security

### ✅ DO: Validate User Input

```dart
class UrlValidator {
  static bool isSecure(String url) {
    try {
      final uri = Uri.parse(url);
      
      // Only allow safe schemes
      if (!['http', 'https', 'file'].contains(uri.scheme)) {
        return false;
      }
      
      // Warn about non-HTTPS
      if (uri.scheme == 'http') {
        print('Warning: Non-secure HTTP connection');
      }
      
      return true;
    } catch (e) {
      return false;
    }
  }

  static bool isTrustedDomain(String url) {
    try {
      final uri = Uri.parse(url);
      
      // Whitelist trusted domains
      final trustedDomains = [
        'example.com',
        'localhost',
        // Add your trusted domains
      ];
      
      return trustedDomains.any(
        (domain) => uri.host.endsWith(domain)
      );
    } catch (e) {
      return false;
    }
  }
}
```

### ✅ DO: Sanitize File Paths

```dart
String sanitizeFilePath(String path) {
  // Remove dangerous characters
  path = path.replaceAll('..', '');
  path = path.replaceAll('//', '/');
  
  // Ensure it's an absolute path
  if (!path.startsWith('/') && !path.contains(':')) {
    return '';
  }
  
  return path;
}
```

### ❌ DON'T: Store Sensitive Data in Web Content

```javascript
// Avoid: Exposing API keys or sensitive data
const API_KEY = 'secret_key_12345';  // DON'T DO THIS

// Good: Fetch from secure backend
async function getApiKey() {
  const response = await fetch('https://secure-api.com/get-key');
  return await response.json();
}
```

---

## Summary Checklist

### Performance
- [ ] Optimize web content (images, animations)
- [ ] Minimize DOM complexity
- [ ] Use `requestAnimationFrame` for animations
- [ ] Debounce frequent updates

### Memory
- [ ] Clean up event listeners
- [ ] Monitor memory usage regularly
- [ ] Set appropriate thresholds
- [ ] Avoid loading unnecessary assets

### Power
- [ ] Enable auto power saving
- [ ] Adapt to power source (battery vs AC)
- [ ] Pause when not visible
- [ ] Stop heavy animations when paused

### Multi-Monitor
- [ ] Handle monitor changes gracefully
- [ ] Test different configurations
- [ ] Support all monitors, not just primary

### Errors
- [ ] Check all return values
- [ ] Validate URLs before use
- [ ] Provide user feedback

### UX
- [ ] Save user preferences
- [ ] Provide visual feedback
- [ ] Handle first-time setup
- [ ] Show meaningful status information

### Security
- [ ] Validate all user input
- [ ] Sanitize file paths
- [ ] Use HTTPS when possible
- [ ] Don't expose sensitive data

---

## See Also

- [Developer API Reference](DEVELOPER_API_REFERENCE.md)
- [Usage Examples](API_USAGE_EXAMPLES.md)
- [Troubleshooting](TROUBLESHOOTING.md)

