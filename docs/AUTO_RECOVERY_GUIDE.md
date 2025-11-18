# Auto Recovery 完整指南 (v2.4.0+)

## 📖 简介

Auto Recovery 是 AnyWP Engine v2.4.0+ 引入的自动恢复功能，可以在 Windows Explorer 重启后自动恢复壁纸，无需开发者手动处理。

## 🎯 核心问题

### 问题背景

Windows Explorer (explorer.exe) 可能因以下原因重启：
- 任务管理器中被终止
- 系统崩溃后自动重启
- 系统更新或配置更改

当 Explorer 重启时，所有桌面窗口（包括壁纸 WebView）都会被销毁，导致壁纸消失。

### 传统解决方案（v2.3.x 及更早）

开发者需要手动监听并处理恢复：

```dart
// ❌ 繁琐：需要 ~20 行代码
AnyWPEngine.setOnMessageCallback((message) {
  if (message['type'] == 'WALLPAPER_RECREATE_REQUIRED') {
    // 手动保存状态
    // 手动重新初始化壁纸
    // 手动恢复状态
  }
});
```

### 新解决方案（v2.4.0+）

一行代码启用自动恢复：

```dart
// ✅ 简单：只需 1 行代码
await AnyWPEngine.enableAutoRecovery(true);
```

---

## 🚀 快速开始

### 方案 A：简单静态壁纸（推荐入门）

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 1️⃣ 启用 Auto Recovery
  await AnyWPEngine.enableAutoRecovery(true);
  
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: ElevatedButton(
          child: Text('设置壁纸'),
          onPressed: () async {
            // 2️⃣ 初始化壁纸（默认 autoSave: true）
            await AnyWPEngine.initializeWallpaperOnMonitor(
              url: 'https://example.com/wallpaper.jpg',
              monitorIndex: 0,
            );
            // ✅ 完成！壁纸会自动保存并在 Explorer 重启后恢复
          },
        ),
      ),
    );
  }
}
```

**预期日志**：

```log
[00:00:01.000] [SYSTEM] INFO: Auto Recovery enabled ✅
[00:00:05.000] [Plugin] INFO: Initializing Wallpaper on Monitor 0
[00:00:05.200] [AutoRecovery] INFO: Saved configuration for monitor 0 ✅
  - URL: https://example.com/wallpaper.jpg
  - Transparent: true
  - Total saved: 1

// ... Explorer 重启 ...

[00:01:00.000] [WEBVIEW] INFO: Received message: WALLPAPER_RECREATE_REQUIRED
[00:01:00.100] [AutoRecovery] INFO: Starting auto recovery for 1 wallpaper(s) ✅
[00:01:01.000] [Plugin] INFO: Initializing Wallpaper on Monitor 0
[00:01:01.500] [AutoRecovery] INFO: Successfully recovered wallpaper ✅
[00:01:01.600] [AutoRecovery] INFO: Auto recovery completed - Success: 1, Failed: 0
```

---

### 方案 B：轮播壁纸（控制保存时机）

```dart
class CarouselManager {
  Future<void> startCarousel() async {
    // 1️⃣ 初始化轮播 HTML（不自动保存）
    await AnyWPEngine.initializeWallpaperOnMonitor(
      url: 'file:///C:/wallpapers/carousel.html',
      monitorIndex: 0,
      autoSave: false,  // ⭐ 不要在每次切换时保存
    );
    
    // 2️⃣ 发送初始图片列表
    await AnyWPEngine.sendMessage({
      'type': 'updateCarousel',
      'data': {
        'images': [
          'https://example.com/img1.jpg',
          'https://example.com/img2.jpg',
          'https://example.com/img3.jpg',
        ],
        'interval': 60000,  // 60秒
        'autoPlay': true,
      },
    });
    
    // 3️⃣ ⭐ 现在保存配置（保存 carousel.html 的 URL）
    await AnyWPEngine.saveCurrentWallpaperConfiguration();
    print('✅ 轮播配置已保存，Explorer 重启后会自动恢复');
  }
  
  Future<void> nextWallpaper() async {
    // 切换到下一张（不需要重新保存）
    await AnyWPEngine.sendMessage({'type': 'next'});
  }
}
```

**关键点**：
- 保存的是 `carousel.html` 的 URL，而不是当前显示的图片
- Explorer 重启后会恢复 `carousel.html`，然后由 HTML 内部逻辑继续轮播

---

### 方案 C：交互式壁纸（用户配置后保存）

```dart
class InteractiveWallpaperManager {
  Future<void> initWallpaper() async {
    // 1️⃣ 初始化交互式壁纸（不自动保存）
    await AnyWPEngine.initializeWallpaperOnMonitor(
      url: 'file:///C:/wallpapers/interactive.html',
      monitorIndex: 0,
      autoSave: false,  // ⭐ 等待用户确认配置
    );
    
    // 2️⃣ 发送默认配置
    await AnyWPEngine.sendMessage({
      'type': 'updateSettings',
      'data': {
        'theme': 'dark',
        'widgets': ['clock', 'weather', 'calendar'],
        'opacity': 0.8,
      },
    });
  }
  
