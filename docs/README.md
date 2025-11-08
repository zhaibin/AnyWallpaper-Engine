# AnyWP Engine - Documentation Center

**欢迎！** 选择你的开发者类型，快速找到需要的文档。

---

## 👥 我是哪类开发者？

### 📱 类型 1: Flutter 应用开发者

**你的目标：** 将 AnyWP Engine 集成到你的 Flutter Windows 应用中

**你需要：** Dart API 文档、集成指南、最佳实践

**开始 →** [📱 Flutter 开发者文档导航](FOR_FLUTTER_DEVELOPERS.md)

---

### 🌐 类型 2: Web 开发者

**你的目标：** 开发可作为桌面壁纸的网页内容（HTML/React/Vue等）

**你需要：** JavaScript SDK 文档、框架集成教程、示例代码

**开始 →** [🌐 Web 开发者文档导航](FOR_WEB_DEVELOPERS.md)

---

## 📚 核心文档快速链接

### For Flutter Developers 📱

| 文档 | 用途 | 优先级 |
|-----|------|--------|
| [DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md) | Dart API 完整参考 | ⭐⭐⭐ |
| [API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md) | 7个实用示例 | ⭐⭐⭐ |
| [BEST_PRACTICES.md](BEST_PRACTICES.md) | 最佳实践指南 | ⭐⭐ |
| [QUICK_START.md](QUICK_START.md) | 快速开始 | ⭐ |
| [PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md) | 集成方式 | ⭐ |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | 故障排查 | ⭐ |

### For Web Developers 🌐

| 文档 | 用途 | 优先级 |
|-----|------|--------|
| [WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md) | JS SDK 完整指南 | ⭐⭐⭐ |
| [WEB_DEVELOPER_GUIDE.md](WEB_DEVELOPER_GUIDE.md) | JS SDK Guide (EN) | ⭐⭐⭐ |
| [API_BRIDGE.md](API_BRIDGE.md) | 技术实现原理 | ⭐ |
| examples/*.html | 示例代码 | ⭐⭐ |

### Technical & Architecture 🔧

| 文档 | 用途 | 状态 |
|-----|------|------|
| [INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md) | 系统架构 | ⭐ v1.3.2 模块化 |
| [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) | 技术细节 | ⭐ v1.3.2 模块化 |
| [API_BRIDGE.md](API_BRIDGE.md) | C++/JS 桥接 | ⭐ v1.3.2 模块化 |
| [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) | 项目结构 | ⭐ v1.3.2 模块化 |

### General 📖

| 文档 | 用途 |
|-----|------|
| [CHANGELOG_CN.md](../CHANGELOG_CN.md) | 更新日志 |
| [SDK_CHANGELOG.md](SDK_CHANGELOG.md) | SDK 版本历史 |
| [CHEAT_SHEET_CN.md](CHEAT_SHEET_CN.md) | 速查表 |
| [TESTING_GUIDE.md](TESTING_GUIDE.md) | 测试指南 |

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

**Current Version: 1.3.2-dev** 🆕

| Version | Date | Changes |
|---------|------|---------|
| **1.3.2** | 2025-11-07 | 🏗️ **C++ 模块化重构** |
| - | - | 核心代码从 4000+ 行拆分为 8 个模块 |
| - | - | 单一职责、低耦合、易测试 |
| - | - | ✅ 外部 API 完全向后兼容 |
| - | - | ✅ 对 Flutter/Web 开发者透明 |
| **1.3.1** | 2025-11-07 | 🖥️ 显示器热插拔完整实现 |
| **1.3.0** | 2025-11-07 | 🔥 会话切换与多显示器稳定性提升 |
| **1.2.0** | 2025-11-06 | 🗂️ 应用级存储隔离 + 测试 UI 优化 |
| **1.1.0** | 2025-11-05 | **Instant resume** (<50ms) ⚡ |
| - | - | Lightweight pause strategy |
| - | - | Page Visibility API |
| 1.0.0 | 2025-11-03 | Initial release with full features |

**v1.3.2 架构改进详情**:
- 📁 新增 `windows/modules/` - 5 个功能模块
- 📁 新增 `windows/utils/` - 3 个工具类
- 📖 已更新 4 个核心技术文档
- 🔧 构建系统自动集成所有模块

See [CHANGELOG_CN.md](../CHANGELOG_CN.md) for complete version history.

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
