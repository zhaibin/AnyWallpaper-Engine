# AnyWP Engine - Documentation

Welcome to the AnyWP Engine documentation! This guide will help you integrate and use AnyWP Engine in your Flutter applications.

## 📚 Documentation Index

### Getting Started
- **[Quick Start Guide](QUICK_START.md)** - Get up and running in 5 minutes
- **[Package Usage Guide (中文)](PACKAGE_USAGE_GUIDE_CN.md)** - 包使用指南
- **[Project Structure](PROJECT_STRUCTURE.md)** - Understand the codebase

### API Documentation
- **[Developer API Reference](DEVELOPER_API_REFERENCE.md)** ⭐ - Complete Dart/Flutter API
- **[API Usage Examples](API_USAGE_EXAMPLES.md)** ⭐ - Practical code examples
- **[Best Practices Guide](BEST_PRACTICES.md)** ⭐ - Optimization and guidelines
- **[Web Developer Guide (中文)](WEB_DEVELOPER_GUIDE_CN.md)** ⭐ - Complete JavaScript SDK guide

### Integration Guides
- **[Integration Architecture](INTEGRATION_ARCHITECTURE.md)** - System architecture overview
- **[API Bridge](API_BRIDGE.md)** - C++, Dart, and JavaScript communication
- **[Web Developer Guide (中文)](WEB_DEVELOPER_GUIDE_CN.md)** - Web 开发者指南
- **[Web Developer Guide](WEB_DEVELOPER_GUIDE.md)** - Web developer guide (English)

### Technical Documentation
- **[Technical Notes](TECHNICAL_NOTES.md)** - Implementation details
- **[Testing Guide](TESTING_GUIDE.md)** - Testing your implementation
- **[Troubleshooting](TROUBLESHOOTING.md)** - Common issues and solutions

### Additional Resources
- **[Cheat Sheet (中文)](CHEAT_SHEET_CN.md)** - Quick reference
- **[SDK Changelog](SDK_CHANGELOG.md)** - SDK version history
- **[Usage Examples](USAGE_EXAMPLES.md)** - More examples

---

## 🚀 Quick Links

### For Flutter Developers

**New to AnyWP Engine?**
1. Read [Quick Start Guide](QUICK_START.md)
2. Check [Developer API Reference](DEVELOPER_API_REFERENCE.md)
3. Try [API Usage Examples](API_USAGE_EXAMPLES.md)
4. Follow [Best Practices](BEST_PRACTICES.md)

**Key Features:**
- 🖥️ Multi-monitor support
- 🔋 Auto power saving (lock/idle/fullscreen detection)
- 💾 Memory optimization
- ⚡ High performance
- 🎯 Interactive wallpapers
- 📱 Easy-to-use Dart API

### For Web Developers

**Creating Wallpaper Content?**
1. Read [Web Developer Guide](WEB_DEVELOPER_GUIDE.md)
2. Check [SDK API Reference](SDK_API_REFERENCE.md)
3. See working examples in [examples/](../examples/) folder

**Available SDKs:**
- AnyWP SDK v4.0.0 (JavaScript)
- React/Vue/Angular compatible
- SPA framework support
- Full event handling

---

## 📖 API Overview

### Basic Usage

```dart
import 'package:anywp_engine/anywp_engine.dart';

// Start wallpaper
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com/wallpaper.html',
);

// Stop wallpaper
await AnyWPEngine.stopWallpaper();
```

### Multi-Monitor Support

```dart
// Get monitors
List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();

// Start on specific monitor
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com/wallpaper.html',
  monitorIndex: 1,
);
```

### Power Management

```dart
// Enable auto power saving
await AnyWPEngine.setAutoPowerSaving(true);

// Manual control
await AnyWPEngine.pauseWallpaper();
await AnyWPEngine.resumeWallpaper();

// Check status
String state = await AnyWPEngine.getPowerState();
int memoryMB = await AnyWPEngine.getMemoryUsage();
```

### Configuration

```dart
// Configure settings
await AnyWPEngine.setIdleTimeout(600);      // 10 minutes
await AnyWPEngine.setMemoryThreshold(200);  // 200MB
await AnyWPEngine.setCleanupInterval(30);   // 30 minutes

// Get current configuration
Map<String, dynamic> config = await AnyWPEngine.getConfiguration();
```

### Callbacks

```dart
// Listen for monitor changes
AnyWPEngine.setOnMonitorChangeCallback(() {
  print('Monitors changed!');
});

// Listen for power state changes
AnyWPEngine.setOnPowerStateChangeCallback((old, newState) {
  print('Power state: $old -> $newState');
});
```

---

## 🎯 Common Scenarios