  Future<void> onUserClickApply(Map<String, dynamic> userSettings) async {
    // 3️⃣ 用户点击"应用"按钮后，发送新配置
    await AnyWPEngine.sendMessage({
      'type': 'updateSettings',
      'data': userSettings,
    });
    
    // 4️⃣ 等待壁纸应用设置
    await Future.delayed(Duration(milliseconds: 500));
    
    // 5️⃣ ⭐ 现在保存配置
    final success = await AnyWPEngine.saveCurrentWallpaperConfiguration();
    
    if (success) {
      print('✅ 配置已保存，Explorer 重启后会自动恢复');
      showSnackBar('设置已保存');
    } else {
      print('❌ 保存失败，请检查 Auto Recovery 是否已启用');
    }
  }
}
```

**关键点**：
- 用户可以随意调整设置（不会频繁保存）
- 只有在用户明确点击"应用"或"保存"时才保存配置
- 保存的是壁纸 URL（`interactive.html`），壁纸内部可以使用 `AnyWP.saveState()` 保存自己的状态

---

## 📚 API 参考

### 1. `enableAutoRecovery(bool enabled)`

启用或禁用自动恢复功能。

```dart
// 启用
await AnyWPEngine.enableAutoRecovery(true);

// 禁用
await AnyWPEngine.enableAutoRecovery(false);
```

**注意**：
- 必须在 `main()` 中尽早调用
- 启用后，所有后续的 `initializeWallpaperOnMonitor` 调用（autoSave=true）都会自动保存配置
- 禁用后，已保存的配置会被清除

---

### 2. `initializeWallpaperOnMonitor({url, monitorIndex, autoSave})`

初始化壁纸，可选择是否自动保存配置。

```dart
// 自动保存（默认）
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com/wallpaper.jpg',
  monitorIndex: 0,
  autoSave: true,  // 默认值
);

// 不自动保存（手动控制）
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///C:/carousel.html',
  monitorIndex: 0,
  autoSave: false,  // 稍后手动调用 saveCurrentWallpaperConfiguration()
);
```

**参数**：
- `url` (String): 壁纸 URL
- `monitorIndex` (int): 显示器索引（0开始）
- `autoSave` (bool, 可选): 是否自动保存配置，默认 `true`

**返回**：
- `true`: 成功
- `false`: 失败

---

### 3. `saveCurrentWallpaperConfiguration({monitorIndex})`

手动保存当前壁纸配置。

```dart
// 保存所有显示器的配置
await AnyWPEngine.saveCurrentWallpaperConfiguration();

// 保存指定显示器的配置
await AnyWPEngine.saveCurrentWallpaperConfiguration(monitorIndex: 0);
```

**参数**：
- `monitorIndex` (int, 可选): 显示器索引，默认 `-1`（所有显示器）

**返回**：
- `true`: 成功保存
- `false`: 失败（Auto Recovery 未启用或没有活动壁纸）

**使用时机**：
- ✅ 用户确认轮播选择
- ✅ 用户应用交互式壁纸设置
- ✅ 壁纸状态初始化完成
- ❌ 不要在每次轮播切换时调用
- ❌ 不要在频繁更新时调用

---

### 4. `isAutoRecoveryEnabled()`

检查 Auto Recovery 是否已启用。

```dart
final isEnabled = await AnyWPEngine.isAutoRecoveryEnabled();
print('Auto Recovery: ${isEnabled ? "ON" : "OFF"}');
```

**返回**：
- `true`: 已启用
- `false`: 未启用

---

## 🔍 故障排查

### 问题 1：壁纸没有自动恢复

**可能原因**：

1. **忘记启用 Auto Recovery**

```dart
// ❌ 错误：没有启用
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  // 缺少这一行 ⬇️
  // await AnyWPEngine.enableAutoRecovery(true);
  runApp(MyApp());
}
```

**解决方案**：在 `main()` 中添加 `await AnyWPEngine.enableAutoRecovery(true);`

---

2. **使用了 `autoSave: false` 但忘记手动保存**

```dart
// ❌ 错误：初始化时不保存，但后续也没有手动保存
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///carousel.html',
  monitorIndex: 0,
  autoSave: false,
);
// 缺少 ⬇️
// await AnyWPEngine.saveCurrentWallpaperConfiguration();
```

**解决方案**：调用 `saveCurrentWallpaperConfiguration()`

---

3. **检查日志**

查看日志确认配置是否已保存：

```log
// ✅ 成功的日志
[AutoRecovery] INFO: Saved configuration for monitor 0 - URL: ..., Total saved: 1

