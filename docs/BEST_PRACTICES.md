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

## Advanced Features (v2.0)

### Error Handling Best Practices

#### ✅ DO: Use ErrorHandler for All Critical Operations

```cpp
// C++ side - Always wrap critical operations
TRY_CATCH_REPORT("ModuleName", "OperationName", {
  // Critical operation here
  if (!InitializeResource()) {
    return false;
  }
});
```

#### ✅ DO: Implement Retry Logic for Transient Failures

```cpp
// Use ErrorHandler's built-in retry mechanism
bool success = ErrorHandler::TryRecover(
  "DatabaseConnection",
  []() { return ConnectToDatabase(); },
  3,      // Max 3 attempts
  1000    // 1 second delay between attempts
);

if (!success) {
  Logger::Instance().Error("Database", "Failed after 3 retries");
}
```

#### ✅ DO: Categorize Errors Properly

```cpp
// Resource errors
LOG_AND_REPORT_ERROR(
  ErrorHandler::ErrorLevel::ERROR,
  ErrorHandler::ErrorCategory::RESOURCE,
  "ModuleName", "Operation", "Memory allocation failed"
);

// Permission errors
LOG_AND_REPORT_ERROR(
  ErrorHandler::ErrorLevel::WARNING,
  ErrorHandler::ErrorCategory::PERMISSION,
  "ModuleName", "Operation", "Access denied"
);
```

#### ❌ DON'T: Swallow Exceptions Without Logging

```cpp
// Avoid: Silent failure
try {
  RiskyOperation();
} catch (...) {
  // Nothing - bad!
}

// Good: Report errors
TRY_CATCH_REPORT("Module", "RiskyOperation", {
  RiskyOperation();
});
```

---

### Performance Testing Best Practices

#### ✅ DO: Benchmark Critical Code Paths

```cpp
// Benchmark initialization
void InitializeWallpaper(const string& url) {
  BENCHMARK_SCOPE("InitializeWallpaper");
  
  // Initialization code...
  CreateWebView();
  LoadContent(url);
}

// Later, check statistics
auto stats = PerformanceBenchmark::Instance().GetStatistics("InitializeWallpaper");
if (stats.avg_duration_ms > 1000) {
  Logger::Instance().Warning("Performance", 
    "InitializeWallpaper is slow: " + to_string(stats.avg_duration_ms) + "ms");
}
```

#### ✅ DO: Use Scoped Timers for Fine-Grained Measurement

```cpp
void ProcessData() {
  {
    ScopedTimer timer("DataValidation");
    ValidateData();
  }
  
  {
    ScopedTimer timer("DataTransformation");
    TransformData();
  }
  
  {
    ScopedTimer timer("DataStorage");
    StoreData();
  }
}
```

#### ✅ DO: Profile Before Optimizing

```dart
// Dart side - Measure before making changes
Future<void> loadWallpaper() async {
  final stopwatch = Stopwatch()..start();
  
  await AnyWPEngine.initializeWallpaper(url: url);
  
  stopwatch.stop();
  print('Load time: ${stopwatch.elapsedMilliseconds}ms');
  
  if (stopwatch.elapsedMilliseconds > 2000) {
    print('Warning: Slow wallpaper load');
  }
}
```

---

### Permission Control Best Practices

#### ✅ DO: Configure Permissions Early

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // Configure permission policy before initializing wallpaper
  await configurePermissions();
  
  runApp(MyApp());
}

