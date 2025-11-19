# AnyWP Engine - API Usage Examples

Practical examples for common use cases.

## Table of Contents

1. [Basic Setup](#basic-setup)
2. [Multi-Monitor Scenarios](#multi-monitor-scenarios)
3. [Power Management](#power-management)
4. [Dynamic Configuration](#dynamic-configuration)
5. [Advanced Integration](#advanced-integration)

---

## Basic Setup

### Example 1: Simple Wallpaper

最简单的壁纸应用：

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() {
  runApp(MyWallpaperApp());
}

class MyWallpaperApp extends StatefulWidget {
  @override
  _MyWallpaperAppState createState() => _MyWallpaperAppState();
}

class _MyWallpaperAppState extends State<MyWallpaperApp> {
  bool _isRunning = false;
  final _urlController = TextEditingController(
    text: 'https://example.com/wallpaper.html',
  );

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('My Wallpaper')),
        body: Padding(
          padding: EdgeInsets.all(20),
          child: Column(
            children: [
              TextField(
                controller: _urlController,
                decoration: InputDecoration(labelText: 'URL'),
              ),
              SizedBox(height: 20),
              ElevatedButton(
                onPressed: _isRunning ? _stop : _start,
                child: Text(_isRunning ? 'Stop' : 'Start'),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Future<void> _start() async {
    final success = await AnyWPEngine.initializeWallpaper(
      url: _urlController.text,
    );
    
    if (success) {
      setState(() => _isRunning = true);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Wallpaper started!')),
      );
    }
  }

  Future<void> _stop() async {
    await AnyWPEngine.stopWallpaper();
    setState(() => _isRunning = false);
  }

  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }
}
```

---

## Multi-Monitor Scenarios

### Example 2: Different Content Per Monitor

为每个显示器显示不同内容：

```dart
class MultiMonitorWallpaper extends StatefulWidget {
  @override
  _MultiMonitorWallpaperState createState() => _MultiMonitorWallpaperState();
}

class _MultiMonitorWallpaperState extends State<MultiMonitorWallpaper> {
  List<MonitorInfo> _monitors = [];
  Map<int, String> _monitorUrls = {};
  Map<int, bool> _monitorStatus = {};

  @override
  void initState() {
    super.initState();
    _loadMonitors();
  }

  Future<void> _loadMonitors() async {
    final monitors = await AnyWPEngine.getMonitors();
    setState(() {
      _monitors = monitors;
      
      // Setup default URLs for each monitor
      for (var monitor in monitors) {
        _monitorUrls[monitor.index] = monitor.isPrimary
            ? 'https://example.com/primary.html'
            : 'https://example.com/secondary.html';
        _monitorStatus[monitor.index] = false;
      }
    });
  }

  Future<void> _startMonitor(int index) async {
    final url = _monitorUrls[index] ?? '';
    if (url.isEmpty) return;

    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: url,
      monitorIndex: index,
      enableMouseTransparent: true,
    );

    if (success) {
      setState(() => _monitorStatus[index] = true);
    }
  }

  Future<void> _stopMonitor(int index) async {
    await AnyWPEngine.stopWallpaperOnMonitor(index);
    setState(() => _monitorStatus[index] = false);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Multi-Monitor Control')),
      body: ListView.builder(
        itemCount: _monitors.length,
        itemBuilder: (context, i) {
          final monitor = _monitors[i];
          final isRunning = _monitorStatus[monitor.index] ?? false;

          return Card(
            child: ListTile(
              leading: Icon(
                monitor.isPrimary ? Icons.star : Icons.monitor,
                color: monitor.isPrimary ? Colors.amber : null,
              ),
              title: Text('Monitor ${monitor.index}${monitor.isPrimary ? ' (Primary)' : ''}'),
              subtitle: Text('${monitor.width}x${monitor.height}'),
              trailing: Switch(
                value: isRunning,
                onChanged: (value) {
                  if (value) {
                    _startMonitor(monitor.index);
                  } else {
                    _stopMonitor(monitor.index);
                  }
                },
              ),
            ),
          );
        },
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () => _showUrlDialog(),
        child: Icon(Icons.edit),
      ),
    );
  }

  void _showUrlDialog() {
    // Show dialog to edit URLs for each monitor
    // Implementation omitted for brevity
  }
}
```

### Example 3: Mirror Mode

在所有显示器上显示相同内容：

```dart
class MirrorModeWallpaper extends StatelessWidget {
  Future<void> startMirrorMode(String url) async {
    final results = await AnyWPEngine.initializeWallpaperOnAllMonitors(
      url: url,
      enableMouseTransparent: true,
    );

    int successCount = 0;
    results.forEach((index, success) {
      if (success) successCount++;
    });

    print('Started on $successCount/${results.length} monitors');
  }

  Future<void> stopMirrorMode() async {
    await AnyWPEngine.stopWallpaperOnAllMonitors();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Mirror Mode')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ElevatedButton(
              onPressed: () => startMirrorMode('https://example.com/mirror.html'),
              child: Text('Start Mirror Mode'),
            ),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: stopMirrorMode,
              child: Text('Stop All'),
            ),
          ],
        ),
      ),
    );
  }
}
```

---

## Power Management

### Example 4: Battery-Aware Wallpaper

根据电池状态自动调整省电策略：

```dart
import 'package:battery_plus/battery_plus.dart';

