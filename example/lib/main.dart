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
  final TextEditingController _urlController = TextEditingController(
    text: 'file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine/examples/test_simple.html',
  );
  bool _isRunning = false;
  bool _mouseTransparent = true;  // Default: wallpaper mode (transparent)
  
  // Multi-monitor support
  List<MonitorInfo> _monitors = [];
  Map<int, bool> _monitorWallpapers = {};  // Track which monitors have wallpapers
  int _selectedTabIndex = 0;  // 0 = single, 1 = multi-monitor

  @override
  void initState() {
    super.initState();
    _loadMonitors();
    // Auto-start for testing - 3 seconds delay
    Future.delayed(Duration(seconds: 3), () {
      print('[APP] Auto-starting wallpaper for test...');
      _startWallpaper();
    });
  }
  
  Future<void> _loadMonitors() async {
    final monitors = await AnyWPEngine.getMonitors();
    setState(() {
      _monitors = monitors;
      // Initialize wallpaper tracking
      for (final monitor in monitors) {
        _monitorWallpapers[monitor.index] = false;
      }
    });
    print('[APP] Found ${monitors.length} monitor(s):');
    for (final monitor in monitors) {
      print('[APP]   $monitor');
    }
  }

  Future<void> _startWallpaper() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Starting wallpaper with URL: $url');
    final success = await AnyWPEngine.initializeWallpaper(
      url: url,
      enableMouseTransparent: _mouseTransparent,
    );

    setState(() {
      _isRunning = success;
    });

    if (success) {
      _showMessage('Wallpaper started successfully!');
    } else {
      _showMessage('Failed to start wallpaper');
    }
  }

  Future<void> _stopWallpaper() async {
    print('[APP] Stopping wallpaper');
    final success = await AnyWPEngine.stopWallpaper();

    setState(() {
      _isRunning = false;
    });

    if (success) {
      _showMessage('Wallpaper stopped');
    } else {
      _showMessage('Failed to stop wallpaper');
    }
  }

  Future<void> _navigateToUrl() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Navigating to URL: $url');
    final success = await AnyWPEngine.navigateToUrl(url);

    if (success) {
      _showMessage('Navigation successful');
    } else {
      _showMessage('Navigation failed');
    }
  }

  Future<void> _startWallpaperOnMonitor(int monitorIndex) async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Starting wallpaper on monitor $monitorIndex with URL: $url');
    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: url,
      monitorIndex: monitorIndex,
      enableMouseTransparent: _mouseTransparent,
    );

    setState(() {
      _monitorWallpapers[monitorIndex] = success;
    });

    if (success) {
      _showMessage('Wallpaper started on monitor $monitorIndex');
    } else {
      _showMessage('Failed to start wallpaper on monitor $monitorIndex');
    }
  }

  Future<void> _stopWallpaperOnMonitor(int monitorIndex) async {
    print('[APP] Stopping wallpaper on monitor $monitorIndex');
    final success = await AnyWPEngine.stopWallpaperOnMonitor(monitorIndex);

    setState(() {
      _monitorWallpapers[monitorIndex] = false;
    });

    if (success) {
      _showMessage('Wallpaper stopped on monitor $monitorIndex');
    } else {
      _showMessage('Failed to stop wallpaper on monitor $monitorIndex');
    }
  }

  Future<void> _startWallpaperOnAllMonitors() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Starting wallpaper on all monitors');
    final results = await AnyWPEngine.initializeWallpaperOnAllMonitors(
      url: url,
      enableMouseTransparent: _mouseTransparent,
    );

    setState(() {
      _monitorWallpapers.addAll(results);
    });

    final successCount = results.values.where((v) => v).length;
    _showMessage('Started wallpaper on $successCount/${results.length} monitor(s)');
  }

  Future<void> _stopWallpaperOnAllMonitors() async {
    print('[APP] Stopping wallpaper on all monitors');
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

  Widget _buildSingleMonitorTab() {
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
                      Text(
                        'Status: ${_isRunning ? "Running" : "Stopped"}',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                          color: _isRunning ? Colors.green : Colors.red,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'WebView2 will be displayed as desktop wallpaper behind icons',
                        style: TextStyle(
                          fontSize: 14,
                          color: Colors.grey[600],
                        ),
                      ),
                    ],
                  ),
                ),
              ),
              const SizedBox(height: 24),
              TextField(
                controller: _urlController,
                decoration: const InputDecoration(
                  labelText: 'URL',
                  hintText: 'Enter URL to display',
                  border: OutlineInputBorder(),
                  prefixIcon: Icon(Icons.link),
                ),
              ),
              const SizedBox(height: 16),
              CheckboxListTile(
                title: const Text('Enable Mouse Transparency'),
                subtitle: const Text('Allow clicks to pass through to desktop'),
                value: _mouseTransparent,
                onChanged: (value) {
                  setState(() {
                    _mouseTransparent = value ?? true;
                  });
                },
              ),
              const SizedBox(height: 24),
              Row(
                children: [
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: _isRunning ? null : _startWallpaper,
                      icon: const Icon(Icons.play_arrow),
                      label: const Text('Start Wallpaper'),
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.all(16),
                        backgroundColor: Colors.green,
                        foregroundColor: Colors.white,
                      ),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: !_isRunning ? null : _stopWallpaper,
                      icon: const Icon(Icons.stop),
                      label: const Text('Stop Wallpaper'),
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.all(16),
                        backgroundColor: Colors.red,
                        foregroundColor: Colors.white,
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),
              ElevatedButton.icon(
                onPressed: !_isRunning ? null : _navigateToUrl,
                icon: const Icon(Icons.navigation),
                label: const Text('Navigate to URL'),
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.all(16),
                ),
              ),
              const SizedBox(height: 24),
              Card(
                color: Colors.blue[50],
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Row(
                        children: [
                          Icon(Icons.info_outline, color: Colors.blue),
                          SizedBox(width: 8),
                          Text(
                            'Instructions:',
                            style: TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 16,
                            ),
                          ),
                        ],
                      ),
                      const SizedBox(height: 8),
                      Text(
                        '1. Enter a URL (e.g., https://www.bing.com)\n'
                        '2. Click "Start Wallpaper"\n'
                        '3. Check your desktop - WebView2 should appear behind icons\n'
                        '4. Use "Navigate to URL" to change content\n'
                        '5. Click "Stop Wallpaper" to remove',
                        style: TextStyle(color: Colors.grey[800]),
                      ),
                    ],
                  ),
                ),
              ),
      ],
    );
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
                    Text(
                      'Detected Monitors: ${_monitors.length}',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 16),
                ElevatedButton.icon(
                  onPressed: _loadMonitors,
                  icon: Icon(Icons.refresh),
                  label: Text('Refresh Monitors'),
                ),
              ],
            ),
          ),
        ),
        SizedBox(height: 16),
        TextField(
          controller: _urlController,
          decoration: const InputDecoration(
            labelText: 'URL',
            hintText: 'Enter URL to display',
            border: OutlineInputBorder(),
            prefixIcon: Icon(Icons.link),
          ),
        ),
        SizedBox(height: 16),
        CheckboxListTile(
          title: const Text('Enable Mouse Transparency'),
          subtitle: const Text('Allow clicks to pass through to desktop'),
          value: _mouseTransparent,
          onChanged: (value) {
            setState(() {
              _mouseTransparent = value ?? true;
            });
          },
        ),
        SizedBox(height: 16),
        Row(
          children: [
            Expanded(
              child: ElevatedButton.icon(
                onPressed: _startWallpaperOnAllMonitors,
                icon: Icon(Icons.play_arrow),
                label: Text('Start on All'),
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
                onPressed: _stopWallpaperOnAllMonitors,
                icon: Icon(Icons.stop),
                label: Text('Stop All'),
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
        ..._monitors.map((monitor) => Card(
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
                SizedBox(height: 8),
                Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: _monitorWallpapers[monitor.index] == true
                            ? null
                            : () => _startWallpaperOnMonitor(monitor.index),
                        icon: Icon(Icons.play_arrow, size: 16),
                        label: Text('Start'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.green,
                          foregroundColor: Colors.white,
                        ),
                      ),
                    ),
                    SizedBox(width: 8),
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: _monitorWallpapers[monitor.index] != true
                            ? null
                            : () => _stopWallpaperOnMonitor(monitor.index),
                        icon: Icon(Icons.stop, size: 16),
                        label: Text('Stop'),
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
        )).toList(),
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
                  '2. Enter a URL to display\n'
                  '3. Use "Start on All" to set wallpaper on all monitors\n'
                  '4. Or use individual controls for each monitor\n'
                  '5. Each monitor can have independent wallpapers',
                  style: TextStyle(color: Colors.grey[800]),
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
            title: const Text('AnyWallpaper Engine - Desktop Wallpaper'),
            backgroundColor: Colors.blue,
            bottom: TabBar(
              onTap: (index) {
                setState(() {
                  _selectedTabIndex = index;
                });
              },
              tabs: [
                Tab(icon: Icon(Icons.desktop_windows), text: 'Single Monitor'),
                Tab(icon: Icon(Icons.view_column), text: 'Multi-Monitor'),
              ],
            ),
          ),
          body: TabBarView(
            children: [
              _buildSingleMonitorTab(),
              _buildMultiMonitorTab(),
            ],
          ),
        ),
      ),
    );
  }
}

