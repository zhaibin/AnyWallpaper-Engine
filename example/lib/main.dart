import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
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

class _MyAppState extends State<MyApp> {
  // Multi-monitor support
  List<MonitorInfo> _monitors = [];
  Map<int, bool> _monitorWallpapers = {};  // Track which monitors have wallpapers
  Map<int, TextEditingController> _monitorUrlControllers = {};  // Each monitor has its own URL
  Map<int, bool> _monitorLoading = {};  // Track loading state for each monitor
  bool _allMonitorsLoading = false;  // Track "Start All" / "Stop All" loading state
  bool _mouseTransparent = true;  // Default: wallpaper mode (transparent)
  
  // Power saving & optimization
  String _powerState = 'Loading...';
  int _memoryUsageMB = 0;
  bool _autoPowerSaving = true;
  int _selectedTab = 0;

  @override
  void initState() {
    super.initState();
    _loadMonitors();
    
    // Listen for monitor changes from native side
    AnyWPEngine.setOnMonitorChangeCallback(() {
      print('[APP] Monitor change callback received!');
      _handleMonitorChange();
    });
  }
  
  Future<void> _handleMonitorChange() async {
    print('[APP] Handling monitor change - refreshing monitor list...');
    
    try {
      await _loadMonitors();
      
      if (mounted) {
        _showMessage('Display configuration changed - UI updated');
      }
      
      print('[APP] Monitor change handled successfully');
    } catch (e, stackTrace) {
      print('[APP] ERROR in _handleMonitorChange: $e');
      print('[APP] StackTrace: $stackTrace');
    }
  }
  
  @override
  void dispose() {
    for (final controller in _monitorUrlControllers.values) {
      controller.dispose();
    }
    super.dispose();
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
                  if (controller != null)
                    TextField(
                      controller: controller,
                      decoration: InputDecoration(
                        labelText: 'URL for Monitor ${monitor.index}',
                        hintText: 'Enter URL to display',
                        border: OutlineInputBorder(),
                        prefixIcon: Icon(Icons.link, size: 20),
                        contentPadding: EdgeInsets.symmetric(horizontal: 12, vertical: 12),
                      ),
                      style: TextStyle(fontSize: 14),
                    ),
                  SizedBox(height: 8),
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
                  '2. Enter different URLs for each monitor (optional)\n'
                  '3. Use "Start All" to set all monitors at once\n'
                  '4. Or use individual controls for each monitor\n'
                  '5. Each monitor displays its own independent content!\n\n'
                  '💡 Tips:\n'
                  '  • Try different HTML files on different monitors\n'
                  '  • test_draggable.html - 拖拽演示（支持鼠标透明）\n'
                  '  • test_simple.html, test_api.html - 基础测试\n'
                  '  • Mouse transparency works with drag & drop via hook!',
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