class BatteryAwareWallpaper extends StatefulWidget {
  @override
  _BatteryAwareWallpaperState createState() => _BatteryAwareWallpaperState();
}

class _BatteryAwareWallpaperState extends State<BatteryAwareWallpaper> {
  final _battery = Battery();
  bool _isOnBattery = false;

  @override
  void initState() {
    super.initState();
    _monitorBattery();
  }

  Future<void> _monitorBattery() async {
    // Check initial state
    final state = await _battery.batteryState;
    _updatePowerSettings(state);

    // Listen for changes
    _battery.onBatteryStateChanged.listen(_updatePowerSettings);
  }

  Future<void> _updatePowerSettings(BatteryState state) async {
    final isOnBattery = state == BatteryState.discharging;
    
    setState(() => _isOnBattery = isOnBattery);

    if (isOnBattery) {
      // Aggressive power saving on battery
      await AnyWPEngine.setIdleTimeout(180);      // 3 min
      await AnyWPEngine.setMemoryThreshold(150);  // 150MB
      await AnyWPEngine.setCleanupInterval(30);   // 30 min
      await AnyWPEngine.setAutoPowerSaving(true);
      
      print('Battery mode: Aggressive power saving');
    } else {
      // Normal settings on AC power
      await AnyWPEngine.setIdleTimeout(600);      // 10 min
      await AnyWPEngine.setMemoryThreshold(300);  // 300MB
      await AnyWPEngine.setCleanupInterval(60);   // 60 min
      
      print('AC power: Normal settings');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Battery-Aware Wallpaper'),
        actions: [
          Icon(_isOnBattery ? Icons.battery_alert : Icons.power),
        ],
      ),
      body: Center(
        child: Text(
          _isOnBattery
              ? 'On Battery - Power Saving Active'
              : 'On AC Power - Normal Mode',
          style: Theme.of(context).textTheme.headline6,
        ),
      ),
    );
  }
}
```

### Example 5: Manual Power Control

手动控制暂停/恢复：

```dart
class ManualPowerControl extends StatefulWidget {
  @override
  _ManualPowerControlState createState() => _ManualPowerControlState();
}

class _ManualPowerControlState extends State<ManualPowerControl> {
  String _powerState = 'UNKNOWN';
  int _memoryUsage = 0;

  @override
  void initState() {
    super.initState();
    _refreshStatus();
    
    // Listen for power state changes
    AnyWPEngine.setOnPowerStateChangeCallback((old, newState) {
      setState(() => _powerState = newState);
      _showNotification('Power state: $old -> $newState');
    });
  }

  Future<void> _refreshStatus() async {
    final state = await AnyWPEngine.getPowerState();
    final memory = await AnyWPEngine.getMemoryUsage();
    
    setState(() {
      _powerState = state;
      _memoryUsage = memory;
    });
  }

  Future<void> _pause() async {
    await AnyWPEngine.pauseWallpaper();
    await _refreshStatus();
    _showNotification('Wallpaper paused');
  }

  Future<void> _resume() async {
    await AnyWPEngine.resumeWallpaper();
    await _refreshStatus();
    _showNotification('Wallpaper resumed');
  }

  Future<void> _optimizeMemory() async {
    final before = await AnyWPEngine.getMemoryUsage();
    await AnyWPEngine.optimizeMemory();
    await Future.delayed(Duration(seconds: 1));
    final after = await AnyWPEngine.getMemoryUsage();
    
    final freed = before - after;
    _showNotification('Freed ${freed}MB (${before}MB → ${after}MB)');
    await _refreshStatus();
  }