Future<void> configurePermissions() async {
  // Set restrictive policy for production
  // Use native API or configure through ConfigManager
}
```

#### ✅ DO: Whitelist Trusted Domains

```cpp
// C++ side - Configure trusted domains
void SetupPermissions() {
  auto& pm = PermissionManager::Instance();
  
  // Allow specific domains
  pm.AddWhitelistPattern("https://example.com/*");
  pm.AddWhitelistPattern("https://cdn.example.com/*");
  
  // Block dangerous patterns
  pm.AddBlacklistPattern("file:///*");
  pm.AddBlacklistPattern("*://localhost/*");
  
  // Enforce HTTPS
  pm.SetEnforceHttps(true);
}
```

#### ✅ DO: Check Permissions Before Navigation

```dart
Future<void> navigateToUrl(String url) async {
  // Validate URL format
  if (!isValidUrl(url)) {
    showError('Invalid URL format');
    return;
  }
  
  // Permission check happens automatically in C++ layer
  try {
    await AnyWPEngine.navigateToUrl(url);
  } catch (e) {
    if (e.toString().contains('access denied')) {
      showError('URL not allowed by security policy');
    }
  }
}
```

#### ✅ DO: Review Permission Audit Logs

```cpp
// Periodically review permission violations
void ReviewPermissions() {
  auto& pm = PermissionManager::Instance();
  auto audit_log = pm.GetAuditLog();
  
  for (const auto& entry : audit_log) {
    if (!entry.allowed) {
      Logger::Instance().Warning("Security", 
        "Blocked: " + entry.url + " (" + entry.reason + ")");
    }
  }
}
```

---

### Event Bus Best Practices

#### ✅ DO: Use EventBus for Decoupled Communication

```cpp
// Module A - Publish event
void OnWallpaperStarted(int monitor_index) {
  Event event("wallpaper_started");
  event.data = "Monitor " + to_string(monitor_index);
  event.priority = 10;
  EventBus::Instance().Publish(event);
}

// Module B - Subscribe to event
void SetupListeners() {
  EventBus::Instance().Subscribe("wallpaper_started", 
    [](const Event& event) {
      Logger::Instance().Info("ModuleB", "Wallpaper started: " + event.data);
    });
}
```

#### ✅ DO: Use Event Priorities for Critical Events

```cpp
// High priority event (processed first)
Event critical_event("error_occurred");
critical_event.priority = 100;
EventBus::Instance().Publish(critical_event);

// Normal priority event
Event info_event("info_message");
info_event.priority = 10;
EventBus::Instance().Publish(info_event);
```

#### ✅ DO: Clean Up Event Subscriptions

```cpp
class MyModule {
  void Initialize() {
    // Store subscription ID
    subscription_id_ = EventBus::Instance().Subscribe("event_type", 
      [this](const Event& e) { HandleEvent(e); });
  }
  
  ~MyModule() {
    // Unsubscribe on destruction
    EventBus::Instance().Unsubscribe("event_type", subscription_id_);
  }
  
private:
  int subscription_id_;
};
```

#### ❌ DON'T: Create Circular Event Dependencies

```cpp
// Avoid: Event A triggers Event B which triggers Event A
EventBus::Instance().Subscribe("event_a", [](const Event& e) {
  Event event_b("event_b");
  EventBus::Instance().Publish(event_b);  // Bad if event_b triggers event_a
});
```

---

### Configuration Management Best Practices

#### ✅ DO: Use ConfigManager for All Settings

```cpp
// Initialize with defaults
void InitializeConfig() {
  auto& cm = ConfigManager::Instance();
  
  // Set default values
  cm.Set("log.level", "INFO");
  cm.Set("log.file_enabled", true);
  cm.Set("power.idle_timeout", 300);
  cm.Set("power.memory_threshold", 300);
  
  // Load from file (overrides defaults)
  cm.LoadFromFile("config.json");
}
```

#### ✅ DO: Use Profiles for Different Environments

```cpp
// Development environment
ConfigManager::Instance().SetActiveProfile("dev");
ConfigManager::Instance().Set("log.level", "DEBUG");
ConfigManager::Instance().Set("performance.benchmarking", true);

// Production environment
ConfigManager::Instance().SetActiveProfile("prod");
ConfigManager::Instance().Set("log.level", "WARNING");
ConfigManager::Instance().Set("performance.benchmarking", false);
```

#### ✅ DO: Validate Configuration Values

```cpp
// Add validator for critical settings
ConfigManager::Instance().AddValidator("power.idle_timeout", 
  [](const ConfigValue& value) {
    int timeout = value.GetInt();
    return timeout >= 60 && timeout <= 3600;  // 1 min to 1 hour
  });

