import 'dart:async';
import 'package:flutter/services.dart';

/// 显示器信息
class MonitorInfo {
  final int index;
  final String deviceName;
  final int left;
  final int top;
  final int width;
  final int height;
  final bool isPrimary;

  MonitorInfo({
    required this.index,
    required this.deviceName,
    required this.left,
    required this.top,
    required this.width,
    required this.height,
    required this.isPrimary,
  });

  factory MonitorInfo.fromMap(Map<dynamic, dynamic> map) {
    return MonitorInfo(
      index: map['index'] as int,
      deviceName: map['deviceName'] as String,
      left: map['left'] as int,
      top: map['top'] as int,
      width: map['width'] as int,
      height: map['height'] as int,
      isPrimary: map['isPrimary'] as bool,
    );
  }

  @override
  String toString() {
    return 'MonitorInfo(index: $index, name: $deviceName, ${width}x$height @ ($left, $top)${isPrimary ? ' [PRIMARY]' : ''})';
  }
}

class AnyWPEngine {
  static const MethodChannel _channel = MethodChannel('anywp_engine');
  
  // Callback for monitor change events
  static void Function()? _onMonitorChangeCallback;
  
  // Callback for power state changes
  static void Function(String oldState, String newState)? _onPowerStateChangeCallback;
  
  /// Set callback for monitor change events
  static void setOnMonitorChangeCallback(void Function() callback) {
    print('[AnyWPEngine] Setting up monitor change callback');
    _onMonitorChangeCallback = callback;
    _setupMethodCallHandler();
    print('[AnyWPEngine] Monitor change callback setup complete');
  }
  
  /// Set callback for power state changes
  /// 
  /// The callback receives two parameters:
  /// - [oldState]: The previous power state
  /// - [newState]: The new power state
  /// 
  /// States can be: ACTIVE, IDLE, SCREEN_OFF, LOCKED, FULLSCREEN_APP, PAUSED
  /// 
  /// Example:
  /// ```dart
  /// AnyWPEngine.setOnPowerStateChangeCallback((oldState, newState) {
  ///   print('Power state changed: $oldState -> $newState');
  ///   if (newState == 'LOCKED') {
  ///     // Handle system lock
  ///   }
  /// });
  /// ```
  static void setOnPowerStateChangeCallback(
    void Function(String oldState, String newState) callback
  ) {
    print('[AnyWPEngine] Setting up power state change callback');
    _onPowerStateChangeCallback = callback;
    _setupMethodCallHandler();
    print('[AnyWPEngine] Power state change callback setup complete');
  }
  
  /// Setup method call handler (internal)
  static void _setupMethodCallHandler() {
    // Set method call handler for callbacks from native
    _channel.setMethodCallHandler((call) async {
      print('[AnyWPEngine] Received method call from native: ${call.method}');
      
      try {
        if (call.method == 'onMonitorChange') {
          print('[AnyWPEngine] Monitor change detected from native - calling callback');
          if (_onMonitorChangeCallback != null) {
            _onMonitorChangeCallback!();
            print('[AnyWPEngine] Callback executed successfully');
          } else {
            print('[AnyWPEngine] WARNING: Monitor change callback is null!');
          }
        } else if (call.method == 'onPowerStateChange') {
          final args = call.arguments as Map<dynamic, dynamic>;
          final oldState = args['oldState'] as String;
          final newState = args['newState'] as String;
          
          print('[AnyWPEngine] Power state changed: $oldState -> $newState');
          if (_onPowerStateChangeCallback != null) {
            _onPowerStateChangeCallback!(oldState, newState);
            print('[AnyWPEngine] Power state callback executed successfully');
          } else {
            print('[AnyWPEngine] WARNING: Power state callback is null!');
          }
        } else {
          print('[AnyWPEngine] Unknown method: ${call.method}');
        }
      } catch (e, stackTrace) {
        print('[AnyWPEngine] ERROR in method handler: $e');
        print('[AnyWPEngine] StackTrace: $stackTrace');
      }
    });
  }