  void _showNotification(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message)),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Power Control')),
      body: Padding(
        padding: EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Card(
              child: Padding(
                padding: EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('Status', style: Theme.of(context).textTheme.headline6),
                    SizedBox(height: 10),
                    Text('Power State: $_powerState'),
                    Text('Memory Usage: ${_memoryUsage}MB'),
                  ],
                ),
              ),
            ),
            SizedBox(height: 20),
            ElevatedButton.icon(
              onPressed: _pause,
              icon: Icon(Icons.pause),
              label: Text('Pause'),
            ),
            ElevatedButton.icon(
              onPressed: _resume,
              icon: Icon(Icons.play_arrow),
              label: Text('Resume'),
            ),
            ElevatedButton.icon(
              onPressed: _optimizeMemory,
              icon: Icon(Icons.memory),
              label: Text('Optimize Memory'),
            ),
            ElevatedButton.icon(
              onPressed: _refreshStatus,
              icon: Icon(Icons.refresh),
              label: Text('Refresh Status'),
            ),
          ],
        ),
      ),
    );
  }
}
```

---

## Dynamic Configuration

### Example 6: User Preferences

让用户自定义配置：

```dart
class UserPreferences extends StatefulWidget {
  @override
  _UserPreferencesState createState() => _UserPreferencesState();
}

class _UserPreferencesState extends State<UserPreferences> {
  Map<String, dynamic> _config = {};
  bool _autoPowerSaving = true;

  @override
  void initState() {
    super.initState();
    _loadConfig();
  }

  Future<void> _loadConfig() async {
    final config = await AnyWPEngine.getConfiguration();
    setState(() {
      _config = config;
      _autoPowerSaving = config['autoPowerSavingEnabled'] ?? true;
    });
  }

  Future<void> _saveConfig() async {
    await AnyWPEngine.setIdleTimeout(_config['idleTimeoutSeconds']);
    await AnyWPEngine.setMemoryThreshold(_config['memoryThresholdMB']);
    await AnyWPEngine.setCleanupInterval(_config['cleanupIntervalMinutes']);
    await AnyWPEngine.setAutoPowerSaving(_autoPowerSaving);
    
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text('Settings saved!')),
    );
  }

  @override
  Widget build(BuildContext context) {
    if (_config.isEmpty) {
      return Scaffold(body: Center(child: CircularProgressIndicator()));
    }

    return Scaffold(
      appBar: AppBar(title: Text('Preferences')),
      body: ListView(
        children: [
          SwitchListTile(
            title: Text('Auto Power Saving'),
            subtitle: Text('Automatically pause when idle/locked'),
            value: _autoPowerSaving,
            onChanged: (value) {
              setState(() => _autoPowerSaving = value);
            },
          ),
          ListTile(
            title: Text('Idle Timeout'),
            subtitle: Text('${_config['idleTimeoutSeconds']} seconds'),
            trailing: Icon(Icons.edit),
            onTap: () => _editValue('idleTimeoutSeconds', 'seconds'),
          ),
          ListTile(
            title: Text('Memory Threshold'),
            subtitle: Text('${_config['memoryThresholdMB']} MB'),
            trailing: Icon(Icons.edit),
            onTap: () => _editValue('memoryThresholdMB', 'MB'),
          ),
          ListTile(
            title: Text('Cleanup Interval'),
            subtitle: Text('${_config['cleanupIntervalMinutes']} minutes'),
            trailing: Icon(Icons.edit),
            onTap: () => _editValue('cleanupIntervalMinutes', 'minutes'),
          ),
          Padding(
            padding: EdgeInsets.all(20),
            child: ElevatedButton(
              onPressed: _saveConfig,
              child: Text('Save Settings'),
            ),
          ),
        ],
      ),
    );
  }

  Future<void> _editValue(String key, String unit) async {
    final controller = TextEditingController(
      text: _config[key].toString(),
    );

    final result = await showDialog<int>(
      context: context,
      builder: (context) => AlertDialog(
        title: Text('Edit $key'),
        content: TextField(
          controller: controller,
          keyboardType: TextInputType.number,
          decoration: InputDecoration(
            labelText: 'Value ($unit)',
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: Text('Cancel'),
          ),
          TextButton(
            onPressed: () {
              final value = int.tryParse(controller.text);
              Navigator.pop(context, value);
            },
            child: Text('OK'),
          ),
        ],
      ),
    );

    if (result != null) {
      setState(() => _config[key] = result);
    }
  }
}
```

---

## Advanced Integration

### Example 7: Complete Wallpaper Manager

完整的壁纸管理器：

```dart
class WallpaperManager with ChangeNotifier {
  List<MonitorInfo> _monitors = [];
  Map<int, bool> _runningMonitors = {};
  String _powerState = 'UNKNOWN';
  int _memoryUsage = 0;

