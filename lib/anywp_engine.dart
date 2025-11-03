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
    _onMonitorChangeCallback = callback;
    
    // Set method call handler for callbacks from native
    _channel.setMethodCallHandler((call) async {
      if (call.method == 'onMonitorChange') {
        print('[AnyWPEngine] Monitor change detected from native');
        _onMonitorChangeCallback?.call();
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
}