  /// Initialize WebView2 as desktop wallpaper
  static Future<bool> initializeWallpaper({
    required String url,
    bool enableMouseTransparent = true,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('initializeWallpaper', {
        'url': url,
        'enableMouseTransparent': enableMouseTransparent,
      });
      return result ?? false;
    } catch (e) {
      print('Error initializing wallpaper: $e');
      return false;
    }
  }

  /// Stop and cleanup wallpaper
  static Future<bool> stopWallpaper() async {
    try {
      final result = await _channel.invokeMethod<bool>('stopWallpaper');
      return result ?? false;
    } catch (e) {
      print('Error stopping wallpaper: $e');
      return false;
    }
  }

  /// Navigate to URL
  static Future<bool> navigateToUrl(String url) async {
    try {
      final result = await _channel.invokeMethod<bool>('navigateToUrl', {
        'url': url,
      });
      return result ?? false;
    } catch (e) {
      print('Error navigating to URL: $e');
      return false;
    }
  }

  // ========== Multi-Monitor Support ==========

  /// Get all available monitors
  static Future<List<MonitorInfo>> getMonitors() async {
    try {
      final result = await _channel.invokeMethod<List<dynamic>>('getMonitors');
      if (result == null) return [];
      
      return result.map((e) => MonitorInfo.fromMap(e as Map<dynamic, dynamic>)).toList();
    } catch (e) {
      print('Error getting monitors: $e');
      return [];
    }
  }

  /// Initialize WebView2 as desktop wallpaper on specific monitor
  static Future<bool> initializeWallpaperOnMonitor({
    required String url,
    required int monitorIndex,
    bool enableMouseTransparent = true,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('initializeWallpaperOnMonitor', {
        'url': url,
        'monitorIndex': monitorIndex,
        'enableMouseTransparent': enableMouseTransparent,
      });
      return result ?? false;
    } catch (e) {
      print('Error initializing wallpaper on monitor $monitorIndex: $e');
      return false;
    }
  }

  /// Stop wallpaper on specific monitor
  static Future<bool> stopWallpaperOnMonitor(int monitorIndex) async {
    try {
      final result = await _channel.invokeMethod<bool>('stopWallpaperOnMonitor', {
        'monitorIndex': monitorIndex,
      });
      return result ?? false;
    } catch (e) {
      print('Error stopping wallpaper on monitor $monitorIndex: $e');
      return false;
    }
  }

  /// Navigate to URL on specific monitor
  static Future<bool> navigateToUrlOnMonitor(String url, int monitorIndex) async {
    try {
      final result = await _channel.invokeMethod<bool>('navigateToUrlOnMonitor', {
        'url': url,
        'monitorIndex': monitorIndex,
      });
      return result ?? false;
    } catch (e) {
      print('Error navigating to URL on monitor $monitorIndex: $e');
      return false;
    }
  }

  /// Initialize wallpaper on all monitors
  static Future<Map<int, bool>> initializeWallpaperOnAllMonitors({
    required String url,
    bool enableMouseTransparent = true,
  }) async {
    final monitors = await getMonitors();
    final results = <int, bool>{};
    
    for (final monitor in monitors) {
      final success = await initializeWallpaperOnMonitor(
        url: url,
        monitorIndex: monitor.index,
        enableMouseTransparent: enableMouseTransparent,
      );
      results[monitor.index] = success;
    }
    
    return results;
  }

  /// Stop wallpaper on all monitors
  static Future<bool> stopWallpaperOnAllMonitors() async {
    final monitors = await getMonitors();
    bool allSuccess = true;
    
    for (final monitor in monitors) {
      final success = await stopWallpaperOnMonitor(monitor.index);
      if (!success) allSuccess = false;
    }
    
    return allSuccess;
  }

  // ========== Power Saving & Optimization APIs ==========

