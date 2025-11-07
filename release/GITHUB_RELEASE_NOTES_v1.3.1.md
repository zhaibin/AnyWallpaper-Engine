# AnyWP Engine v1.3.1 - 显示器热插拔完整实现 🎯

## 🌟 发布亮点

本次更新完整实现了显示器热插拔的自动化管理：
- ✅ **智能配置记忆** - 拔掉显示器前自动保存配置，插回时精确恢复
- ✅ **URL 失败回退** - 加载失败时自动尝试其他可用 URL，防死循环保护
- ✅ **窗口位置保持** - 解决 Windows 系统在显示器变化时窗口跳动问题

---

## ✨ 新增功能

### 1. 显示器配置记忆

**功能说明**：
- 拔掉显示器前自动保存该显示器的 URL 和运行状态
- 插回显示器时自动识别并恢复原有配置
- 基于 `deviceName`（如 `\\.\DISPLAY2`）精确识别显示器，不依赖索引

**使用场景**：
```
场景：双显示器不同内容
- 主显示器运行 test_simple.html
- 副显示器运行 test_draggable.html
- 🔌 拔掉副显示器
  → 系统自动保存：💾 URL=test_draggable.html, Running=true
- 🔌 插回副显示器
  → 系统自动恢复：📂 找到保存的配置
  → 副显示器显示 test_draggable.html（不是 test_simple.html）✓
```

**技术实现**：
```dart
// 拔掉前保存配置（在 _loadMonitors() 清理前）
_monitorConfigMemory[deviceName] = MonitorConfig(
  url: url,
  wasRunning: wasRunning,
  lastSeen: DateTime.now(),
);

// 插回时恢复配置
if (_monitorConfigMemory.containsKey(newMonitor.deviceName)) {
  if (savedConfig.wasRunning) {
    urlToUse = savedConfig.url;  // 使用保存的 URL
  }
}
```

### 2. URL 失败回退机制

**功能说明**：
- URL 加载失败时自动尝试主显示器的 URL
- 只使用**已成功运行**的 URL 做回退源，防止死循环
- 多级保护：主显示器也失败则停止尝试

**使用场景**：
```
场景：URL 失败回退
- 主显示器运行 test_simple.html（成功 ✓）
- 副显示器保存了错误配置 error.html（不存在）
- 🔌 插回副显示器
  → 尝试：error.html → ❌ FAILED
  → 回退：test_simple.html → ✅ SUCCESS
  → 副显示器使用主显示器的 URL ✓
```

**防死循环保护**：
```dart
String? _getPrimaryMonitorUrl() {
  // 只返回正在运行的壁纸 URL
  if (_monitorWallpapers[monitor.index] == true) {
    return url;  // 已验证成功的 URL
  }
  return null;  // 不返回失败的 URL
}
```

### 3. 窗口位置保存

**功能说明**：
- 使用 `window_manager` 包管理窗口位置
- 显示器变化前自动保存窗口位置
- 显示器变化后 500ms 自动恢复到原位置
- 解决 Windows 系统在显示器变化时窗口跳动问题

**使用场景**：
```
场景：窗口位置保持
- 双显示器状态，窗口在位置 A
- 🔌 拔掉副显示器 → 单显示器
- 用户拖动窗口到位置 B
- 🔌 插回副显示器
  → Windows 尝试移动窗口回位置 A
  → 500ms 后自动恢复到位置 B ✓
  → 窗口保持在用户设置的位置 ✓
```

**技术实现**：
```dart
class _MyAppState extends State<MyApp> with WindowListener {
  Offset? _savedWindowPosition;
  
  @override
  void initState() {
    super.initState();
    windowManager.addListener(this);
  }
  
  @override
  void onWindowMoved() async {
    _savedWindowPosition = await windowManager.getPosition();
  }
  
  // 显示器变化后恢复位置
  Future.delayed(Duration(milliseconds: 500), () async {
    await windowManager.setPosition(_savedWindowPosition!);
  });
}
```

---

## 🔧 技术改进

### 关键修复：配置保存时机

**问题**：副屏恢复时使用主屏 URL 而不是保存的 URL

**根本原因**：配置保存的时机错误
```dart
// 之前：在 _loadMonitors() 之后保存（controllers 已清理）
await _loadMonitors();
// 保存配置 ❌ 太晚了

// 现在：在 _loadMonitors() 之前保存（controllers 还存在）
final newMonitors = await AnyWPEngine.getMonitors();
// 立即保存配置 ✅ 正确时机
await _loadMonitors();
```

### 智能恢复策略

优先级系统：
1. **优先级 1**：恢复保存的配置（基于 deviceName）
2. **优先级 2**：使用当前活跃的壁纸
3. **优先级 3**：不自动启动（无运行壁纸时）

```dart
// 优先级 1：恢复保存的配置
if (_monitorConfigMemory.containsKey(newMonitor.deviceName)) {
  if (savedConfig.wasRunning) {
    urlToUse = savedConfig.url;
  }
}

// 优先级 2：使用当前活跃的壁纸
else if (hasActiveWallpaper) {
  urlToUse = activeWallpaperUrl;
}

// 优先级 3：不自动启动
else {
  // 不应用壁纸
}
```

---

## 📚 新增依赖

```yaml
dependencies:
  window_manager: ^0.3.7  # 窗口位置管理（仅 example）
```

---

## 🔍 调试日志

新增完整的 emoji 标记日志系统，便于调试和追踪：

