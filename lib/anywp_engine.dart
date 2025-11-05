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
  
  /// Set callback for monitor change events
  static void setOnMonitorChangeCallback(void Function() callback) {
    print('[AnyWPEngine] Setting up monitor change callback');
    _onMonitorChangeCallback = callback;
    
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
            print('[AnyWPEngine] WARNING: Callback is null!');
          }
        } else {
          print('[AnyWPEngine] Unknown method: ${call.method}');
        }
      } catch (e, stackTrace) {
        print('[AnyWPEngine] ERROR in method handler: $e');
        print('[AnyWPEngine] StackTrace: $stackTrace');
      }
    });
    
    print('[AnyWPEngine] Monitor change callback setup complete');
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
}

