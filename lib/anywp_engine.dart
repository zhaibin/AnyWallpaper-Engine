import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
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
  final double? scaleFactor;  // macOS: 1.0 (standard) or 2.0 (Retina)
  final int? dpi;  // macOS: 72 or 144
  final int? physicalWidth;  // macOS: actual pixels (width * scaleFactor)
  final int? physicalHeight;  // macOS: actual pixels (height * scaleFactor)

  MonitorInfo({
    required this.index,
    required this.deviceName,
    required this.left,
    required this.top,
    required this.width,
    required this.height,
    required this.isPrimary,
    this.scaleFactor,
    this.dpi,
    this.physicalWidth,
    this.physicalHeight,
  });

  factory MonitorInfo.fromMap(Map<dynamic, dynamic> map) {
    return MonitorInfo(
      index: map['index'] as int,
      deviceName: map['deviceName'] as String,
      left: map['left'] as int,
      top: map['top'] as int,
      width: map['width'] as int,
      height: map['height'] as int,
      isPrimary: (map['isPrimary'] is bool) 
          ? map['isPrimary'] as bool 
          : (map['isPrimary'] as int) == 1,  // macOS 可能返回整数 0/1
      scaleFactor: map['scaleFactor'] != null ? (map['scaleFactor'] as num).toDouble() : null,
      dpi: map['dpi'] as int?,
      physicalWidth: map['physicalWidth'] as int?,
      physicalHeight: map['physicalHeight'] as int?,
    );
  }

  @override
  String toString() {
    String result = 'MonitorInfo(index: $index, name: $deviceName, ${width}x$height @ ($left, $top)${isPrimary ? ' [PRIMARY]' : ''})';
    if (scaleFactor != null) {
      result += ', scale: $scaleFactor, dpi: $dpi';
    }
    return result;
  }
}

class AnyWPEngine {
  static const MethodChannel _channel = MethodChannel('anywp_engine');
  
  // Callback for monitor change events
  static void Function()? _onMonitorChangeCallback;
  
  // Callback for power state changes
  static void Function(String oldState, String newState)? _onPowerStateChangeCallback;
  static Timer? _powerStatePollingTimer;
  
  // Callback for messages from JavaScript
  static void Function(Map<String, dynamic> message)? _onMessageCallback;
  static Timer? _messagePollingTimer;
  
  // Callback for auto recovery completion (v2.4.1+)
  static Future<void> Function(List<int> recoveredMonitors)? _onRecoveryCallback;
  
  /// Set callback for monitor change events
  static void setOnMonitorChangeCallback(void Function() callback) {
    debugPrint('[AnyWPEngine] Setting up monitor change callback');
    _onMonitorChangeCallback = callback;
    _setupMethodCallHandler();
    debugPrint('[AnyWPEngine] Monitor change callback setup complete');
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
    debugPrint('[AnyWPEngine] Setting up power state change callback');
    _onPowerStateChangeCallback = callback;
    _setupMethodCallHandler();
    _startPowerStatePolling();
    debugPrint('[AnyWPEngine] Power state change callback setup complete');
  }
  
  /// Start polling for power state changes (v2.1.1+ Fix: avoids InvokeMethod deadlock)
  static void _startPowerStatePolling() {
    // Cancel existing timer if any
    _powerStatePollingTimer?.cancel();
    
    // Poll for power state changes every 1000ms
    _powerStatePollingTimer = Timer.periodic(const Duration(milliseconds: 1000), (timer) async {
      if (_onPowerStateChangeCallback == null) {
        timer.cancel();
        return;
      }
      
      try {
        final changes = await _channel.invokeMethod<List>('getPendingPowerStateChanges');
        if (changes != null && changes.isNotEmpty) {
          debugPrint('[AnyWPEngine] Retrieved ${changes.length} pending power state changes');
          for (final changeData in changes) {
            if (changeData is Map) {
              final oldState = changeData['oldState'] as String?;
              final newState = changeData['newState'] as String?;
              if (oldState != null && newState != null) {
                debugPrint('[AnyWPEngine] Power state changed: $oldState -> $newState');
                _onPowerStateChangeCallback!(oldState, newState);
              }
            }
          }
        }
      } catch (e) {
        // Silently ignore errors to avoid spam
      }
    });
    
    debugPrint('[AnyWPEngine] Power state polling started (1000ms interval)');
  }
  
  /// Set callback for messages from JavaScript
  /// 
  /// The callback receives a map containing the message data from JavaScript.
  /// This enables bidirectional communication between Flutter and the wallpaper.
  /// 
  /// Message format (standard):
  /// ```json
  /// {
  ///   "id": "unique-message-id",
  ///   "type": "message_type",
  ///   "timestamp": 1699876543210,
  ///   "data": { ... }
  /// }
  /// ```
  /// 
  /// Common message types:
  /// - `carouselStateChanged`: Wallpaper carousel state updated
  /// - `wallpaperReady`: Wallpaper initialization complete
  /// - `error`: Error occurred in JavaScript
  /// - `heartbeat`: Heartbeat/ping message
  /// - Custom types defined by your wallpaper
  /// 
  /// Example:
  /// ```dart
  /// AnyWPEngine.setOnMessageCallback((message) {
  ///   print('Received from JavaScript: ${message['type']}');
  ///   
  ///   switch (message['type']) {
  ///     case 'carouselStateChanged':
  ///       final data = message['data'] as Map<String, dynamic>;
  ///       final currentIndex = data['currentIndex'] as int;
  ///       print('Carousel index: $currentIndex');
  ///       break;
  ///       
  ///     case 'error':
  ///       final data = message['data'] as Map<String, dynamic>;
  ///       debugPrint('Error: ${data['message']}');
  ///       break;
  ///   }
  /// });
  /// ```
  static void setOnMessageCallback(
    void Function(Map<String, dynamic> message) callback
  ) {
    debugPrint('[AnyWPEngine] Setting up message callback');
    _onMessageCallback = callback;
    _setupMethodCallHandler();
    _startMessagePolling();
    debugPrint('[AnyWPEngine] Message callback and polling setup complete');
  }

  /// Clear registered callbacks and stop background polling timers.
  ///
  /// Call this when the owning widget/app is disposed and no longer wants to
  /// receive plugin callbacks.
  static void clearCallbacks() {
    _onMonitorChangeCallback = null;
    _onPowerStateChangeCallback = null;
    _onMessageCallback = null;
    _onRecoveryCallback = null;
    _powerStatePollingTimer?.cancel();
    _powerStatePollingTimer = null;
    _messagePollingTimer?.cancel();
    _messagePollingTimer = null;
  }
  