// Invalid values will be rejected
try {
  ConfigManager::Instance().Set("power.idle_timeout", 10);  // Throws
} catch (const exception& e) {
  Logger::Instance().Error("Config", "Invalid timeout value");
}
```

#### ✅ DO: React to Configuration Changes

```cpp
// Listen for config changes
ConfigManager::Instance().AddChangeListener("log.level", 
  [](const ConfigValue& old_val, const ConfigValue& new_val) {
    string new_level = new_val.GetString();
    Logger::Instance().SetMinLevel(new_level);
    Logger::Instance().Info("Config", "Log level changed to: " + new_level);
  });
```

#### ✅ DO: Persist Configuration Changes

```cpp
// Save configuration periodically or on change
void OnConfigurationChanged() {
  ConfigManager::Instance().SaveToFile("config.json");
}

// Or save on application exit
void OnApplicationExit() {
  ConfigManager::Instance().SaveToFile("config.json");
}
```

---

### Dependency Injection Best Practices

#### ✅ DO: Register Services Early

```cpp
// In main initialization
void InitializeServices() {
  auto& sl = ServiceLocator::Instance();
  
  // Register core services
  auto logger = make_shared<LoggerImpl>();
  sl.Register<ILogger>(logger);
  
  auto storage = make_shared<StatePersistenceImpl>();
  sl.Register<IStateStorage>(storage);
  
  auto webview_mgr = make_shared<WebViewManagerImpl>();
  sl.Register<IWebViewManager>(webview_mgr);
}
```

#### ✅ DO: Use Interfaces for Testability

```cpp
// Production code uses interface
class MyModule {
public:
  MyModule() {
    logger_ = ServiceLocator::Instance().Resolve<ILogger>();
  }
  
  void DoWork() {
    logger_->Info("MyModule", "Working...");
  }
  
private:
  shared_ptr<ILogger> logger_;
};

// Test code can inject mock
class MockLogger : public ILogger {
  // Mock implementation
};

// In tests
ServiceLocator::Instance().Register<ILogger>(make_shared<MockLogger>());
```

#### ✅ DO: Check Service Availability

```cpp
void UseOptionalService() {
  auto& sl = ServiceLocator::Instance();
  
  if (sl.Has<IWebViewManager>()) {
    auto webview = sl.Resolve<IWebViewManager>();
    webview->Navigate("https://example.com");
  } else {
    Logger::Instance().Warning("Module", "WebView service not available");
  }
}
```

---

## Summary Checklist (v2.0)

### Architecture
- [ ] Use modular design (follow Single Responsibility Principle)
- [ ] Implement proper error handling (ErrorHandler)
- [ ] Add performance benchmarks (PerformanceBenchmark)
- [ ] Configure permissions (PermissionManager)
- [ ] Use event bus for decoupled communication (EventBus)
- [ ] Centralize configuration (ConfigManager)
- [ ] Use dependency injection (ServiceLocator)

### Code Quality
- [ ] Write unit tests (target ≥95% coverage)
- [ ] Document all public APIs
- [ ] Use meaningful variable and function names
- [ ] Follow naming conventions (PascalCase for C++, camelCase for Dart)
- [ ] Avoid code duplication (extract common patterns)

### Performance
- [ ] Benchmark critical paths before optimizing
- [ ] Profile memory usage regularly
- [ ] Use efficient data structures (prefer O(1) lookups)
- [ ] Minimize disk I/O (use buffering)
- [ ] Cache expensive computations

### Security
- [ ] Validate all user input
- [ ] Use permission manager for URL access
- [ ] Sanitize file paths
- [ ] Enforce HTTPS when possible
- [ ] Audit permission violations regularly

### Testing
- [ ] Write unit tests for all modules
- [ ] Test error handling paths
- [ ] Test with multiple monitors
- [ ] Test hot-plug scenarios
- [ ] Test power state transitions

---

## See Also

- [Developer API Reference](DEVELOPER_API_REFERENCE.md)
- [Usage Examples](API_USAGE_EXAMPLES.md)
- [Troubleshooting](TROUBLESHOOTING.md)
- [Architecture Design](ARCHITECTURE_DESIGN.md) (v2.0) ⭐
- [Technical Notes](TECHNICAL_NOTES.md)