  /// Manually pause wallpaper (stops rendering and animations)
  /// 
  /// This reduces CPU/GPU usage and memory consumption.
  /// Use this when you want to temporarily stop the wallpaper.
  static Future<bool> pauseWallpaper() async {
    try {
      final result = await _channel.invokeMethod<bool>('pauseWallpaper');
      return result ?? false;
    } catch (e) {
      print('Error pausing wallpaper: $e');
      return false;
    }
  }

  /// Resume previously paused wallpaper
  /// 
  /// This restores normal rendering and animations.
  static Future<bool> resumeWallpaper() async {
    try {
      final result = await _channel.invokeMethod<bool>('resumeWallpaper');
      return result ?? false;
    } catch (e) {
      print('Error resuming wallpaper: $e');
      return false;
    }
  }

  /// Enable or disable automatic power saving
  /// 
  /// When enabled (default), the engine will automatically pause wallpaper when:
  /// - System is locked
  /// - Screen is off
  /// - A fullscreen application is running
  /// - User is idle for more than 5 minutes
  /// 
  /// Set [enabled] to false to disable automatic power saving.
  static Future<bool> setAutoPowerSaving(bool enabled) async {
    try {
      final result = await _channel.invokeMethod<bool>('setAutoPowerSaving', {
        'enabled': enabled,
      });
      return result ?? false;
    } catch (e) {
      print('Error setting auto power saving: $e');
      return false;
    }
  }

  /// Get current power state
  /// 
  /// Returns one of:
  /// - "ACTIVE": Wallpaper is running normally
  /// - "IDLE": User is inactive, wallpaper may be paused
  /// - "SCREEN_OFF": Screen is off
  /// - "LOCKED": System is locked
  /// - "FULLSCREEN_APP": A fullscreen app is running
  /// - "PAUSED": Manually paused
  static Future<String> getPowerState() async {
    try {
      final result = await _channel.invokeMethod<String>('getPowerState');
      return result ?? 'UNKNOWN';
    } catch (e) {
      print('Error getting power state: $e');
      return 'UNKNOWN';
    }
  }

  /// Get current memory usage in MB
  /// 
  /// Returns the working set size of the current process.
  static Future<int> getMemoryUsage() async {
    try {
      final result = await _channel.invokeMethod<int>('getMemoryUsage');
      return result ?? 0;
    } catch (e) {
      print('Error getting memory usage: $e');
      return 0;
    }
  }

  /// Manually trigger memory optimization
  /// 
  /// This will:
  /// - Clear WebView cache
  /// - Trigger JavaScript garbage collection
  /// - Trim process working set
  /// 
  /// Note: This is automatically called when wallpaper is paused.
  static Future<bool> optimizeMemory() async {
    try {
      final result = await _channel.invokeMethod<bool>('optimizeMemory');
      return result ?? false;
    } catch (e) {
      print('Error optimizing memory: $e');
      return false;
    }
  }

  // ========== Configuration APIs ==========