  /// Set callback for wallpaper recovery completion (v2.4.1+)
  /// 
  /// When Explorer restarts and the wallpaper is automatically recovered,
  /// this callback is invoked after the wallpaper display is restored.
  /// Use it to restore your application state (e.g., carousel config, playback state).
  /// 
  /// **Use Cases:**
  /// - Restore interactive wallpaper state
  /// - Re-send configuration data to HTML
  /// - Restore play/pause state
  /// - Update UI to reflect recovery
  /// 
  /// **Parameters:**
  /// - [callback]: Function receiving list of recovered monitor indices
  /// 
  /// **Example (Basic):**
  /// ```dart
  /// void main() async {
  ///   await AnyWPEngine.enableAutoRecovery(true);
  ///   
  ///   // Set recovery callback
  ///   AnyWPEngine.setOnRecoveryCallback((monitors) async {
  ///     print('Wallpaper recovered on monitors: $monitors');
  ///     
  ///     // Re-send carousel configuration
  ///     await AnyWPEngine.sendMessage({
  ///       'type': 'updateCarousel',
  ///       'data': {'images': myImages, 'interval': 5000},
  ///     });
  ///   });
  ///   
  ///   runApp(MyApp());
  /// }
  /// ```
  /// 
  /// **Example (Advanced - Restore Playback State):**
  /// ```dart
  /// class CarouselManager {
  ///   String _playState = 'stopped';
  ///   
  ///   void setupRecovery() {
  ///     AnyWPEngine.setOnRecoveryCallback((monitors) async {
  ///       print('Restoring wallpaper state...');
  ///       
  ///       // Step 1: Send carousel data
  ///       await AnyWPEngine.sendMessage({
  ///         'type': 'updateCarousel',
  ///         'data': {'images': _images, 'interval': _interval},
  ///       });
  ///       
  ///       // Step 2: Restore playback state
  ///       if (_playState == 'playing') {
  ///         await Future.delayed(Duration(milliseconds: 500));
  ///         await AnyWPEngine.sendMessage({'type': 'play'});
  ///         print('✅ Playback state restored');
  ///       }
  ///     });
  ///   }
  /// }
  /// ```
  /// 
  /// **Notes:**
  /// - Callback fires ~2-3 seconds after Explorer restart (after WebView loads)
  /// - If not set, wallpaper still auto-recovers (just without app state)
  /// - Requires [enableAutoRecovery] to be `true`
  /// - Callback is optional - only needed for stateful wallpapers
  /// 
  /// **See also:**
  /// - [enableAutoRecovery] - Enable auto-recovery feature
  /// - [setOnMessageCallback] - Receive messages from JavaScript
  static void setOnRecoveryCallback(
    Future<void> Function(List<int> recoveredMonitors)? callback
  ) {
    debugPrint('[AnyWPEngine] Setting recovery callback');
    _onRecoveryCallback = callback;
    
    // Ensure message polling is active to receive AUTO_RECOVERY_REQUEST
    if (callback != null && _messagePollingTimer == null) {
      _setupMethodCallHandler();
      _startMessagePolling();
      debugPrint('[AnyWPEngine] Message polling started for recovery handling');
    }
    
    debugPrint('[AnyWPEngine] Recovery callback setup complete');
  }
  
  /// Start polling for messages from JavaScript (avoids InvokeMethod deadlock)
  static void _startMessagePolling() {
    // Cancel existing timer if any
    _messagePollingTimer?.cancel();
    
    // Poll for messages every 1 second
    _messagePollingTimer = Timer.periodic(const Duration(seconds: 1), (timer) async {
      if (_onMessageCallback == null) {
        timer.cancel();
        return;
      }
      
      try {
        final messages = await _channel.invokeMethod<List>('getPendingMessages');
        if (messages != null && messages.isNotEmpty) {
          debugPrint('[AnyWPEngine] Retrieved ${messages.length} pending messages');
          for (final messageJson in messages) {
            if (messageJson is String) {
              _processMessage(messageJson);
            }
          }
        }
      } catch (e) {
        // Silently ignore errors to avoid spam
      }
    });
    
    debugPrint('[AnyWPEngine] Message polling started (1 second interval)');
  }
  
  /// Process a single message
  static void _processMessage(String messageJson) {
    try {
      final message = jsonDecode(messageJson) as Map<String, dynamic>;
      final messageType = message['type'] as String?;
      
      debugPrint('[AnyWPEngine] Processing message: $messageType');
      
      // v2.4.1+ Auto-handle recovery requests
      if (messageType == 'AUTO_RECOVERY_REQUEST') {
        debugPrint('[AnyWPEngine] 🔄 Auto recovery request received from C++');
        _handleAutoRecoveryRequest(message);
        return; // Don't forward to user callback
      }
      
      // Forward other messages to user callback
      if (_onMessageCallback != null) {
        _onMessageCallback!(message);
      }
    } catch (e) {
      debugPrint('[AnyWPEngine] ERROR: Failed to process message: $e');
    }
  }
  
  /// Handle AUTO_RECOVERY_REQUEST message from C++ (v2.4.1+)
  static Future<void> _handleAutoRecoveryRequest(Map<String, dynamic> message) async {
    try {
      final messageData = message['data'] as Map<String, dynamic>?;
      if (messageData == null) {
        debugPrint('[AnyWPEngine] ⚠️  No data in recovery request');
        return;
      }
      
      final configs = messageData['configs'] as List<dynamic>?;
      if (configs == null || configs.isEmpty) {
        debugPrint('[AnyWPEngine] ⚠️  No configurations to recover');
        return;
      }
      
      debugPrint('[AnyWPEngine] 📋 Recovery configurations: ${configs.length} monitor(s)');
      for (var config in configs) {
        debugPrint('[AnyWPEngine]   - Monitor ${config['monitorIndex']}: ${config['url']}');
      }
      
      // Step 1: Stop existing wallpapers
      debugPrint('[AnyWPEngine] 🛑 Stopping existing wallpapers...');
      await stopWallpaper();
      await Future.delayed(const Duration(milliseconds: 500));
      
      // Step 2: Recreate wallpapers
      debugPrint('[AnyWPEngine] 🔄 Recreating wallpapers...');
      int successCount = 0;
      final List<int> recoveredMonitors = [];
      
      for (var config in configs) {
        final monitorIndex = config['monitorIndex'] as int;
        final url = config['url'] as String;
        
        try {
          final result = await initializeWallpaperOnMonitor(
            url: url,
            monitorIndex: monitorIndex,
            autoSave: true,
          );
          
          if (result == true) {
            successCount++;
            recoveredMonitors.add(monitorIndex);
            debugPrint('[AnyWPEngine] ✅ Monitor $monitorIndex recovered successfully');
          } else {
            debugPrint('[AnyWPEngine] ❌ Monitor $monitorIndex recovery failed');
          }
        } catch (e) {
          debugPrint('[AnyWPEngine] ❌ Monitor $monitorIndex recovery exception: $e');
        }
      }
      
      debugPrint('[AnyWPEngine] 🎉 Auto recovery completed: $successCount/${configs.length} monitors');
      
      // Step 3: Wait for WebView to fully load
      if (recoveredMonitors.isNotEmpty) {
        debugPrint('[AnyWPEngine] ⏳ Waiting for WebView to load (2 seconds)...');
        await Future.delayed(const Duration(seconds: 2));
        
        // Step 4: Call user's recovery callback if set
        if (_onRecoveryCallback != null) {
          debugPrint('[AnyWPEngine] 📞 Calling user recovery callback...');
          try {
            await _onRecoveryCallback!(recoveredMonitors);
            debugPrint('[AnyWPEngine] ✅ User recovery callback completed');
          } catch (e) {
            debugPrint('[AnyWPEngine] ❌ User recovery callback error: $e');
          }
        } else {
          debugPrint('[AnyWPEngine] ℹ️  No recovery callback set (basic recovery only)');
        }
      }
    } catch (e) {
      debugPrint('[AnyWPEngine] ❌ Auto recovery error: $e');
    }
  }
  