  List<MonitorInfo> get monitors => _monitors;
  Map<int, bool> get runningMonitors => _runningMonitors;
  String get powerState => _powerState;
  int get memoryUsage => _memoryUsage;

  Future<void> initialize() async {
    // Setup callbacks
    AnyWPEngine.setOnMonitorChangeCallback(() {
      _handleMonitorChange();
    });

    AnyWPEngine.setOnPowerStateChangeCallback((old, newState) {
      _powerState = newState;
      notifyListeners();
    });

    // Load initial state
    await _loadMonitors();
    await _refreshStatus();

    // Configure defaults
    await AnyWPEngine.setIdleTimeout(300);
    await AnyWPEngine.setMemoryThreshold(300);
    await AnyWPEngine.setAutoPowerSaving(true);
  }

  Future<void> _loadMonitors() async {
    _monitors = await AnyWPEngine.getMonitors();
    
    for (var monitor in _monitors) {
      _runningMonitors[monitor.index] = false;
    }
    
    notifyListeners();
  }

  Future<void> _handleMonitorChange() async {
    print('Monitor configuration changed');
    await _loadMonitors();
  }

  Future<void> _refreshStatus() async {
    _powerState = await AnyWPEngine.getPowerState();
    _memoryUsage = await AnyWPEngine.getMemoryUsage();
    notifyListeners();
  }

  Future<bool> startMonitor(int index, String url) async {
    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: url,
      monitorIndex: index,
      enableMouseTransparent: true,
    );

    if (success) {
      _runningMonitors[index] = true;
      notifyListeners();
    }

    return success;
  }

  Future<bool> stopMonitor(int index) async {
    final success = await AnyWPEngine.stopWallpaperOnMonitor(index);
    
    if (success) {
      _runningMonitors[index] = false;
      notifyListeners();
    }

    return success;
  }

  Future<void> startAll(String url) async {
    final results = await AnyWPEngine.initializeWallpaperOnAllMonitors(
      url: url,
      enableMouseTransparent: true,
    );

    results.forEach((index, success) {
      _runningMonitors[index] = success;
    });

    notifyListeners();
  }

  Future<void> stopAll() async {
    await AnyWPEngine.stopWallpaperOnAllMonitors();
    
    _runningMonitors.forEach((key, _) {
      _runningMonitors[key] = false;
    });

    notifyListeners();
  }

  Future<void> pause() async {
    await AnyWPEngine.pauseWallpaper();
    await _refreshStatus();
  }

  Future<void> resume() async {
    await AnyWPEngine.resumeWallpaper();
    await _refreshStatus();
  }

  Future<void> optimizeMemory() async {
    await AnyWPEngine.optimizeMemory();
    await _refreshStatus();
  }
}

// Usage with Provider
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => WallpaperManager()..initialize(),
      child: MaterialApp(
        home: WallpaperScreen(),
      ),
    );
  }
}

class WallpaperScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Consumer<WallpaperManager>(
      builder: (context, manager, child) {
        return Scaffold(
          appBar: AppBar(
            title: Text('Wallpaper Manager'),
            actions: [
              Text('${manager.memoryUsage}MB'),
              IconButton(
                icon: Icon(Icons.memory),
                onPressed: manager.optimizeMemory,
              ),
            ],
          ),
          body: ListView.builder(
            itemCount: manager.monitors.length,
            itemBuilder: (context, i) {
              final monitor = manager.monitors[i];
              final isRunning = manager.runningMonitors[monitor.index] ?? false;

              return ListTile(
                title: Text('Monitor ${monitor.index}'),
                subtitle: Text('${monitor.width}x${monitor.height}'),
                trailing: Switch(
                  value: isRunning,
                  onChanged: (value) async {
                    if (value) {
                      await manager.startMonitor(
                        monitor.index,
                        'https://example.com/wallpaper.html',
                      );
                    } else {
                      await manager.stopMonitor(monitor.index);
                    }
                  },
                ),
              );
            },
          ),
        );
      },
    );
  }
}
```

---

## Next Steps

- Read [Developer API Reference](DEVELOPER_API_REFERENCE.md) for complete API documentation
- See [Best Practices Guide](BEST_PRACTICES.md) for optimization tips
- Check [Troubleshooting](TROUBLESHOOTING.md) if you encounter issues