### Scenario 1: Simple Wallpaper App
→ See [Example 1](API_USAGE_EXAMPLES.md#example-1-simple-wallpaper)

### Scenario 2: Multi-Monitor Setup
→ See [Example 2](API_USAGE_EXAMPLES.md#example-2-different-content-per-monitor)

### Scenario 3: Battery-Aware Application
→ See [Example 4](API_USAGE_EXAMPLES.md#example-4-battery-aware-wallpaper)

### Scenario 4: Advanced Integration
→ See [Example 7](API_USAGE_EXAMPLES.md#example-7-complete-wallpaper-manager)

---

## 🔋 Power Saving Features

AnyWP Engine automatically pauses wallpaper when:

| Scenario | Detection Method | Action | Resume Time |
|----------|------------------|--------|-------------|
| System Locked | `WM_WTSSESSION_CHANGE` | Lightweight pause | **<50ms** ⚡ |
| Screen Off | `WM_POWERBROADCAST` | Lightweight pause | **<50ms** ⚡ |
| Fullscreen App | Polling (2s interval) | Lightweight pause | **<50ms** ⚡ |
| User Idle | `GetLastInputInfo()` | Lightweight pause | **<50ms** ⚡ |

**Lightweight Pause Strategy:**
- ✅ WebView2 stops rendering (saves power)
- ✅ Preserves DOM state and memory
- ✅ Auto-pauses videos and audio
- ✅ Notifies web content via Page Visibility API
- ✅ **Instant resume** when unlocking (no reload)

**Benefits:**
- 🔋 Extended battery life (90% reduction)
- ⚡ Instant resume (<50ms vs 500-1000ms)
- 💾 State preserved (better UX)
- 🎮 No interference with games
- 🎬 Videos/audio auto-pause

---

## 📊 Performance Tips

1. **Web Content Optimization**
   - Use efficient animations (CSS > JavaScript)
   - Optimize images (WebP format)
   - Minimize DOM complexity

2. **Memory Management**
   - Set appropriate thresholds
   - Monitor memory usage
   - Enable periodic cleanup

3. **Power Efficiency**
   - Enable auto power saving
   - Adapt to power source
   - Pause when not visible

4. **Multi-Monitor**
   - Handle monitor changes
   - Test different configurations
   - Support all monitors

See [Best Practices Guide](BEST_PRACTICES.md) for detailed recommendations.

---

## 🐛 Troubleshooting

**Common Issues:**
- Wallpaper not showing → Check [Troubleshooting Guide](TROUBLESHOOTING.md#wallpaper-not-showing)
- High memory usage → See [Memory Management](BEST_PRACTICES.md#memory-management)
- Performance issues → Read [Performance Optimization](BEST_PRACTICES.md#performance-optimization)

---

## 📝 Version History

**Current Version: 1.1.0** 🆕

| Version | Date | Changes |
|---------|------|---------|
| **1.1.0** | 2025-11-05 | **Instant resume** (<50ms) ⚡ |
| - | - | Lightweight pause strategy |
| - | - | Page Visibility API |
| - | - | Auto video/audio pause |
| - | - | Configuration APIs |
| 1.0.0 | 2025-11-03 | Initial release with full features |
| - | - | Multi-monitor support |
| - | - | Power saving & optimization |
| - | - | Comprehensive API |

See [SDK Changelog](SDK_CHANGELOG.md) for detailed version history.

---

## 🤝 Contributing

We welcome contributions! Please:
1. Read the documentation thoroughly
2. Follow [Best Practices](BEST_PRACTICES.md)
3. Test on multiple monitors
4. Update documentation if needed

---

## 📄 License

See [LICENSE](../LICENSE) file for details.

---

## 🔗 Related Links

- [GitHub Repository](https://github.com/yourusername/AnyWallpaper-Engine)
- [Example Application](../example/)
- [HTML Examples](../examples/)
- [Technical Notes](TECHNICAL_NOTES.md)

---

## 💡 Quick Reference

### Essential APIs

| API | Purpose |
|-----|---------|
| `initializeWallpaper()` | Start wallpaper |
| `stopWallpaper()` | Stop wallpaper |
| `getMonitors()` | Get monitor list |
| `pauseWallpaper()` | Pause rendering |
| `getPowerState()` | Check power state |
| `setAutoPowerSaving()` | Configure auto pause |
| `optimizeMemory()` | Free memory |

### Power States

| State | Description |
|-------|-------------|
| `ACTIVE` | Normal operation |
| `IDLE` | User inactive |
| `LOCKED` | System locked |
| `FULLSCREEN_APP` | Fullscreen app running |
| `PAUSED` | Manually paused |

### Configuration Defaults

| Setting | Default | Minimum | Note |
|---------|---------|---------|------|
| Idle Timeout | 300s (5min) | 60s | Time before auto-pause |
| Memory Threshold | **150MB** | 100MB | **Aggressive** (was 300MB) |
| Cleanup Interval | **15min** | 10min | **More frequent** (was 60min) |

**Auto-Optimization Triggers:**
- ✅ After page load (3s delay)
- ✅ After navigation (3s delay)
- ✅ When memory > threshold
- ✅ Every cleanup interval
- ✅ On pause

**Typical Memory Usage:**
- Initial: ~200MB
- After auto-optimize: ~100MB
- Steady state: 80-120MB

---

**Ready to start?** Check out the [Quick Start Guide](QUICK_START.md)!

**Need help?** See [Troubleshooting](TROUBLESHOOTING.md) or open an issue.

**Want examples?** Browse [API Usage Examples](API_USAGE_EXAMPLES.md)!