  /// Setup method call handler (internal)
  static void _setupMethodCallHandler() {
    // Set method call handler for callbacks from native
    _channel.setMethodCallHandler((call) async {
      debugPrint('[AnyWPEngine] Received method call from native: ${call.method}');
      
      try {
        if (call.method == 'onMonitorChange') {
          debugPrint('[AnyWPEngine] Monitor change detected from native - calling callback');
          if (_onMonitorChangeCallback != null) {
            _onMonitorChangeCallback!();
            debugPrint('[AnyWPEngine] Callback executed successfully');
          } else {
            debugPrint('[AnyWPEngine] WARNING: Monitor change callback is null!');
          }
        } else if (call.method == 'onPowerStateChange') {
          final args = call.arguments as Map<dynamic, dynamic>;
          final oldState = args['oldState'] as String;
          final newState = args['newState'] as String;
          
          debugPrint('[AnyWPEngine] Power state changed: $oldState -> $newState');
          if (_onPowerStateChangeCallback != null) {
            _onPowerStateChangeCallback!(oldState, newState);
            debugPrint('[AnyWPEngine] Power state callback executed successfully');
          } else {
            debugPrint('[AnyWPEngine] WARNING: Power state callback is null!');
          }
        } else if (call.method == 'onMessage') {
          final args = call.arguments as Map<dynamic, dynamic>;
          final messageJson = args['message'] as String;
          
          debugPrint('[AnyWPEngine] Message received from JavaScript');
          debugPrint('[AnyWPEngine] Raw message: $messageJson');
          
          if (_onMessageCallback != null) {
            try {
              // Parse JSON message
              final message = jsonDecode(messageJson) as Map<String, dynamic>;
              debugPrint('[AnyWPEngine] Parsed message type: ${message['type']}');
              
              _onMessageCallback!(message);
              debugPrint('[AnyWPEngine] Message callback executed successfully');
            } catch (e) {
              debugPrint('[AnyWPEngine] ERROR: Failed to parse message JSON: $e');
            }
          } else {
            debugPrint('[AnyWPEngine] WARNING: Message callback is null!');
          }
        } else {
          debugPrint('[AnyWPEngine] Unknown method: ${call.method}');
        }
      } catch (e, stackTrace) {
        debugPrint('[AnyWPEngine] ERROR in method handler: $e');
        debugPrint('[AnyWPEngine] StackTrace: $stackTrace');
      }
    });
  }

