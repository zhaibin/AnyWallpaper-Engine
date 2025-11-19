# Auto Recovery 最简使用指南

> ⚠️ **重要**：Auto Recovery 功能**必须**配合 `initializeWallpaperOnMonitor` API 使用！

## 🚀 快速开始（2步）

### Step 1: 启用 Auto Recovery

在应用启动时调用一次：

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 启用 Auto Recovery
  await AnyWPEngine.enableAutoRecovery(true);
  
  runApp(MyApp());
}
```

### Step 2: 使用标准 API 设置壁纸

**关键**：必须使用 `initializeWallpaperOnMonitor` 设置壁纸

```dart
// ✅ 正确的方式
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  monitorIndex: 0,
  // autoSave: true 是默认值，会自动保存配置
);
```

**完成！** Explorer 重启后会自动恢复。

---

## ❌ 常见错误

### 错误 1: 不使用标准 API

```dart
// ❌ 错误：直接创建 WebView，跳过了引擎 API
final webview = await createWebView(...);
```

**结果**：Auto Recovery 完全不工作，因为引擎不知道有壁纸存在。

### 错误 2: 没有启用 Auto Recovery

```dart
// ❌ 忘记调用 enableAutoRecovery
await AnyWPEngine.initializeWallpaperOnMonitor(...);
```

**结果**：配置不会被保存，重启后不恢复。

### 错误 3: 使用了 autoSave: false 但没有手动保存

```dart
// ❌ 禁用了自动保存，但忘记手动保存
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: '...',
  monitorIndex: 0,
  autoSave: false,  // 禁用自动保存
);
// 没有调用 saveCurrentWallpaperConfiguration()
```

**结果**：配置不会被保存。

---

## 📊 验证 Auto Recovery 是否工作

### 方法 1: 查看日志

**应该看到的日志**：

```log
[Engine] Auto recovery ENABLED - Configurations will be saved for recovery
[Engine] Wallpaper configuration saved for monitor 0
[Engine] Auto recovery: 1 configuration(s) saved
```

**如果没有看到**：
1. 检查是否调用了 `enableAutoRecovery(true)`
2. 检查是否使用了 `initializeWallpaperOnMonitor`

### 方法 2: 实际测试

1. 运行应用并设置壁纸
2. 打开任务管理器
3. 找到 "Windows 资源管理器"
4. 右键 → "结束任务"
5. 文件管理器 → 右键任务栏 → "任务管理器"
6. 文件 → "运行新任务" → 输入 `explorer.exe`
7. 检查壁纸是否自动恢复

---

## 🔧 高级场景（可选）

### 场景 1: 轮播壁纸

```dart
// Step 1: 初始化轮播 HTML（不立即保存）
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///carousel.html',
  monitorIndex: 0,
  autoSave: false,  // 等轮播数据加载完再保存
);

// Step 2: 发送轮播数据
await AnyWPEngine.sendMessage(
  message: {'type': 'updateCarousel', 'data': carouselData},
);

// Step 3: 数据发送完成后，手动保存
await AnyWPEngine.saveCurrentWallpaperConfiguration();
```

### 场景 2: 交互式壁纸

```dart
// 初始化时不保存
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///interactive.html',
  monitorIndex: 0,
  autoSave: false,
);

// 用户设置完成后，手动保存
await onUserSettingsComplete() async {
  await AnyWPEngine.sendMessage(
    message: {'type': 'applySettings', 'settings': userSettings},
  );
  
  // 保存配置
  await AnyWPEngine.saveCurrentWallpaperConfiguration();
}
```

---

## ⚠️ 重要提醒

### 必须条件（缺一不可）

1. ✅ 调用 `enableAutoRecovery(true)`
2. ✅ 使用 `initializeWallpaperOnMonitor` 设置壁纸
3. ✅ 配置被保存（autoSave: true 或手动调用 save）

### 不支持的场景

❌ **直接操作 WebView**：引擎无法追踪
❌ **自定义壁纸管理**：必须通过引擎 API
❌ **手动创建窗口**：必须使用引擎的窗口管理

---

## 🐛 故障排查

### 问题: "配置没有保存"

**症状**：日志中看到 `saveCurrentWallpaperConfiguration returned: false`

**原因**：
1. Auto Recovery 未启用
2. 没有活动的壁纸实例

**解决**：
```dart
// 确保顺序正确
await AnyWPEngine.enableAutoRecovery(true);  // 1. 先启用
await AnyWPEngine.initializeWallpaperOnMonitor(...);  // 2. 再初始化
```

### 问题: "Explorer 重启后壁纸不恢复"

**症状**：Explorer 重启后桌面空白

**原因**：配置文件损坏或不存在

**解决**：
1. 检查日志确认配置已保存
2. 重新运行应用并设置壁纸
3. 确保应用有写入权限

---

## 📚 完整示例

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // ✅ Step 1: 启用 Auto Recovery
  await AnyWPEngine.enableAutoRecovery(true);
  
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: WallpaperPage(),
    );
  }
}

class WallpaperPage extends StatelessWidget {
  Future<void> setWallpaper() async {
    // ✅ Step 2: 使用标准 API 设置壁纸
    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: 'file:///C:/wallpapers/my_wallpaper.html',
      monitorIndex: 0,
      // autoSave: true 是默认值
    );
    
    if (success) {
      print('✅ 壁纸设置成功，Auto Recovery 已启用');
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('壁纸设置')),
      body: Center(
        child: ElevatedButton(
          onPressed: setWallpaper,
          child: Text('设置壁纸'),
        ),
      ),
    );
  }
}
```

---

**版本**: v2.4.1  
**更新**: 2025-11-19  
**状态**: ✅ 生产就绪




