import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() {
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

  @override
  void initState() {
    super.initState();
    _loadMonitors();
  }
  
  @override
  void dispose() {
    for (final controller in _monitorUrlControllers.values) {
      controller.dispose();
    }
    super.dispose();
  }
  
  Future<void> _loadMonitors() async {
    final monitors = await AnyWPEngine.getMonitors();
    setState(() {
      _monitors = monitors;
      // Initialize wallpaper tracking, loading state and URL controllers for each monitor
      for (final monitor in monitors) {
        _monitorWallpapers[monitor.index] = false;
        _monitorLoading[monitor.index] = false;
        // Each monitor gets its own URL controller with default URL
        _monitorUrlControllers[monitor.index] = TextEditingController(
          text: 'file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_simple.html',
        );
      }
    });
    print('[APP] Found ${monitors.length} monitor(s):');
    for (final monitor in monitors) {
      print('[APP]   $monitor');
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
                  '  • Disable mouse transparency for interactive wallpapers\n'
                  '  • Use test_simple.html, test_api.html, or test_iframe_ads.html',
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
      home: Scaffold(
        appBar: AppBar(
          title: const Text('AnyWallpaper Engine - Multi-Monitor Wallpaper'),
          backgroundColor: Colors.blue,
        ),
        body: _buildMultiMonitorTab(),
      ),
    );
  }
}