```
💾 Saved config for \\.\DISPLAY2:
   URL: file:///副屏页面.html
   Running: true

📂 Found saved config for \\.\DISPLAY2:
   Saved URL: file:///副屏页面.html
   Was Running: true
   Last Seen: 2025-11-07 ...

✅ Will RESTORE previous wallpaper on monitor 1

▶️ Starting wallpaper on monitor 1
   Device: \\.\DISPLAY2
   URL: file:///副屏页面.html
   Controller updated with URL
   Result: ✅ SUCCESS

🔄 Using active wallpaper URL from monitor 0: ...
⚠️ No active wallpaper found to copy
❌ No saved config found for ...
🔍 No saved config or wasn't running, checking for active wallpapers...
```

---

## 🎯 完整使用示例

### 基础配置

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'package:window_manager/window_manager.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 初始化窗口管理器（防止跳动）
  await windowManager.ensureInitialized();
  
  // 设置应用名称
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  runApp(MyApp());
}

class _MyAppState extends State<MyApp> with WindowListener {
  Map<String, MonitorConfig> _monitorConfigMemory = {};
  Offset? _savedWindowPosition;
  
  @override
  void initState() {
    super.initState();
    
    // 注册窗口监听器
    windowManager.addListener(this);
    
    // 启动显示器轮询（每 3 秒）
    Timer.periodic(Duration(seconds: 3), (timer) {
      _checkMonitorChanges();
    });
  }
  
  @override
  void onWindowMoved() async {
    _savedWindowPosition = await windowManager.getPosition();
  }
}
```

### 场景 1：双显示器不同内容

```dart
// 主显示器运行 Simple
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_simple.html',
  monitorIndex: 0,
);

// 副显示器运行 Draggable
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_draggable.html',
  monitorIndex: 1,
);

// 🔌 拔掉副显示器
// → 自动保存：💾 Saved config for \\.\DISPLAY2: URL=test_draggable.html, Running=true

// 🔌 插回副显示器
// → 自动恢复：📂 Found saved config, ✅ Will RESTORE: test_draggable.html
// → 副显示器显示 Draggable（不是 Simple）✓
```

### 场景 2：URL 失败回退

```dart
// 主显示器运行正常页面
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_simple.html',
  monitorIndex: 0,  // 成功 ✓
);

// 保存了错误的副显示器配置
_monitorConfigMemory['\\.\DISPLAY2'] = MonitorConfig(
  url: 'file:///error.html',  // 不存在的文件
  wasRunning: true,
);

// 🔌 插回副显示器
// → 尝试：file:///error.html → ❌ FAILED
// → 回退：file:///test_simple.html → ✅ SUCCESS
// → 副显示器使用主显示器的 URL ✓
```

---

## ⚠️ 注意事项

1. **显示器识别**：基于 `deviceName`（如 `\\.\DISPLAY1`）而非索引
2. **配置持久化**：仅在内存中保存，应用重启后需重新学习
3. **窗口位置**：需要 `window_manager` 包，500ms 延迟避免与 Windows 冲突
4. **回退保护**：只使用运行成功的 URL 做回退源，防止死循环
5. **轮询间隔**：3 秒检查一次，平衡响应速度和性能

---

## 🎬 测试场景验证

| 场景 | 预期行为 | 测试状态 |
|------|----------|----------|
| 双显示器不同内容 | 拔插后各自恢复原内容 | ✅ 通过 |
| URL 失败回退 | 自动尝试主显示器 URL | ✅ 通过 |
| 主副都失败 | 停止尝试，不死循环 | ✅ 通过 |
| 窗口位置保持 | 拔插后位置不变 | ✅ 通过 |
| 无运行壁纸插入 | 不自动启动 | ✅ 通过 |

---

## 🐛 修复的问题

1. **副屏配置丢失** - 修复了保存时机，确保在 controllers 清理前保存
2. **URL 回退死循环** - 只使用成功运行的 URL 做回退
3. **窗口位置跳动** - 使用 `window_manager` 自动恢复位置
4. **配置混淆** - 使用 `deviceName` 精确匹配，不依赖索引

---

## 📦 安装和升级

### 新用户

从 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases) 下载 `anywp_engine_v1.3.1.zip`，解压后运行：

```powershell
# 在 Flutter 项目根目录执行
packages\anywp_engine_v1.3.1\setup_precompiled.bat
```

### 现有用户升级

1. 备份当前配置
2. 下载新版本 ZIP
3. 解压并执行 `setup_precompiled.bat`
4. 添加 `window_manager` 依赖（如果使用示例代码）

```yaml
dependencies:
  window_manager: ^0.3.7
```

---

## 📄 完整文档

- **Flutter 开发者**: [DEVELOPER_API_REFERENCE.md](docs/DEVELOPER_API_REFERENCE.md)
- **Web 开发者**: [WEB_DEVELOPER_GUIDE_CN.md](docs/WEB_DEVELOPER_GUIDE_CN.md)
- **集成指南**: [PRECOMPILED_DLL_INTEGRATION.md](docs/PRECOMPILED_DLL_INTEGRATION.md)
- **更新日志**: [CHANGELOG_CN.md](CHANGELOG_CN.md)

---

## 🙏 致谢

感谢所有测试和反馈的用户！特别感谢对多显示器场景的详细测试。

---

**完整 CHANGELOG**: [CHANGELOG_CN.md](CHANGELOG_CN.md#480---2025-11-07---显示器热插拔完整实现)