// ❌ 失败的日志
[Plugin] INFO: AutoSave disabled, configuration not saved automatically
```

---

### 问题 2：轮播壁纸恢复后显示错误的图片

**说明**：
- Auto Recovery 保存的是**壁纸 URL**（例如 `carousel.html`）
- 不会保存**当前显示的图片**
- 恢复后，轮播会从头开始或根据 HTML 内部逻辑恢复

**解决方案**：
如果需要恢复到具体图片，可以在 HTML 中使用 `AnyWP.saveState()` 保存当前索引：

```javascript
// 在 carousel.html 中
let currentIndex = 0;

function nextImage() {
  currentIndex++;
  // 保存当前索引
  AnyWP.saveState(JSON.stringify({ currentIndex }));
}

// 页面加载时恢复
window.addEventListener('load', async () => {
  const state = await AnyWP.loadState();
  if (state) {
    const { currentIndex: savedIndex } = JSON.parse(state);
    currentIndex = savedIndex;
  }
});
```

---

### 问题 3：日志显示"配置已保存"但恢复时失败

**可能原因**：保存的 URL 路径无效（文件不存在）

```dart
// ❌ 错误：路径拼写错误
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///C:/wallpapers/carouusel.html',  // ⬅️ 拼写错误
  monitorIndex: 0,
);
```

**解决方案**：
- 使用绝对路径
- 确保文件存在
- 检查恢复日志中的错误信息

---

## 📊 对比表：v2.3.x vs v2.4.0+

| 特性 | v2.3.x (手动) | v2.4.0+ (自动) |
|------|---------------|----------------|
| **代码量** | ~20 行 | 1 行 |
| **复杂度** | 高（需要理解消息机制） | 低（一行启用） |
| **多显示器** | 需手动处理每个显示器 | 自动处理所有显示器 |
| **状态管理** | 开发者手动保存/恢复 | 引擎自动保存/恢复 |
| **错误处理** | 开发者负责 | 引擎内置重试和延迟 |
| **时机控制** | 无法控制 | 可选 `autoSave` 参数 |

---

## 🎓 最佳实践

### ✅ 推荐

1. **在 `main()` 中尽早启用 Auto Recovery**

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await AnyWPEngine.enableAutoRecovery(true);  // ⭐ 第一件事
  runApp(MyApp());
}
```

2. **简单壁纸使用默认 `autoSave: true`**

```dart
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com/wallpaper.jpg',
  monitorIndex: 0,
  // autoSave: true 是默认值，可以省略
);
```

3. **频繁变化的壁纸使用 `autoSave: false` + 手动保存**

```dart
// 初始化时不保存
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///carousel.html',
  monitorIndex: 0,
  autoSave: false,
);

// 用户确认后保存
await AnyWPEngine.saveCurrentWallpaperConfiguration();
```

4. **提供用户可见的保存反馈**

```dart
final success = await AnyWPEngine.saveCurrentWallpaperConfiguration();
if (success) {
  ScaffoldMessenger.of(context).showSnackBar(
    SnackBar(content: Text('壁纸设置已保存')),
  );
}
```

---

### ❌ 避免

1. **不要在循环或定时器中频繁保存**

```dart
// ❌ 错误：每秒保存一次
Timer.periodic(Duration(seconds: 1), (_) {
  AnyWPEngine.saveCurrentWallpaperConfiguration();  // 性能浪费
});
```

2. **不要在每次轮播切换时保存**

```dart
// ❌ 错误：每次切换都保存
void nextImage() {
  currentIndex++;
  AnyWPEngine.sendMessage({'type': 'next'});
  AnyWPEngine.saveCurrentWallpaperConfiguration();  // 不必要
}
```

3. **不要忘记启用 Auto Recovery**

```dart
// ❌ 错误：调用了 saveCurrentWallpaperConfiguration() 但没有先启用 Auto Recovery
await AnyWPEngine.saveCurrentWallpaperConfiguration();  // 会返回 false
```

---

## 🔗 相关文档

- [Flutter 开发者完整指南](FOR_FLUTTER_DEVELOPERS.md)
- [API 参考文档](DEVELOPER_API_REFERENCE.md)
- [CHANGELOG](../CHANGELOG_CN.md)

---

## 📝 常见问题 FAQ

**Q: Auto Recovery 会影响性能吗？**

A: 不会。配置保存是内存操作（只在初始化时保存 URL 和参数），恢复操作只在 Explorer 重启时触发（极少发生）。

**Q: 可以保存壁纸的内部状态吗？**

A: Auto Recovery 只保存壁纸 URL 和基本参数。如果需要保存壁纸内部状态（如轮播索引、用户设置），请在 HTML 中使用 `AnyWP.saveState()` API。

**Q: 支持多显示器吗？**

A: 完全支持。每个显示器的配置独立保存和恢复。

**Q: 恢复需要多久？**

A: 通常 1-2 秒（包括 1 秒系统稳定延迟 + WebView 创建时间）。

**Q: 可以禁用 Auto Recovery 吗？**

A: 可以。调用 `await AnyWPEngine.enableAutoRecovery(false);` 即可禁用并清除所有已保存的配置。

---

**最后更新**: 2025-11-19  
**适用版本**: v2.4.0+


