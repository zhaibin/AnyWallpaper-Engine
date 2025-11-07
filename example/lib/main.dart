import 'dart:async';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'package:window_manager/window_manager.dart';

/// 显示器配置（用于记忆拔掉前的状态）
class MonitorConfig {
  final String url;
  final bool wasRunning;
  final DateTime lastSeen;
  
  MonitorConfig({
    required this.url,
    required this.wasRunning,
    required this.lastSeen,
  });
}

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // Initialize window manager to prevent window position jumping
  await windowManager.ensureInitialized();
  
  // Set application name for storage isolation
  await AnyWPEngine.setApplicationName('AnyWallpaperDemo');
  
  // Print storage path for verification
  final storagePath = await AnyWPEngine.getStoragePath();
  print('[APP] Storage path: $storagePath');
  
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> with WindowListener {
  // Multi-monitor support
  List<MonitorInfo> _monitors = [];
  Map<int, bool> _monitorWallpapers = {};  // Track which monitors have wallpapers
  Map<int, TextEditingController> _monitorUrlControllers = {};  // Each monitor has its own URL
  Map<int, bool> _monitorLoading = {};  // Track loading state for each monitor
  bool _allMonitorsLoading = false;  // Track "Start All" / "Stop All" loading state
  bool _mouseTransparent = true;  // Default: wallpaper mode (transparent)
  Timer? _monitorCheckTimer;  // Timer for polling monitor changes
  bool _isHandlingMonitorChange = false;  // Prevent overlapping monitor change handling
  
  // Monitor configuration memory - preserves settings when monitors are unplugged
  Map<String, MonitorConfig> _monitorConfigMemory = {};  // Key: deviceName (e.g., \\.\DISPLAY2)
  
  // Window position memory - prevents jumping when monitors change
  Offset? _savedWindowPosition;
  
  // Power saving & optimization
  String _powerState = 'Loading...';
  int _memoryUsageMB = 0;
  bool _autoPowerSaving = true;
  
  // Quick test pages
  final List<Map<String, String>> _testPages = [
    {'name': 'Simple', 'file': 'test_simple.html', 'icon': '🎨'},
    {'name': 'Draggable', 'file': 'test_draggable.html', 'icon': '🖱️'},
    {'name': 'Drag Debug', 'file': 'test_drag_debug.html', 'icon': '🔍'},
    {'name': 'API Test', 'file': 'test_api.html', 'icon': '⚙️'},
    {'name': 'Click Test', 'file': 'test_basic_click.html', 'icon': '👆'},
    {'name': 'Visibility', 'file': 'test_visibility.html', 'icon': '👁️'},
    {'name': 'React', 'file': 'test_react.html', 'icon': '⚛️'},
    {'name': 'Vue', 'file': 'test_vue.html', 'icon': '💚'},
    {'name': 'iFrame Ads', 'file': 'test_iframe_ads.html', 'icon': '📺'},
  ];

  @override
  void initState() {
    super.initState();
    
    // Register window listener to save/restore position
    windowManager.addListener(this);
    
    _loadMonitors();
    
    // Start polling for monitor changes (every 3 seconds to reduce UI thrashing)
    // Note: Direct InvokeMethod callback causes crashes, so we use polling instead
    _monitorCheckTimer = Timer.periodic(Duration(seconds: 3), (timer) {
      _checkMonitorChanges();
    });
    
    print('[APP] Monitor polling started (every 3 seconds)');
  }
  
  Future<void> _handleMonitorChange() async {
    print('[APP] Handling monitor change - detecting changes and auto-applying...');
    
    try {
       // Get old monitor configuration BEFORE any changes
       final oldMonitors = List<MonitorInfo>.from(_monitors);
       final oldIndices = oldMonitors.map((m) => m.index).toSet();
       
       // Get new monitor list (without updating state yet)
       final newMonitors = await AnyWPEngine.getMonitors();
       final newIndices = newMonitors.map((m) => m.index).toSet();
       
       // Detect changes
       final addedIndices = newIndices.difference(oldIndices);
       final removedIndices = oldIndices.difference(newIndices);
       
       // IMPORTANT: Save configuration BEFORE _loadMonitors() clears controllers
       if (removedIndices.isNotEmpty) {
         print('[APP] Detected removed monitors: $removedIndices');
         for (final removedIndex in removedIndices) {
           // Find monitor in OLD list
           final removedMonitor = oldMonitors.firstWhere(
             (m) => m.index == removedIndex,
             orElse: () => oldMonitors.first,
           );
           
           // Save BEFORE controllers are disposed
           if (_monitorUrlControllers.containsKey(removedIndex)) {
             final url = _monitorUrlControllers[removedIndex]!.text.trim();
             final wasRunning = _monitorWallpapers[removedIndex] == true;
             
             if (url.isNotEmpty) {
               _monitorConfigMemory[removedMonitor.deviceName] = MonitorConfig(
                 url: url,
                 wasRunning: wasRunning,
                 lastSeen: DateTime.now(),
               );
               print('[APP] 💾 Saved config for ${removedMonitor.deviceName}:');
               print('[APP]    URL: $url');
               print('[APP]    Running: $wasRunning');
             }
           }
         }
       }
       
       // Now refresh monitor list (this will clean up controllers)
       await _loadMonitors();
      
       if (addedIndices.isNotEmpty) {
        print('[APP] Detected new monitors: $addedIndices');
        
        int successCount = 0;
        Map<int, bool> updatedWallpapers = {};
        
        // Process each newly added monitor
        for (final index in addedIndices) {
          final newMonitor = _monitors.firstWhere((m) => m.index == index);
          String? urlToUse;
          bool shouldStart = false;
          
          // Strategy 1: Check if this monitor has saved configuration (was unplugged before)
          if (_monitorConfigMemory.containsKey(newMonitor.deviceName)) {
            final savedConfig = _monitorConfigMemory[newMonitor.deviceName]!;
            
            print('[APP] 📂 Found saved config for ${newMonitor.deviceName}:');
            print('[APP]    Saved URL: ${savedConfig.url}');
            print('[APP]    Was Running: ${savedConfig.wasRunning}');
            print('[APP]    Last Seen: ${savedConfig.lastSeen}');
            
            // Only restore if it was running before being unplugged
            if (savedConfig.wasRunning) {
              urlToUse = savedConfig.url;
              shouldStart = true;
              print('[APP] ✅ Will RESTORE previous wallpaper on monitor $index');
            } else {
              print('[APP] ⏸️ Wallpaper was NOT running, will not auto-start');
            }
          } else {
            print('[APP] ❌ No saved config found for ${newMonitor.deviceName}');
          }
          
          // Strategy 2: If no saved config, check if there's a running wallpaper on any other monitor
          if (!shouldStart) {
            print('[APP] 🔍 No saved config or wasn\'t running, checking for active wallpapers...');
            for (final entry in _monitorWallpapers.entries) {
              if (entry.value == true && _monitorUrlControllers.containsKey(entry.key)) {
                final activeUrl = _monitorUrlControllers[entry.key]!.text.trim();
                if (activeUrl.isNotEmpty) {
                  urlToUse = activeUrl;
                  shouldStart = true;
                  print('[APP] 🔄 Using active wallpaper URL from monitor ${entry.key}: $urlToUse');
                  break;
                }
              }
            }
            if (!shouldStart) {
              print('[APP] ⚠️ No active wallpaper found to copy');
            }
          }
          
          // Apply wallpaper if we have a URL
          if (shouldStart && urlToUse != null && urlToUse.isNotEmpty) {
            print('[APP] ▶️ Starting wallpaper on monitor $index');
            print('[APP]    Device: ${newMonitor.deviceName}');
            print('[APP]    URL: $urlToUse');
            
            // Set URL in controller
            if (_monitorUrlControllers[index] != null) {
              _monitorUrlControllers[index]!.text = urlToUse;
              print('[APP]    Controller updated with URL');
            }
            
            // Try to start wallpaper
            bool success = await AnyWPEngine.initializeWallpaperOnMonitor(
              url: urlToUse,
              monitorIndex: index,
              enableMouseTransparent: _mouseTransparent,
            );
            
            print('[APP]    Result: ${success ? "✅ SUCCESS" : "❌ FAILED"}');
            
            // If failed, try fallback to primary monitor's URL
            if (!success && urlToUse != _getPrimaryMonitorUrl()) {
              print('[APP] Failed with URL: $urlToUse, trying primary monitor URL as fallback');
              final primaryUrl = _getPrimaryMonitorUrl();
              
              if (primaryUrl != null && primaryUrl.isNotEmpty) {
                print('[APP] Fallback to primary URL: $primaryUrl');
                
                if (_monitorUrlControllers[index] != null) {
                  _monitorUrlControllers[index]!.text = primaryUrl;
                }
                
                success = await AnyWPEngine.initializeWallpaperOnMonitor(
                  url: primaryUrl,
                  monitorIndex: index,
                  enableMouseTransparent: _mouseTransparent,
                );
                
                if (success) {
                  print('[APP] Fallback succeeded on monitor $index');
                }
              }
            }
            
            if (success) {
              updatedWallpapers[index] = true;
              successCount++;
              print('[APP] Successfully started wallpaper on monitor $index');
            } else {
              print('[APP] Failed to start wallpaper on monitor $index (even after fallback)');
            }
          } else {
            print('[APP] No URL to apply on monitor $index (no saved config and no active wallpaper)');
          }
        }
        
        // Update state once after all operations
        if (updatedWallpapers.isNotEmpty && mounted) {
          setState(() {
            _monitorWallpapers.addAll(updatedWallpapers);
          });
        }
        
        // Show result message
        if (mounted) {
          if (successCount > 0) {
            _showMessage('Auto-started wallpaper on $successCount new monitor(s)');
          } else if (addedIndices.isNotEmpty) {
            _showMessage('New monitor detected - no wallpaper to apply');
          }
        }
      } else {
        // No new monitors, just a configuration change
        if (mounted) {
          _showMessage('Display configuration changed');
        }
      }
      
      print('[APP] Monitor change handled successfully');
    } catch (e, stackTrace) {
      print('[APP] ERROR in _handleMonitorChange: $e');
      print('[APP] StackTrace: $stackTrace');
      
      if (mounted) {
        _showMessage('Error handling monitor change: $e');
      }
    }
  }
  
  @override
  void dispose() {
    _monitorCheckTimer?.cancel();
    windowManager.removeListener(this);
    for (final controller in _monitorUrlControllers.values) {
      controller.dispose();
    }
    super.dispose();
  }
  
  // WindowListener callbacks - save window position before monitor change
  @override
  void onWindowMoved() async {
    final position = await windowManager.getPosition();
    _savedWindowPosition = position;
    print('[APP] Window position saved: $position');
  }
  
  @override
  void onWindowEvent(String eventName) {
    // Monitor changes can trigger window repositioning by Windows
    // We'll restore position after monitor detection
    print('[APP] Window event: $eventName');
  }
  
  // Check for monitor changes by polling
  Future<void> _checkMonitorChanges() async {
    // Prevent overlapping calls
    if (_isHandlingMonitorChange) {
      return;
    }
    
    try {
      final monitors = await AnyWPEngine.getMonitors();
      
      // Check if monitor count changed
      if (monitors.length != _monitors.length) {
        print('[APP] Monitor count changed: ${_monitors.length} -> ${monitors.length}');
        
        // Save current window position before handling monitor change
        try {
          final currentPosition = await windowManager.getPosition();
          _savedWindowPosition = currentPosition;
          print('[APP] Saved window position before monitor change: $currentPosition');
        } catch (e) {
          print('[APP] Failed to save window position: $e');
        }
        
        _isHandlingMonitorChange = true;
        try {
          await _handleMonitorChange();
          
          // Restore window position after a short delay (let Windows finish its repositioning)
          Future.delayed(Duration(milliseconds: 500), () async {
            if (_savedWindowPosition != null) {
              try {
                await windowManager.setPosition(_savedWindowPosition!);
                print('[APP] Restored window position to: $_savedWindowPosition');
              } catch (e) {
                print('[APP] Failed to restore window position: $e');
              }
            }
          });
        } finally {
          _isHandlingMonitorChange = false;
        }
      }
    } catch (e) {
      // Silently ignore errors during polling to avoid spam
      _isHandlingMonitorChange = false;
    }
  }
  
  Future<void> _loadMonitors() async {
    print('[APP] Loading monitors...');
    
    try {
      final monitors = await AnyWPEngine.getMonitors();
      
      print('[APP] Found ${monitors.length} monitor(s):');
      for (final monitor in monitors) {
        print('[APP]   $monitor');
      }
      
      if (!mounted) {
        print('[APP] Widget not mounted, skipping setState');
        return;
      }
      
      setState(() {
      // Get current monitor indices before update
      final oldIndices = _monitors.map((m) => m.index).toSet();
      final newIndices = monitors.map((m) => m.index).toSet();
      
      // Log changes
      final removed = oldIndices.difference(newIndices);
      final added = newIndices.difference(oldIndices);
      
      if (removed.isNotEmpty) {
        print('[APP] Removed monitors: $removed');
      }
      if (added.isNotEmpty) {
        print('[APP] Added monitors: $added');
      }
      
      _monitors = monitors;
      
      // Initialize wallpaper tracking, loading state and URL controllers for NEW monitors only
      for (final monitor in monitors) {
        if (!_monitorWallpapers.containsKey(monitor.index)) {
          print('[APP] Initializing UI for new monitor ${monitor.index}');
          _monitorWallpapers[monitor.index] = false;
          _monitorLoading[monitor.index] = false;
          _monitorUrlControllers[monitor.index] = TextEditingController(
            text: 'file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_draggable.html',
          );
        }
      }
      
      // Clean up controllers for removed monitors
      final currentIndices = monitors.map((m) => m.index).toSet();
      final keysToRemove = _monitorUrlControllers.keys.where((k) => !currentIndices.contains(k)).toList();
      
      if (keysToRemove.isNotEmpty) {
        print('[APP] Cleaning up UI for removed monitors: $keysToRemove');
      }
      
      for (final key in keysToRemove) {
        _monitorUrlControllers[key]?.dispose();
        _monitorUrlControllers.remove(key);
        _monitorWallpapers.remove(key);
        _monitorLoading.remove(key);
        print('[APP] Removed monitor $key from UI');
      }
      });
      
      print('[APP] Monitor list updated. Total: ${_monitors.length}');
    } catch (e, stackTrace) {
      print('[APP] ERROR in _loadMonitors: $e');
      print('[APP] StackTrace: $stackTrace');
    }
  }


  Future<void> _startWallpaperOnMonitor(int monitorIndex) async {
    final controller = _monitorUrlControllers[monitorIndex];
    if (controller == null) return;
    
    final url = controller.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL for monitor $monitorIndex');
      return;
    }

    // Set loading state
    setState(() {
      _monitorLoading[monitorIndex] = true;
    });

    print('[APP] Starting wallpaper on monitor $monitorIndex with URL: $url');
    
    try {
      final success = await AnyWPEngine.initializeWallpaperOnMonitor(
        url: url,
        monitorIndex: monitorIndex,
        enableMouseTransparent: _mouseTransparent,
      );

      setState(() {
        _monitorWallpapers[monitorIndex] = success;
        _monitorLoading[monitorIndex] = false;
      });

      if (success) {
        _showMessage('Wallpaper started on monitor $monitorIndex');
      } else {
        _showMessage('Failed to start wallpaper on monitor $monitorIndex');
      }
    } catch (e) {
      setState(() {
        _monitorLoading[monitorIndex] = false;
      });
      _showMessage('Error starting wallpaper on monitor $monitorIndex: $e');
    }
  }

  Future<void> _stopWallpaperOnMonitor(int monitorIndex) async {
    // Set loading state
    setState(() {
      _monitorLoading[monitorIndex] = true;
    });

    print('[APP] Stopping wallpaper on monitor $monitorIndex');
    
    try {
      final success = await AnyWPEngine.stopWallpaperOnMonitor(monitorIndex);

      setState(() {
        _monitorWallpapers[monitorIndex] = false;
        _monitorLoading[monitorIndex] = false;
      });

      if (success) {
        _showMessage('Wallpaper stopped on monitor $monitorIndex');
      } else {
        _showMessage('Failed to stop wallpaper on monitor $monitorIndex');
      }
    } catch (e) {
      setState(() {
        _monitorLoading[monitorIndex] = false;
      });
      _showMessage('Error stopping wallpaper on monitor $monitorIndex: $e');
    }
  }

  Future<void> _startWallpaperOnAllMonitors() async {
    // Set loading state
    setState(() {
      _allMonitorsLoading = true;
      for (final monitor in _monitors) {
        _monitorLoading[monitor.index] = true;
      }
    });

    print('[APP] Starting wallpaper on all monitors with individual URLs');
    
    int successCount = 0;
    try {
      for (final monitor in _monitors) {
        final controller = _monitorUrlControllers[monitor.index];
        if (controller == null) continue;
        
        final url = controller.text.trim();
        if (url.isEmpty) continue;
        
        final success = await AnyWPEngine.initializeWallpaperOnMonitor(
          url: url,
          monitorIndex: monitor.index,
          enableMouseTransparent: _mouseTransparent,
        );
        
        setState(() {
          _monitorWallpapers[monitor.index] = success;
        });
        
        if (success) successCount++;
      }
      
      _showMessage('Started wallpaper on $successCount/${_monitors.length} monitor(s)');
    } finally {
      // Clear loading state
      setState(() {
        _allMonitorsLoading = false;
        for (final monitor in _monitors) {
          _monitorLoading[monitor.index] = false;
        }
      });
    }
  }

  Future<void> _stopWallpaperOnAllMonitors() async {
    // Set loading state
    setState(() {
      _allMonitorsLoading = true;
      for (final monitor in _monitors) {
        _monitorLoading[monitor.index] = true;
      }
    });

    print('[APP] Stopping wallpaper on all monitors');
    
    try {
      final success = await AnyWPEngine.stopWallpaperOnAllMonitors();

      setState(() {
        for (final key in _monitorWallpapers.keys) {
          _monitorWallpapers[key] = false;
        }
      });

      if (success) {
        _showMessage('Wallpaper stopped on all monitors');
      } else {
        _showMessage('Some monitors failed to stop');
      }
    } finally {
      // Clear loading state
      setState(() {
        _allMonitorsLoading = false;
        for (final monitor in _monitors) {
          _monitorLoading[monitor.index] = false;
        }
      });
    }
  }

  void _showMessage(String message) {
    print('[APP] $message');
    if (mounted) {
      final messenger = ScaffoldMessenger.maybeOf(context);
      if (messenger != null) {
        messenger.showSnackBar(
          SnackBar(content: Text(message)),
        );
      }
    }
  }
  
  // Get primary monitor's URL (helper for fallback)
  // IMPORTANT: Only returns URL if wallpaper is RUNNING successfully
  // This prevents fallback loops when URLs are invalid
  String? _getPrimaryMonitorUrl() {
    // Find primary monitor with RUNNING wallpaper
    for (final monitor in _monitors) {
      if (monitor.isPrimary) {
        // Check if wallpaper is actually running on this monitor
        if (_monitorWallpapers[monitor.index] == true) {
          final controller = _monitorUrlControllers[monitor.index];
          if (controller != null) {
            final url = controller.text.trim();
            if (url.isNotEmpty) {
              print('[APP] Found RUNNING wallpaper on primary monitor: $url');
              return url;
            }
          }
        } else {
          print('[APP] Primary monitor exists but wallpaper is not running');
        }
      }
    }
    
    // Fallback: return any RUNNING wallpaper URL (not primary)
    for (final entry in _monitorWallpapers.entries) {
      if (entry.value == true && _monitorUrlControllers.containsKey(entry.key)) {
        final url = _monitorUrlControllers[entry.key]!.text.trim();
        if (url.isNotEmpty) {
          print('[APP] Found RUNNING wallpaper on monitor ${entry.key}: $url');
          return url;
        }
      }
    }
    
    print('[APP] No RUNNING wallpaper found for fallback');
    return null;
  }
  
  // Load quick test page URL and auto-start/navigate wallpaper
  Future<void> _loadTestPage(int monitorIndex, String filename) async {
    final controller = _monitorUrlControllers[monitorIndex];
    if (controller == null) return;
    
    final url = 'file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/$filename';
    controller.text = url;
    
    // Smart switching: use navigate if already running, otherwise start fresh
    final isRunning = _monitorWallpapers[monitorIndex] == true;
    
    if (isRunning) {
      // Seamless navigation without showing desktop
      print('[APP] Seamlessly navigating to $filename on monitor $monitorIndex');
      
      setState(() {
        _monitorLoading[monitorIndex] = true;
      });
      
      try {
        final success = await AnyWPEngine.navigateToUrlOnMonitor(url, monitorIndex);
        
        setState(() {
          _monitorLoading[monitorIndex] = false;
        });
        
        if (success) {
          _showMessage('🎨 Switched to: $filename');
        } else {
          _showMessage('Failed to switch page, restarting...');
          // Fallback to restart if navigate failed
          await _startWallpaperOnMonitor(monitorIndex);
        }
      } catch (e) {
        setState(() {
          _monitorLoading[monitorIndex] = false;
        });
        _showMessage('Error: $e');
      }
    } else {
      // First time start
      print('[APP] Starting wallpaper with $filename on monitor $monitorIndex');
      await _startWallpaperOnMonitor(monitorIndex);
    }
  }
  
  // Power Saving & Optimization methods
  Future<void> _refreshPowerState() async {
    try {
      final state = await AnyWPEngine.getPowerState();
      final memory = await AnyWPEngine.getMemoryUsage();
      
      setState(() {
        _powerState = state;
        _memoryUsageMB = memory;
      });
    } catch (e) {
      print('[APP] Error refreshing power state: $e');
    }
  }
  
  Future<void> _pauseWallpaper() async {
    try {
      final success = await AnyWPEngine.pauseWallpaper();
      if (success) {
        _showMessage('Wallpaper paused');
        await _refreshPowerState();
      } else {
        _showMessage('Failed to pause wallpaper');
      }
    } catch (e) {
      _showMessage('Error: $e');
    }
  }
  
  Future<void> _resumeWallpaper() async {
    try {
      final success = await AnyWPEngine.resumeWallpaper();
      if (success) {
        _showMessage('Wallpaper resumed');
        await _refreshPowerState();
      } else {
        _showMessage('Failed to resume wallpaper');
      }
    } catch (e) {
      _showMessage('Error: $e');
    }
  }
  
  Future<void> _toggleAutoPowerSaving() async {
    try {
      final newValue = !_autoPowerSaving;
      final success = await AnyWPEngine.setAutoPowerSaving(newValue);
      if (success) {
        setState(() {
          _autoPowerSaving = newValue;
        });
        _showMessage('Auto power saving: ${newValue ? 'Enabled' : 'Disabled'}');
      }
    } catch (e) {
      _showMessage('Error: $e');
    }
  }
  
  Future<void> _optimizeMemory() async {
    try {
      final before = await AnyWPEngine.getMemoryUsage();
      await AnyWPEngine.optimizeMemory();
      await Future.delayed(Duration(seconds: 1)); // Wait for optimization
      final after = await AnyWPEngine.getMemoryUsage();
      
      final freed = before - after;
      _showMessage('Memory optimized! Freed: ${freed}MB (${before}MB → ${after}MB)');
      
      await _refreshPowerState();
    } catch (e) {
      _showMessage('Error: $e');
    }
  }
  
  Widget _buildOptimizationTab() {
    return ListView(
      padding: const EdgeInsets.all(24.0),
      children: [
        Card(
          elevation: 4,
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.power_settings_new, color: Colors.green),
                    SizedBox(width: 8),
                    Text(
                      'Power Saving Status',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    Spacer(),
                    ElevatedButton.icon(
                      onPressed: _refreshPowerState,
                      icon: Icon(Icons.refresh, size: 16),
                      label: Text('Refresh'),
                    ),
                  ],
                ),
                SizedBox(height: 16),
                _buildStatusRow('Power State', _powerState, _getPowerStateColor(_powerState)),
                SizedBox(height: 8),
                _buildStatusRow('Memory Usage', '${_memoryUsageMB}MB', Colors.blue),
                SizedBox(height: 16),
                CheckboxListTile(
                  title: Text('Auto Power Saving'),
                  subtitle: Text('Automatically pause on lock/idle/fullscreen'),
                  value: _autoPowerSaving,
                  contentPadding: EdgeInsets.zero,
                  onChanged: (value) => _toggleAutoPowerSaving(),
                ),
              ],
            ),
          ),
        ),
        SizedBox(height: 16),
        Card(
          elevation: 4,
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.settings, color: Colors.blue),
                    SizedBox(width: 8),
                    Text(
                      'Manual Controls',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 16),
                Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: _pauseWallpaper,
                        icon: Icon(Icons.pause),
                        label: Text('Pause Wallpaper'),
                        style: ElevatedButton.styleFrom(
                          padding: EdgeInsets.all(16),
                          backgroundColor: Colors.orange,
                          foregroundColor: Colors.white,
                        ),
                      ),
                    ),
                    SizedBox(width: 16),
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: _resumeWallpaper,
                        icon: Icon(Icons.play_arrow),
                        label: Text('Resume Wallpaper'),
                        style: ElevatedButton.styleFrom(
                          padding: EdgeInsets.all(16),
                          backgroundColor: Colors.green,
                          foregroundColor: Colors.white,
                        ),
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 16),
                SizedBox(
                  width: double.infinity,
                  child: ElevatedButton.icon(
                    onPressed: _optimizeMemory,
                    icon: Icon(Icons.memory),
                    label: Text('Optimize Memory'),
                    style: ElevatedButton.styleFrom(
                      padding: EdgeInsets.all(16),
                      backgroundColor: Colors.purple,
                      foregroundColor: Colors.white,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
        SizedBox(height: 16),
        Card(
          color: Colors.blue[50],
          child: Padding(
            padding: EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.info_outline, color: Colors.blue),
                    SizedBox(width: 8),
                    Text(
                      'Optimization Features:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 16,
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 8),
                Text(
                  '🔋 Auto Power Saving:\n'
                  '  • Pauses wallpaper when system is locked\n'
                  '  • Detects fullscreen apps (games/videos)\n'
                  '  • Pauses after 5 min idle\n\n'
                  '💾 Memory Optimization:\n'
                  '  • Clears cache and triggers GC\n'
                  '  • Reduces memory footprint\n'
                  '  • Automatic on pause\n\n'
                  '⚡ Performance:\n'
                  '  • Reduces render frequency when paused\n'
                  '  • Optimized mouse hook\n'
                  '  • Minimal logging overhead',
                  style: TextStyle(color: Colors.grey[800], fontSize: 13),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }
  
  Widget _buildStatusRow(String label, String value, Color color) {
    return Row(
      children: [
        Expanded(
          child: Text(
            label + ':',
            style: TextStyle(fontWeight: FontWeight.w500),
          ),
        ),
        Container(
          padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
          decoration: BoxDecoration(
            color: color.withOpacity(0.1),
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: color.withOpacity(0.3)),
          ),
          child: Text(
            value,
            style: TextStyle(
              color: color,
              fontWeight: FontWeight.bold,
            ),
          ),
        ),
      ],
    );
  }
  
  Color _getPowerStateColor(String state) {
    switch (state) {
      case 'ACTIVE': return Colors.green;
      case 'IDLE': return Colors.orange;
      case 'LOCKED': return Colors.red;
      case 'FULLSCREEN_APP': return Colors.purple;
      case 'PAUSED': return Colors.grey;
      default: return Colors.grey;
    }
  }

  Widget _buildMultiMonitorTab() {
    return ListView(
      padding: const EdgeInsets.all(24.0),
      children: [
        Card(
          elevation: 4,
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.monitor, color: Colors.blue),
                    SizedBox(width: 8),
                    Expanded(
                      child: Text(
                        'Detected Monitors: ${_monitors.length}',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ),
                    ElevatedButton.icon(
                      onPressed: _loadMonitors,
                      icon: Icon(Icons.refresh, size: 16),
                      label: Text('Refresh'),
                      style: ElevatedButton.styleFrom(
                        padding: EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 12),
                CheckboxListTile(
                  title: const Text('Enable Mouse Transparency'),
                  subtitle: const Text('Allow clicks to pass through to desktop'),
                  value: _mouseTransparent,
                  contentPadding: EdgeInsets.zero,
                  onChanged: (value) {
                    setState(() {
                      _mouseTransparent = value ?? true;
                    });
                  },
                ),
              ],
            ),
          ),
        ),
        SizedBox(height: 16),
        Row(
          children: [
            Expanded(
              child: ElevatedButton.icon(
                onPressed: _allMonitorsLoading ? null : _startWallpaperOnAllMonitors,
                icon: _allMonitorsLoading 
                    ? SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white),
                      )
                    : Icon(Icons.play_arrow),
                label: Text(_allMonitorsLoading ? 'Starting...' : 'Start All (with individual URLs)'),
                style: ElevatedButton.styleFrom(
                  padding: EdgeInsets.all(16),
                  backgroundColor: Colors.green,
                  foregroundColor: Colors.white,
                ),
              ),
            ),
            SizedBox(width: 16),
            Expanded(
              child: ElevatedButton.icon(
                onPressed: _allMonitorsLoading ? null : _stopWallpaperOnAllMonitors,
                icon: _allMonitorsLoading 
                    ? SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white),
                      )
                    : Icon(Icons.stop),
                label: Text(_allMonitorsLoading ? 'Stopping...' : 'Stop All'),
                style: ElevatedButton.styleFrom(
                  padding: EdgeInsets.all(16),
                  backgroundColor: Colors.red,
                  foregroundColor: Colors.white,
                ),
              ),
            ),
          ],
        ),
        SizedBox(height: 24),
        Text(
          'Individual Monitor Control:',
          style: TextStyle(
            fontSize: 16,
            fontWeight: FontWeight.bold,
          ),
        ),
        SizedBox(height: 8),
        ..._monitors.map((monitor) {
          final controller = _monitorUrlControllers[monitor.index];
          return Card(
            margin: EdgeInsets.symmetric(vertical: 8),
            child: Padding(
              padding: EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      Icon(
                        monitor.isPrimary ? Icons.star : Icons.monitor,
                        color: monitor.isPrimary ? Colors.amber : Colors.grey,
                      ),
                      SizedBox(width: 8),
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Text(
                              'Monitor ${monitor.index}${monitor.isPrimary ? ' (Primary)' : ''}',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                                fontSize: 16,
                              ),
                            ),
                            Text(
                              '${monitor.width}x${monitor.height} @ (${monitor.left}, ${monitor.top})',
                              style: TextStyle(
                                fontSize: 12,
                                color: Colors.grey[600],
                              ),
                            ),
                            Text(
                              monitor.deviceName,
                              style: TextStyle(
                                fontSize: 10,
                                color: Colors.grey[500],
                              ),
                            ),
                          ],
                        ),
                      ),
                      Chip(
                        label: Text(
                          _monitorWallpapers[monitor.index] == true ? 'Running' : 'Stopped',
                          style: TextStyle(fontSize: 12),
                        ),
                        backgroundColor: _monitorWallpapers[monitor.index] == true
                            ? Colors.green[100]
                            : Colors.grey[300],
                      ),
                    ],
                  ),
                  SizedBox(height: 12),
                  // Quick test pages section
                  Text(
                    '🚀 Quick Test Pages:',
                    style: TextStyle(
                      fontWeight: FontWeight.w600,
                      fontSize: 13,
                      color: Colors.blue[700],
                    ),
                  ),
                  SizedBox(height: 8),
                  Wrap(
                    spacing: 6,
                    runSpacing: 6,
                    children: _testPages.map((page) {
                      return InkWell(
                        onTap: () => _loadTestPage(monitor.index, page['file']!),
                        borderRadius: BorderRadius.circular(8),
                        child: Container(
                          padding: EdgeInsets.symmetric(horizontal: 10, vertical: 6),
                          decoration: BoxDecoration(
                            color: Colors.blue[50],
                            border: Border.all(color: Colors.blue[200]!),
                            borderRadius: BorderRadius.circular(8),
                          ),
                          child: Row(
                            mainAxisSize: MainAxisSize.min,
                            children: [
                              Text(
                                page['icon']!,
                                style: TextStyle(fontSize: 14),
                              ),
                              SizedBox(width: 4),
                              Text(
                                page['name']!,
                                style: TextStyle(
                                  fontSize: 12,
                                  fontWeight: FontWeight.w500,
                                  color: Colors.blue[900],
                                ),
                              ),
                            ],
                          ),
                        ),
                      );
                    }).toList(),
                  ),
                  SizedBox(height: 12),
                  // Custom URL input
                  Text(
                    '🔗 Custom URL:',
                    style: TextStyle(
                      fontWeight: FontWeight.w600,
                      fontSize: 13,
                      color: Colors.grey[700],
                    ),
                  ),
                  SizedBox(height: 8),
                  if (controller != null)
                    TextField(
                      controller: controller,
                      decoration: InputDecoration(
                        labelText: 'URL for Monitor ${monitor.index}',
                        hintText: 'Or enter custom URL here',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.link, size: 20),
                        contentPadding: EdgeInsets.symmetric(horizontal: 12, vertical: 12),
                      ),
                      style: TextStyle(fontSize: 14),
                    ),
                  SizedBox(height: 12),
                  Row(
                    children: [
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: (_monitorWallpapers[monitor.index] == true || 
                                     _monitorLoading[monitor.index] == true)
                              ? null
                              : () => _startWallpaperOnMonitor(monitor.index),
                          icon: _monitorLoading[monitor.index] == true
                              ? SizedBox(
                                  width: 14,
                                  height: 14,
                                  child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white),
                                )
                              : Icon(Icons.play_arrow, size: 16),
                          label: Text(_monitorLoading[monitor.index] == true ? 'Starting...' : 'Start'),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.green,
                            foregroundColor: Colors.white,
                          ),
                        ),
                      ),
                      SizedBox(width: 8),
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: (_monitorWallpapers[monitor.index] != true || 
                                     _monitorLoading[monitor.index] == true)
                              ? null
                              : () => _stopWallpaperOnMonitor(monitor.index),
                          icon: _monitorLoading[monitor.index] == true
                              ? SizedBox(
                                  width: 14,
                                  height: 14,
                                  child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white),
                                )
                              : Icon(Icons.stop, size: 16),
                          label: Text(_monitorLoading[monitor.index] == true ? 'Stopping...' : 'Stop'),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.red,
                            foregroundColor: Colors.white,
                          ),
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          );
        }).toList(),
        SizedBox(height: 24),
        Card(
          color: Colors.blue[50],
          child: Padding(
            padding: EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.info_outline, color: Colors.blue),
                    SizedBox(width: 8),
                    Text(
                      'Multi-Monitor Instructions:',
                      style: TextStyle(
                        fontWeight: FontWeight.bold,
                        fontSize: 16,
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 8),
                Text(
                  '1. Check detected monitors above\n'
                  '2. Click 🚀 Quick Test buttons - wallpaper starts automatically!\n'
                  '3. Or enter custom URL and click "Start" manually\n'
                  '4. Use "Start All" to apply to all monitors at once\n'
                  '5. Each monitor displays its own independent content!\n\n'
                  '💡 Quick Test Pages:\n'
                  '  🎨 Simple - Basic wallpaper test\n'
                  '  🖱️ Draggable - Drag & drop demo (mouse hook)\n'
                  '  🔍 Drag Debug - Drag debug with detailed logs\n'
                  '  ⚙️ API Test - Full API testing\n'
                  '  👆 Click Test - Click detection test\n'
                  '  👁️ Visibility - Power saving test\n'
                  '  ⚛️ React / 💚 Vue - SPA framework tests\n'
                  '  📺 iFrame Ads - Ad detection test\n\n'
                  '✨ Tips:\n'
                  '  • Quick test buttons auto-start instantly (no need to click Start!)\n'
                  '  • Mouse transparency works with drag & drop!\n'
                  '  • Try different pages on different monitors',
                  style: TextStyle(color: Colors.grey[800], fontSize: 13),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'AnyWallpaper Engine Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        useMaterial3: true,
      ),
      home: DefaultTabController(
        length: 2,
        child: Scaffold(
          appBar: AppBar(
            title: const Text('AnyWallpaper Engine'),
            backgroundColor: Colors.blue,
            bottom: TabBar(
              tabs: [
                Tab(icon: Icon(Icons.monitor), text: 'Wallpaper'),
                Tab(icon: Icon(Icons.tune), text: 'Optimization'),
              ],
            ),
          ),
          body: TabBarView(
            children: [
              _buildMultiMonitorTab(),
              _buildOptimizationTab(),
            ],
          ),
        ),
      ),
    );
  }
}