  /// Set idle timeout in seconds
  /// 
  /// After this duration of no user input, the wallpaper will automatically pause.
  /// 
  /// - [seconds]: Timeout duration (minimum 60, default 300 = 5 minutes)
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// // Set idle timeout to 10 minutes
  /// await AnyWPEngine.setIdleTimeout(600);
  /// 
  /// // Disable idle detection (set to a very large value)
  /// await AnyWPEngine.setIdleTimeout(3600 * 24); // 24 hours
  /// ```
  static Future<bool> setIdleTimeout(int seconds) async {
    if (seconds < 60) {
      print('Warning: Idle timeout should be at least 60 seconds');
      seconds = 60;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setIdleTimeout', {
        'seconds': seconds,
      });
      return result ?? false;
    } catch (e) {
      print('Error setting idle timeout: $e');
      return false;
    }
  }

  /// Set memory optimization threshold in MB
  /// 
  /// When memory usage exceeds this threshold during periodic cleanup,
  /// optimization will be triggered automatically.
  /// 
  /// - [thresholdMB]: Memory threshold in MB (minimum 100, default 300)
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// // Set threshold to 200MB
  /// await AnyWPEngine.setMemoryThreshold(200);
  /// ```
  static Future<bool> setMemoryThreshold(int thresholdMB) async {
    if (thresholdMB < 100) {
      print('Warning: Memory threshold should be at least 100 MB');
      thresholdMB = 100;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setMemoryThreshold', {
        'thresholdMB': thresholdMB,
      });
      return result ?? false;
    } catch (e) {
      print('Error setting memory threshold: $e');
      return false;
    }
  }

  /// Set periodic cleanup interval in minutes
  /// 
  /// Controls how often the engine checks memory usage and performs cleanup.
  /// 
  /// - [minutes]: Cleanup interval (minimum 10, default 60)
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// // Check every 30 minutes
  /// await AnyWPEngine.setCleanupInterval(30);
  /// ```
  static Future<bool> setCleanupInterval(int minutes) async {
    if (minutes < 10) {
      print('Warning: Cleanup interval should be at least 10 minutes');
      minutes = 10;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setCleanupInterval', {
        'minutes': minutes,
      });
      return result ?? false;
    } catch (e) {
      print('Error setting cleanup interval: $e');
      return false;
    }
  }

  /// Get current configuration
  /// 
  /// Returns a map containing:
  /// - 'idleTimeoutSeconds': Current idle timeout in seconds
  /// - 'memoryThresholdMB': Current memory threshold in MB
  /// - 'cleanupIntervalMinutes': Current cleanup interval in minutes
  /// - 'autoPowerSavingEnabled': Whether auto power saving is enabled
  static Future<Map<String, dynamic>> getConfiguration() async {
    try {
      final result = await _channel.invokeMethod<Map<dynamic, dynamic>>('getConfiguration');
      if (result == null) return {};
      
      return result.map((key, value) => MapEntry(key.toString(), value));
    } catch (e) {
      print('Error getting configuration: $e');
      return {};
    }
  }

  // ========== State Persistence APIs ==========

  /// Save wallpaper state
  /// 
  /// Saves a key-value pair to persistent storage (Windows Registry).
  /// The state will be preserved across app restarts.
  /// 
  /// - [key]: The state key (used for retrieval)
  /// - [value]: The state value (will be JSON-encoded if not a string)
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// // Save string value
  /// await AnyWPEngine.saveState('wallpaper_url', 'https://example.com');
  /// 
  /// // Save JSON object
  /// await AnyWPEngine.saveState('settings', jsonEncode({
  ///   'volume': 0.5,
  ///   'autoplay': true,
  /// }));
  /// ```
  static Future<bool> saveState(String key, String value) async {
    try {
      final result = await _channel.invokeMethod<bool>('saveState', {
        'key': key,
        'value': value,
      });
      return result ?? false;
    } catch (e) {
      print('Error saving state: $e');
      return false;
    }
  }

  /// Load wallpaper state
  /// 
  /// Retrieves a previously saved state value from persistent storage.
  /// 
  /// - [key]: The state key
  /// - Returns: The state value, or empty string if not found
  /// 
  /// Example:
  /// ```dart
  /// // Load string value
  /// final url = await AnyWPEngine.loadState('wallpaper_url');
  /// 
  /// // Load and parse JSON object
  /// final settingsJson = await AnyWPEngine.loadState('settings');
  /// if (settingsJson.isNotEmpty) {
  ///   final settings = jsonDecode(settingsJson);
  ///   print('Volume: ${settings['volume']}');
  /// }
  /// ```
  static Future<String> loadState(String key) async {
    try {
      final result = await _channel.invokeMethod<String>('loadState', {
        'key': key,
      });
      return result ?? '';
    } catch (e) {
      print('Error loading state: $e');
      return '';
    }
  }

  /// Clear all saved state
  /// 
  /// Removes all state data from persistent storage.
  /// This operation cannot be undone.
  /// 
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// await AnyWPEngine.clearState();
  /// ```
  static Future<bool> clearState() async {
    try {
      final result = await _channel.invokeMethod<bool>('clearState');
      return result ?? false;
    } catch (e) {
      print('Error clearing state: $e');
      return false;
    }
  }
}