  /// Initialize WebView2 as desktop wallpaper (single monitor / primary monitor)
  /// 
  /// Creates a wallpaper on the primary monitor with **Simple Mode** (mouse transparent).
  /// 
  /// Parameters:
  /// - [url]: The URL to load in the wallpaper WebView
  /// 
  /// Returns: `true` if successful, `false` otherwise
  /// 
  /// **Simple Mode (Default):**
  /// - Mouse clicks pass through wallpaper to desktop icons
  /// - Desktop remains fully functional
  /// - Perfect for passive animations and information displays
  /// 
  /// **Important Notes:**
  /// - For multi-monitor setups, use [initializeWallpaperOnMonitor]
  /// - Settings persist across system suspend/resume
  /// 
  /// **Example:**
  /// 
  /// ```dart
  /// // Simple wallpaper (desktop icons clickable)
  /// await AnyWPEngine.initializeWallpaper(
  ///   url: 'https://www.bing.com',
  /// );
  /// ```
  /// 
  /// See also:
  /// - [initializeWallpaperOnMonitor] - For multi-monitor setups
  /// - [stopWallpaper] - Stop the wallpaper
  static Future<bool> initializeWallpaper({
    required String url,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('initializeWallpaper', {
        'url': url,
        'enableMouseTransparent': true,  // Always use Simple Mode
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error initializing wallpaper: $e');
      return false;
    }
  }

  /// Stop and cleanup wallpaper
  static Future<bool> stopWallpaper() async {
    try {
      final result = await _channel.invokeMethod<bool>('stopWallpaper');
      return result ?? false;
    } catch (e) {
      debugPrint('Error stopping wallpaper: $e');
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
      debugPrint('Error navigating to URL: $e');
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
      debugPrint('Error getting monitors: $e');
      return [];
    }
  }

  /// Initialize WebView2 as desktop wallpaper on specific monitor
  ///
  /// Creates a wallpaper instance on the specified monitor with **Simple Mode** (mouse transparent).
  ///
  /// Parameters:
  /// - [url]: The URL to load in the wallpaper WebView
  /// - [monitorIndex]: The index of the target monitor (0-based)
  /// - [autoSave]: Whether to auto-save this configuration for recovery (default: `true`)
  /// - [allowedAccessPath]: Custom path for file access authorization (macOS only).
  ///   If null, uses global setting or defaults to Library directory.
  ///
  /// Returns: `true` if successful, `false` otherwise
  ///
  /// **Simple Mode (Default):**
  /// - Mouse clicks pass through wallpaper to desktop icons
  /// - Desktop remains fully functional
  /// - Perfect for passive animations and information displays
  ///
  /// **Important Notes:**
  /// - Settings persist across system suspend/resume (lock screen, sleep, etc.)
  /// - If Auto Recovery is enabled and `autoSave` is `true`, this configuration will be saved
  ///
  /// **Auto-Save Behavior (v2.4.0+):**
  /// - Set `autoSave: false` when frequently switching wallpapers (e.g., carousel, previews)
  /// - Set `autoSave: true` when user explicitly selects a wallpaper to persist
  /// - Use [saveCurrentWallpaperConfiguration] to manually save at the right time
  ///
  /// **File Access Control (macOS v2.6.4+):**
  /// - By default, file access is authorized to the entire Library directory
  /// - Use `allowedAccessPath` to specify a custom directory for file access
  /// - This allows HTML to load resources from subdirectories within the allowed path
  ///
  /// **Example 1: Simple wallpaper (auto-save):**
  ///
  /// ```dart
  /// // This configuration will be saved for recovery
  /// await AnyWPEngine.initializeWallpaperOnMonitor(
  ///   url: 'file:///path/to/animation.html',
  ///   monitorIndex: 0,
  /// );
  /// ```
  ///
  /// **Example 2: Carousel preview (don't auto-save):**
  ///
  /// ```dart
  /// // Initialize carousel HTML (don't save yet)
  /// await AnyWPEngine.initializeWallpaperOnMonitor(
  ///   url: 'file:///path/to/carousel.html',
  ///   monitorIndex: 0,
  ///   autoSave: false,  // Don't save on every carousel update
  /// );
  /// 
  /// // Later, when user confirms selection
  /// await AnyWPEngine.saveCurrentWallpaperConfiguration();
  /// ```
  ///
  /// **Example 3: Custom file access path (macOS):**
  ///
  /// ```dart
  /// // Load HTML from a custom cache directory with full access
  /// final libraryPath = await AnyWPEngine.getDefaultLibraryPath();
  /// await AnyWPEngine.initializeWallpaperOnMonitor(
  ///   url: 'file://$libraryPath/Application Support/MyApp/wallpaper.html',
  ///   monitorIndex: 0,
  ///   allowedAccessPath: '$libraryPath/Application Support/MyApp',
  /// );
  /// ```
  ///
  /// See also:
  /// - [getMonitors] - Get available monitors
  /// - [saveCurrentWallpaperConfiguration] - Manually save current configuration
  /// - [enableAutoRecovery] - Enable/disable auto-recovery feature
  /// - [setAllowedAccessPath] - Set global file access path (macOS)
  static Future<bool> initializeWallpaperOnMonitor({
    required String url,
    required int monitorIndex,
    bool autoSave = true,
    String? allowedAccessPath,
  }) async {
    try {
      final args = <String, dynamic>{
        'url': url,
        'monitorIndex': monitorIndex,
        'enableMouseTransparent': true,  // Always use Simple Mode
        'autoSave': autoSave,  // v2.4.0+ Control auto-save behavior
      };
      
      // Add allowedAccessPath if specified (macOS only)
      if (allowedAccessPath != null && allowedAccessPath.isNotEmpty) {
        args['allowedAccessPath'] = allowedAccessPath;
      }
      
      final result = await _channel.invokeMethod<bool>('initializeWallpaperOnMonitor', args);
      return result ?? false;
    } catch (e) {
      debugPrint('Error initializing wallpaper on monitor $monitorIndex: $e');
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
      debugPrint('Error stopping wallpaper on monitor $monitorIndex: $e');
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
      debugPrint('Error navigating to URL on monitor $monitorIndex: $e');
      return false;
    }
  }

  /// Initialize wallpaper on all monitors
  static Future<Map<int, bool>> initializeWallpaperOnAllMonitors({
    required String url,
  }) async {
    final monitors = await getMonitors();
    final results = <int, bool>{};
    
    for (final monitor in monitors) {
      final success = await initializeWallpaperOnMonitor(
        url: url,
        monitorIndex: monitor.index,
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
      debugPrint('Error pausing wallpaper: $e');
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
      debugPrint('Error resuming wallpaper: $e');
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
      debugPrint('Error setting auto power saving: $e');
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
      debugPrint('Error getting power state: $e');
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
      debugPrint('Error getting memory usage: $e');
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
      debugPrint('Error optimizing memory: $e');
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
      debugPrint('Warning: Idle timeout should be at least 60 seconds');
      seconds = 60;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setIdleTimeout', {
        'seconds': seconds,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error setting idle timeout: $e');
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
      debugPrint('Warning: Memory threshold should be at least 100 MB');
      thresholdMB = 100;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setMemoryThreshold', {
        'thresholdMB': thresholdMB,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error setting memory threshold: $e');
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
      debugPrint('Warning: Cleanup interval should be at least 10 minutes');
      minutes = 10;
    }
    
    try {
      final result = await _channel.invokeMethod<bool>('setCleanupInterval', {
        'minutes': minutes,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error setting cleanup interval: $e');
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
      debugPrint('Error getting configuration: $e');
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
      debugPrint('Error saving state: $e');
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
      debugPrint('Error loading state: $e');
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
      debugPrint('Error clearing state: $e');
      return false;
    }
  }

  /// Set application name for storage isolation
  /// 
  /// Sets a unique identifier for this application to isolate its storage
  /// from other applications using the AnyWP Engine. This should be called
  /// before any wallpaper initialization.
  /// 
  /// Storage path: %LOCALAPPDATA%\AnyWPEngine\[appName]\state.json
  /// 
  /// - [name]: Application identifier (alphanumeric, spaces converted to underscores)
  /// - Returns: true if successful
  /// 
  /// Example:
  /// ```dart
  /// // Set early in main()
  /// void main() async {
  ///   WidgetsFlutterBinding.ensureInitialized();
  ///   await AnyWPEngine.setApplicationName('MyAwesomeApp');
  ///   runApp(MyApp());
  /// }
  /// ```
  static Future<bool> setApplicationName(String name) async {
    try {
      final result = await _channel.invokeMethod<bool>('setApplicationName', {
        'name': name,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error setting application name: $e');
      return false;
    }
  }

  /// Get application-specific storage path
  /// 
  /// Returns the full path to this application's isolated storage directory.
  /// Useful for documentation or debugging purposes.
  /// 
  /// - Returns: The storage path (e.g., C:\Users\...\AppData\Local\AnyWPEngine\MyApp)
  /// 
  /// Example:
  /// ```dart
  /// final path = await AnyWPEngine.getStoragePath();
  /// print('State stored at: $path');
  /// ```
  static Future<String> getStoragePath() async {
    try {
      final result = await _channel.invokeMethod<String>('getStoragePath');
      return result ?? '';
    } catch (e) {
      debugPrint('Error getting storage path: $e');
      return '';
    }
  }

  /// 获取插件版本号（例如 `2.1.10`）。
  ///
  /// 当预编译包版本与项目依赖不一致时，可用于提示或诊断。
  static Future<String> getPluginVersion() async {
    try {
      final result = await _channel.invokeMethod<String>('getVersion');
      return result ?? '0.0.0';
    } catch (e) {
      debugPrint('Error getting plugin version: $e');
      return '0.0.0';
    }
  }
  
  /// 获取内置 Web SDK 版本号（例如 `2.1.10`）。
  ///
  /// 返回引擎内置集成的 JavaScript SDK (anywp_sdk.js) 的版本号。
  /// 由于 SDK 是在编译时嵌入的，版本号与插件版本保持一致。
  ///
  /// 用途：
  /// - 诊断 Web 壁纸兼容性问题
  /// - 在 UI 中显示完整版本信息
  /// - 文档生成和版本追踪
  ///
  /// 示例：
  /// ```dart
  /// final engineVersion = await AnyWPEngine.getPluginVersion();
  /// final sdkVersion = await AnyWPEngine.getSDKVersion();
  /// print('Engine: $engineVersion, SDK: $sdkVersion');
  /// ```
  static Future<String> getSDKVersion() async {
    try {
      final result = await _channel.invokeMethod<String>('getSDKVersion');
      return result ?? '0.0.0';
    } catch (e) {
      debugPrint('Error getting SDK version: $e');
      return '0.0.0';
    }
  }

  // ============================================================================
  // Debug & Logging Configuration (macOS only, v2.6.5+)
  // ============================================================================

  /// Set native logging level for macOS plugin
  ///
  /// Controls the verbosity of native (Objective-C) logging output.
  /// This helps reduce console spam in production builds.
  ///
  /// **Log Levels:**
  /// - `0`: Debug - All logs (verbose, development only)
  /// - `1`: Info - General information (default in Release)
  /// - `2`: Warn - Warnings only
  /// - `3`: Error - Errors only
  /// - `4`: None - Disable all logging
  ///
  /// **Platform Support:**
  /// - ✅ macOS: Fully supported (controls AWPLogger output)
  /// - ⚠️ Windows: Not yet implemented (returns false)
  ///
  /// **Default Behavior:**
  /// - Debug builds: Level 0 (Debug) - all logs enabled
  /// - Release builds: Level 1 (Info) - debug logs hidden
  ///
  /// **Example:**
  /// ```dart
  /// // Reduce logging noise in production
  /// await AnyWPEngine.setLogLevel(2);  // Warn and Error only
  /// 
  /// // Enable verbose logging for debugging
  /// await AnyWPEngine.setLogLevel(0);  // All logs
  /// 
  /// // Completely silence native logs
  /// await AnyWPEngine.setLogLevel(4);  // No logs
  /// ```
  ///
  /// - [level]: Log level (0-4)
  /// - Returns: `true` if successful, `false` if not supported or failed
  static Future<bool> setLogLevel(int level) async {
    try {
      final result = await _channel.invokeMethod<bool>('setLogLevel', {
        'level': level,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error setting log level: $e');
      return false;
    }
  }

  /// Get current native logging level (macOS only)
  ///
  /// Returns the current log level setting for the native plugin.
  ///
  /// **Platform Support:**
  /// - ✅ macOS: Returns current AWPLogger level (0-4)
  /// - ⚠️ Windows: Not yet implemented (returns -1)
  ///
  /// **Example:**
  /// ```dart
  /// final level = await AnyWPEngine.getLogLevel();
  /// print('Current log level: $level');  // 0 (Debug) to 4 (None)
  /// ```
  ///
  /// - Returns: Current log level (0-4), or -1 if not supported
  static Future<int> getLogLevel() async {
    try {
      final result = await _channel.invokeMethod<int>('getLogLevel');
      return result ?? -1;
    } catch (e) {
      debugPrint('Error getting log level: $e');
      return -1;
    }
  }

  // ============================================================================
  // Custom Scheme Support - File Encryption/Decryption (v2.1.10+)
  // ============================================================================

  /// Encrypt a file using XOR obfuscation (first 64 bytes)
  ///
  /// This method encrypts a file so it can be accessed via the `anywp://` protocol.
  /// The encryption is lightweight (XOR with key 0x5A on first 64 bytes) and designed
  /// for wallpaper content protection, not high-security scenarios.
  ///
  /// **Usage:**
  /// ```dart
  /// // Encrypt an image file
  /// bool success = await AnyWPEngine.encryptFile(
  ///   sourcePath: 'C:/my_wallpapers/image.jpg',
  ///   destPath: 'C:/my_cache/image.encrypted',
  /// );
  /// 
  /// if (success) {
  ///   // Use in wallpaper HTML: anywp://file?path=C:/my_cache/image.encrypted
  /// }
  /// ```
  ///
  /// **Parameters:**
  /// - `sourcePath`: Path to the source file (unencrypted)
  /// - `destPath`: Path where the encrypted file will be saved
  ///
  /// **Returns:** `true` if encryption succeeds, `false` otherwise
  ///
  /// **Note:** 
  /// - Developers can choose any custom cache path
  /// - The engine only handles encryption/decryption, not path management
  /// - Use absolute paths for both source and destination
  static Future<bool> encryptFile({
    required String sourcePath,
    required String destPath,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('encryptFile', {
        'sourcePath': sourcePath,
        'destPath': destPath,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error encrypting file: $e');
      return false;
    }
  }

  /// Decrypt a file (reverse of encryptFile)
  ///
  /// This method decrypts a file that was encrypted using `encryptFile`.
  /// Useful for extracting encrypted wallpaper content back to its original form.
  ///
  /// **Usage:**
  /// ```dart
  /// // Decrypt an encrypted image
  /// bool success = await AnyWPEngine.decryptFile(
  ///   encryptedPath: 'C:/my_cache/image.encrypted',
  ///   destPath: 'C:/output/image.jpg',
  /// );
  /// ```
  ///
  /// **Parameters:**
  /// - `encryptedPath`: Path to the encrypted file
  /// - `destPath`: Path where the decrypted file will be saved
  ///
  /// **Returns:** `true` if decryption succeeds, `false` otherwise
  ///
  /// **Note:**
  /// - XOR encryption is symmetric, so encryption and decryption use the same algorithm
  /// - This method is optional; the `anywp://` protocol handles decryption automatically
  static Future<bool> decryptFile({
    required String encryptedPath,
    required String destPath,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('decryptFile', {
        'encryptedPath': encryptedPath,
        'destPath': destPath,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error decrypting file: $e');
      return false;
    }
  }

  /// 检查插件版本是否兼容。
  ///
  /// [expectedPrefix] 默认为 `1.2.`，表示允许 1.2.x 的任何补丁版本。
  /// 如需更严格的判断，可传入完整版本前缀（例如 `1.2.1`）。
  static Future<bool> isCompatible({String expectedPrefix = '1.2.'}) async {
    final version = await getPluginVersion();
    if (version == '0.0.0') {
      return false;
    }
    return version.startsWith(expectedPrefix);
  }

  // ========== Auto Recovery APIs (v2.3.2+) ==========

  /// Enable or disable automatic wallpaper recovery
  /// 
  /// When enabled, the engine will automatically save wallpaper configurations
  /// and restore them after system events like Explorer restart, display changes, etc.
  /// 
  /// **Recommended Usage (Simple Mode):**
  /// ```dart
  /// void main() async {
  ///   WidgetsFlutterBinding.ensureInitialized();
  ///   
  ///   // Enable auto recovery (one-time setup)
  ///   await AnyWPEngine.enableAutoRecovery(true);
  ///   
  ///   runApp(MyApp());
  /// }
  /// 
  /// // Later, initialize wallpaper normally
  /// await AnyWPEngine.initializeWallpaperOnMonitor(
  ///   url: 'https://example.com',
  ///   monitorIndex: 0,
  /// );
  /// 
  /// // That's it! The engine will auto-recover after Explorer restart
  /// ```
  /// 
  /// **What Gets Saved:**
  /// - Wallpaper URL
  /// - Monitor index
  /// - Mouse transparency mode
  /// - All active wallpaper instances (multi-monitor support)
  /// 
  /// **When Recovery Triggers:**
  /// - Explorer restart (TaskManager kill, crash, etc.)
  /// - WorkerW window destroyed or invalidated
  /// - System display configuration changes
  /// 
  /// **Advantages:**
  /// - ✅ Zero maintenance - no code needed after `initializeWallpaper`
  /// - ✅ Multi-monitor support - all monitors auto-recovered
  /// - ✅ Smart delays - engine handles system stabilization
  /// - ✅ Persistent - survives app restarts (uses local storage)
  /// 
  /// **Parameters:**
  /// - [enabled]: `true` to enable, `false` to disable
  /// 
  /// **Returns:** `true` if successful, `false` otherwise
  /// 
  /// **Note:**
  /// - If disabled, you must manually handle recovery via `setOnMessageCallback`
  /// - See `docs/FOR_FLUTTER_DEVELOPERS.md` for manual recovery examples
  /// 
  /// **See also:**
  /// - [setOnMessageCallback] - Manual recovery mode (advanced users)
  static Future<bool> enableAutoRecovery(bool enabled) async {
    try {
      final result = await _channel.invokeMethod<bool>('enableAutoRecovery', {
        'enabled': enabled,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error enabling auto recovery: $e');
      return false;
    }
  }

  /// Check if auto recovery is currently enabled
  /// 
  /// Returns: `true` if auto recovery is enabled, `false` otherwise
  /// 
  /// Example:
  /// ```dart
  /// final isEnabled = await AnyWPEngine.isAutoRecoveryEnabled();
  /// print('Auto recovery: ${isEnabled ? "ON" : "OFF"}');
  /// ```
  static Future<bool> isAutoRecoveryEnabled() async {
    try {
      final result = await _channel.invokeMethod<bool>('isAutoRecoveryEnabled');
      return result ?? false;
    } catch (e) {
      debugPrint('Error checking auto recovery status: $e');
      return false;
    }
  }

  /// Manually save the current wallpaper configuration for recovery (v2.4.0+)
  /// 
  /// This method explicitly saves the current wallpaper state for auto-recovery.
  /// Use this when you've initialized a wallpaper with `autoSave: false` and 
  /// want to save the configuration at a specific point in time.
  /// 
  /// **When to Use:**
  /// - After user confirms wallpaper selection in a carousel/gallery
  /// - After user applies interactive wallpaper settings
  /// - After wallpaper state has been fully initialized
  /// - When you want explicit control over what gets saved
  /// 
  /// **What Gets Saved:**
  /// - Current wallpaper URL
  /// - Monitor index
  /// - Mouse transparency mode
  /// - All active wallpaper instances (multi-monitor)
  /// 
  /// **Parameters:**
  /// - [monitorIndex]: Optional monitor index to save (-1 = all monitors)
  /// 
  /// **Returns:** `true` if successful, `false` otherwise
  /// 
  /// **Example 1: Carousel workflow**
  /// 
  /// ```dart
  /// void main() async {
  ///   WidgetsFlutterBinding.ensureInitialized();
  ///   
  ///   // Enable auto recovery first
  ///   await AnyWPEngine.enableAutoRecovery(true);
  ///   
  ///   runApp(MyApp());
  /// }
  /// 
  /// class CarouselManager {
  ///   Future<void> startCarousel() async {
  ///     // Step 1: Initialize carousel HTML (don't auto-save)
  ///     await AnyWPEngine.initializeWallpaperOnMonitor(
  ///       url: 'file:///C:/carousel.html',
  ///       monitorIndex: 0,
  ///       autoSave: false,  // Don't save on every carousel change
  ///     );
  ///     
  ///     // Step 2: Send initial images
  ///     await AnyWPEngine.sendMessage({
  ///       'type': 'updateCarousel',
  ///       'data': {'images': [...], 'interval': 60000},
  ///     });
  ///     
  ///     // Step 3: Save configuration now
  ///     await AnyWPEngine.saveCurrentWallpaperConfiguration();
  ///     print('✅ Carousel configuration saved for recovery');
  ///   }
  ///   
  ///   Future<void> nextWallpaper() async {
  ///     // Just switch to next image, don't re-save
  ///     await AnyWPEngine.sendMessage({'type': 'next'});
  ///   }
  /// }
  /// ```
  /// 
  /// **Example 2: Interactive wallpaper with settings**
  /// 
  /// ```dart
  /// class SettingsManager {
  ///   Future<void> initWallpaper() async {
  ///     // Initialize with default settings (don't save yet)
  ///     await AnyWPEngine.initializeWallpaperOnMonitor(
  ///       url: 'file:///C:/interactive.html',
  ///       monitorIndex: 0,
  ///       autoSave: false,
  ///     );
  ///   }
  ///   
  ///   Future<void> applySettings(Map<String, dynamic> settings) async {
  ///     // Send settings to wallpaper
  ///     await AnyWPEngine.sendMessage({
  ///       'type': 'updateSettings',
  ///       'data': settings,
  ///     });
  ///     
  ///     // Wait for wallpaper to apply settings
  ///     await Future.delayed(Duration(milliseconds: 500));
  ///     
  ///     // NOW save the configuration
  ///     final success = await AnyWPEngine.saveCurrentWallpaperConfiguration();
  ///     if (success) {
  ///       print('✅ Settings saved and will be restored after Explorer restart');
  ///     }
  ///   }
  /// }
  /// ```
  /// 
  /// **Example 3: Multi-monitor setup**
  /// 
  /// ```dart
  /// // Save specific monitor only
  /// await AnyWPEngine.saveCurrentWallpaperConfiguration(monitorIndex: 0);
  /// 
  /// // Save all monitors
  /// await AnyWPEngine.saveCurrentWallpaperConfiguration(monitorIndex: -1);
  /// // or simply:
  /// await AnyWPEngine.saveCurrentWallpaperConfiguration();
  /// ```
  /// 
  /// **Note:**
  /// - Auto Recovery must be enabled via [enableAutoRecovery] first
  /// - If Auto Recovery is disabled, this method does nothing
  /// - For simple static wallpapers, just use `autoSave: true` (default)
  /// 
  /// See also:
  /// - [enableAutoRecovery] - Enable auto-recovery feature
  /// - [initializeWallpaperOnMonitor] - Initialize wallpaper with auto-save control
  static Future<bool> saveCurrentWallpaperConfiguration({int monitorIndex = -1}) async {
    try {
      final result = await _channel.invokeMethod<bool>('saveWallpaperConfiguration', {
        'monitorIndex': monitorIndex,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('Error saving wallpaper configuration: $e');
      return false;
    }
  }

  // ========== Bidirectional Communication APIs ==========

  /// Send message to JavaScript wallpaper
  /// 
  /// Sends a message to the JavaScript code running in the wallpaper.
  /// This enables bidirectional communication between Flutter and the wallpaper.
  /// 
  /// Parameters:
  /// - [message]: The message to send (will be JSON-encoded)
  /// - [monitorIndex]: Optional monitor index (-1 or null = all monitors)
  /// 
  /// Message format (recommended):
  /// ```dart
  /// {
  ///   'id': 'unique-message-id',
  ///   'type': 'message_type',
  ///   'timestamp': DateTime.now().millisecondsSinceEpoch,
  ///   'data': { ... }
  /// }
  /// ```
  /// 
  /// Common message types (examples):
  /// - `updateCarousel`: Update wallpaper carousel
  /// - `addToCarousel`: Add item to carousel
  /// - `play`/`pause`/`stop`: Control playback
  /// - `setInterval`: Change carousel interval
  /// - Custom types defined by your wallpaper
  /// 
  /// Returns: `true` if message sent successfully, `false` otherwise
  /// 
  /// Example:
  /// ```dart
  /// // Send message to all monitors
  /// await AnyWPEngine.sendMessage(
  ///   message: {
  ///     'id': 'msg-001',
  ///     'type': 'updateCarousel',
  ///     'timestamp': DateTime.now().millisecondsSinceEpoch,
  ///     'data': {
  ///       'images': [
  ///         'https://example.com/img1.jpg',
  ///         'https://example.com/img2.jpg',
  ///       ],
  ///       'interval': 30000,
  ///       'transition': 'fade',
  ///     },
  ///   },
  /// );
  /// 
  /// // Send message to specific monitor
  /// await AnyWPEngine.sendMessage(
  ///   message: {
  ///     'type': 'play',
  ///     'data': {},
  ///   },
  ///   monitorIndex: 0,
  /// );
  /// 
  /// // Listen for responses
  /// AnyWPEngine.setOnMessageCallback((message) {
  ///   if (message['type'] == 'carouselStateChanged') {
  ///     print('Carousel updated: ${message['data']}');
  ///   }
  /// });
  /// ```
  /// 
  /// See also:
  /// - [setOnMessageCallback] - Receive messages from JavaScript
  static Future<bool> sendMessage({
    required Map<String, dynamic> message,
    int? monitorIndex,
  }) async {
    try {
      // Convert message to JSON string
      final messageJson = jsonEncode(message);
      
      // Build arguments
      final args = <String, dynamic>{
        'message': messageJson,
      };
      
      // Add monitor index if specified
      if (monitorIndex != null && monitorIndex >= 0) {
        args['monitorIndex'] = monitorIndex;
      }
      
      final result = await _channel.invokeMethod<bool>('sendMessage', args);
      return result ?? false;
    } catch (e) {
      debugPrint('Error sending message: $e');
      return false;
    }
  }

  // ========== Bundle Resources (macOS) ==========

  /// Get bundle resource path (macOS only)
  /// 
  /// Returns the absolute path to a resource bundled with the macOS app.
  /// This is useful for loading local HTML test pages without sandbox restrictions.
  /// 
  /// Parameters:
  /// - [resourceName]: Name of the resource (without extension)
  /// - [type]: Resource type/extension (default: 'html')
  /// 
  /// Returns: Absolute file path to the resource, or null if not found
  /// 
  /// Example:
  /// ```dart
  /// // Get path to test page
  /// final testPath = await AnyWPEngine.getBundleResourcePath(
  ///   resourceName: 'test_simple',
  ///   type: 'html',
  /// );
  /// 
  /// if (testPath != null) {
  ///   // Load the bundled test page
  ///   await AnyWPEngine.initializeWallpaper(url: 'file://$testPath');
  /// }
  /// ```
  /// 
  /// Available bundled test pages:
  /// - test_simple.html
  /// - test_api.html
  /// - test_bidirectional.html
  /// - test_basic_click.html
  static Future<String?> getBundleResourcePath({
    required String resourceName,
    String type = 'html',
  }) async {
    try {
      final result = await _channel.invokeMethod<String>('getBundleResourcePath', {
        'resourceName': resourceName,
        'type': type,
      });
      return result;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to get bundle resource: $e');
      return null;
    }
  }

  // ========== Interactive Mode (Cross-Platform) ==========

  /// Set interactive mode for wallpaper
  /// 
  /// Controls whether the wallpaper can capture mouse events or be transparent.
  /// 
  /// **Interactive Mode** (interactive = true):
  /// - Mouse events are captured by the wallpaper
  /// - Users can click, drag, and interact with wallpaper content
  /// - Desktop icons may be harder to click (wallpaper is above them)
  /// 
  /// **Simple Mode** (interactive = false, default):
  /// - Mouse events pass through to desktop
  /// - Wallpaper is display-only
  /// - Desktop icons remain fully clickable
  /// 
  /// Parameters:
  /// - [monitorIndex]: Index of the monitor to change (0 for primary)
  /// - [interactive]: true for interactive mode, false for simple mode
  /// 
  /// Returns: true if successful, false otherwise
  /// 
  /// Platform support:
  /// - ✅ Windows (full support)
  /// - ✅ macOS (full support)
  /// 
  /// Example:
  /// ```dart
  /// // Enable interactive mode for gaming wallpaper
  /// await AnyWPEngine.setInteractiveMode(
  ///   monitorIndex: 0,
  ///   interactive: true,
  /// );
  /// 
  /// // Disable interactive mode for video wallpaper
  /// await AnyWPEngine.setInteractiveMode(
  ///   monitorIndex: 0,
  ///   interactive: false,
  /// );
  /// ```
  /// 
  /// Web SDK notification:
  /// When interactive mode changes, the Web SDK will receive a notification:
  /// ```javascript
  /// window.addEventListener('AnyWP:interactiveMode', (event) => {
  ///   console.log('Interactive mode:', event.detail.interactive);
  /// });
  /// ```
  static Future<bool> setInteractiveMode({
    required int monitorIndex,
    required bool interactive,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('setInteractiveMode', {
        'monitorIndex': monitorIndex,
        'interactive': interactive,
      });
      return result ?? false;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to set interactive mode: $e');
      return false;
    }
  }

  // ========== Local File Server (macOS) ==========

  /// Start Local File Server (macOS)
  /// 
  /// Starts a local file server to serve files from a directory.
  /// This solves CORS issues when loading local resources.
  /// 
  /// Parameters:
  /// - [rootPath]: Root directory to serve files from
  /// 
  /// Returns: Map with server info or null if failed
  ///   - success: true if started successfully
  ///   - baseURL: Base URL for accessing files (localfile://)
  ///   - rootPath: Root directory path
  ///   - error: Error message if failed
  /// 
  /// Platform support:
  /// - ❌ Windows (not needed, use LocalFileServer class in C++)
  /// - ✅ macOS (NSURLProtocol-based)
  /// 
  /// Example:
  /// ```dart
  /// final result = await AnyWPEngine.startFileServer(
  ///   rootPath: '/path/to/wallpaper/files',
  /// );
  /// 
  /// if (result?['success'] == true) {
  ///   String baseURL = result!['baseURL'];
  ///   // Now you can load files like: localfile:///index.html
  ///   await AnyWPEngine.initializeWallpaper(url: '${baseURL}/index.html');
  /// }
  /// ```
  /// 
  /// Notes:
  /// - Uses custom URL scheme: localfile://
  /// - Automatically adds CORS headers
  /// - Detects MIME types automatically
  /// - No need to specify port (scheme-based)
  static Future<Map<String, dynamic>?> startFileServer({
    required String rootPath,
  }) async {
    try {
      final result = await _channel.invokeMethod('startFileServer', {
        'rootPath': rootPath,
      });
      if (result is Map) {
        return Map<String, dynamic>.from(result);
      }
      return null;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to start file server: $e');
      return null;
    }
  }

  /// Stop Local File Server (macOS)
  /// 
  /// Stops the running local file server.
  /// 
  /// Returns: true if stopped successfully
  /// 
  /// Platform support:
  /// - ❌ Windows (not needed)
  /// - ✅ macOS
  /// 
  /// Example:
  /// ```dart
  /// await AnyWPEngine.stopFileServer();
  /// ```
  static Future<bool> stopFileServer() async {
    try {
      final result = await _channel.invokeMethod<bool>('stopFileServer');
      return result ?? false;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to stop file server: $e');
      return false;
    }
  }

  /// Check if File Server is Running (macOS)
  /// 
  /// Checks if the local file server is currently running.
  /// 
  /// Returns: Map with server status or null if not running
  ///   - running: true if server is running
  ///   - baseURL: Base URL if running
  ///   - rootPath: Root directory if running
  /// 
  /// Platform support:
  /// - ❌ Windows (not needed)
  /// - ✅ macOS
  /// 
  /// Example:
  /// ```dart
  /// final status = await AnyWPEngine.isFileServerRunning();
  /// if (status?['running'] == true) {
  ///   print('Server is running: ${status!['baseURL']}');
  /// }
  /// ```
  static Future<Map<String, dynamic>?> isFileServerRunning() async {
    try {
      final result = await _channel.invokeMethod('isFileServerRunning');
      if (result is Map) {
        return Map<String, dynamic>.from(result);
      }
      return null;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to check file server status: $e');
      return null;
    }
  }

  // ========== File Access Control (macOS v2.6.4+) ==========

  /// Set global allowed access path for file loading (macOS)
  /// 
  /// Sets a global path that will be used as the file access authorization
  /// base for all wallpaper instances. This allows HTML files to load
  /// resources from any subdirectory within the allowed path.
  /// 
  /// **Use Case:**
  /// When loading local HTML wallpapers that reference other files (images,
  /// CSS, JavaScript) from different subdirectories, you need to authorize
  /// a parent directory that contains all required resources.
  /// 
  /// **Default Behavior:**
  /// If not set, the engine defaults to authorizing the Library directory
  /// (~/Library), which allows access to Application Support, Caches, etc.
  /// 
  /// Parameters:
  /// - [path]: The directory path to authorize. Pass null or empty to
  ///   reset to default (Library directory).
  /// 
  /// Returns: Map with result info
  ///   - success: true if successful
  ///   - currentPath: The current allowed access path
  /// 
  /// Platform support:
  /// - ❌ Windows (not needed, WebView2 handles this differently)
  /// - ✅ macOS (WKWebView file access control)
  /// 
  /// Example:
  /// ```dart
  /// // Set allowed access to Application Support directory
  /// final appSupportPath = await AnyWPEngine.getApplicationSupportPath();
  /// final result = await AnyWPEngine.setAllowedAccessPath(appSupportPath);
  /// 
  /// if (result?['success'] == true) {
  ///   print('Access path set to: ${result!['currentPath']}');
  ///   
  ///   // Now load HTML that references files in Application Support
  ///   await AnyWPEngine.initializeWallpaperOnMonitor(
  ///     url: 'file://$appSupportPath/MyApp/wallpaper.html',
  ///     monitorIndex: 0,
  ///   );
  /// }
  /// ```
  /// 
  /// **Recommended Setup for Wallpaper Apps:**
  /// ```dart
  /// void main() async {
  ///   WidgetsFlutterBinding.ensureInitialized();
  ///   
  ///   // Set access path at app startup
  ///   final libraryPath = await AnyWPEngine.getDefaultLibraryPath();
  ///   await AnyWPEngine.setAllowedAccessPath(libraryPath);
  ///   
  ///   runApp(MyApp());
  /// }
  /// ```
  /// 
  /// See also:
  /// - [getDefaultLibraryPath] - Get Library directory path
  /// - [getApplicationSupportPath] - Get Application Support path
  /// - [initializeWallpaperOnMonitor] - Initialize with custom access path
  static Future<Map<String, dynamic>?> setAllowedAccessPath(String? path) async {
    try {
      final result = await _channel.invokeMethod('setAllowedAccessPath', {
        'path': path ?? '',
      });
      if (result is Map) {
        return Map<String, dynamic>.from(result);
      }
      return null;
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to set allowed access path: $e');
      return null;
    }
  }

  /// Get the default Library directory path (macOS)
  /// 
  /// Returns the path to the user's Library directory (~/Library).
  /// This is the default base path for file access authorization.
  /// 
  /// Returns: The Library directory path, or empty string on error
  /// 
  /// Platform support:
  /// - ❌ Windows (returns empty string)
  /// - ✅ macOS
  /// 
  /// Example:
  /// ```dart
  /// final libraryPath = await AnyWPEngine.getDefaultLibraryPath();
  /// print('Library path: $libraryPath');
  /// // Output: /Users/username/Library
  /// ```
  static Future<String> getDefaultLibraryPath() async {
    try {
      final result = await _channel.invokeMethod<String>('getDefaultLibraryPath');
      return result ?? '';
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to get default library path: $e');
      return '';
    }
  }

  /// Get the Application Support directory path (macOS)
  /// 
  /// Returns the path to the Application Support directory
  /// (~/Library/Application Support). This is commonly used to store
  /// app-specific data, cache, and resources.
  /// 
  /// Returns: The Application Support path, or empty string on error
  /// 
  /// Platform support:
  /// - ❌ Windows (returns empty string)
  /// - ✅ macOS
  /// 
  /// Example:
  /// ```dart
  /// final appSupportPath = await AnyWPEngine.getApplicationSupportPath();
  /// print('App Support path: $appSupportPath');
  /// // Output: /Users/username/Library/Application Support
  /// 
  /// // Store wallpapers in your app's directory
  /// final myAppPath = '$appSupportPath/MyWallpaperApp';
  /// ```
  static Future<String> getApplicationSupportPath() async {
    try {
      final result = await _channel.invokeMethod<String>('getApplicationSupportPath');
      return result ?? '';
    } catch (e) {
      debugPrint('[AnyWPEngine] Failed to get application support path: $e');
      return '';
    }
  }

}
