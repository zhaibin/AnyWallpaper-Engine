# Auto Recovery 故障排查指南

## 📋 问题现象

启用了 `enableAutoRecovery(true)` 后，Explorer 重启时壁纸没有自动恢复。

---

## 🔍 排查步骤

### Step 1: 确认 Auto Recovery 已启用

**检查日志中是否有以下信息**：

```
[AnyWP] [AutoRecovery] Auto recovery ENABLED
```

**如果没有此日志**：
- ✅ 确认在 `main()` 中调用了 `await AnyWPEngine.enableAutoRecovery(true);`
- ✅ 确认调用在 `runApp()` 之前
- ✅ 确认没有异常导致调用失败

**正确示例**：
```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // ✅ 在这里启用自动恢复
  await AnyWPEngine.enableAutoRecovery(true);
  
  runApp(MyApp());
}
```

---

### Step 2: 确认配置已保存

**检查日志中是否有以下信息**（初始化壁纸后）：

```
[AnyWP] [AutoRecovery] Saved configuration for monitor 0 - URL: https://example.com, Transparent: true (Total saved: 1)
```

**如果没有此日志**：
- ❌ **问题**：Auto Recovery 在初始化壁纸时未启用
- ✅ **解决**：确保在初始化壁纸 **之前** 调用 `enableAutoRecovery(true)`

**错误示例** ❌：
```dart
// 错误：先初始化壁纸，后启用恢复
await AnyWPEngine.initializeWallpaperOnMonitor(url: url, monitorIndex: 0);
await AnyWPEngine.enableAutoRecovery(true);  // 太晚了！配置已经不会被保存
```

**正确示例** ✅：
```dart
// 正确：先启用恢复，再初始化壁纸
await AnyWPEngine.enableAutoRecovery(true);
await AnyWPEngine.initializeWallpaperOnMonitor(url: url, monitorIndex: 0);
```

---

### Step 3: 模拟 Explorer 重启

**手动测试步骤**：

1. 运行你的 Flutter 应用
2. 初始化壁纸（确认能看到壁纸）
3. 打开任务管理器（`Ctrl + Shift + Esc`）
4. 找到 `Windows 资源管理器` 进程
5. 右键 → `结束任务`
6. 等待几秒，Explorer 会自动重启

**预期日志**：
```
[AnyWP] [WorkerW Recovery] Auto recovery enabled, triggering automatic wallpaper recovery
[AnyWP] [AutoRecovery] Starting auto recovery for 1 wallpaper(s)
[AnyWP] [AutoRecovery] Will recover: Monitor 0, URL: https://example.com, Transparent: true
[AnyWP] [AutoRecovery] Waiting 1 second for system to stabilize...
[AnyWP] [AutoRecovery] Starting wallpaper recovery...
[AnyWP] [AutoRecovery] Recovering wallpaper on monitor 0 - URL: https://example.com
[AnyWP] [AutoRecovery] Successfully recovered wallpaper on monitor 0
[AnyWP] [AutoRecovery] Auto recovery completed - Success: 1, Failed: 0
```

---

### Step 4: 检查 Flutter 应用状态

**重要提示** ⚠️：

Auto Recovery **仅在 Flutter 应用仍在运行时有效**。

- ✅ **正常情况**：Explorer 重启，Flutter 应用继续运行，壁纸自动恢复
- ❌ **异常情况**：Explorer 重启导致 Flutter 应用也被关闭，无法自动恢复

**确认方法**：
1. Explorer 重启后，检查任务管理器中你的应用是否还在运行
2. 如果应用已关闭，需要重新启动应用

---

## 🐛 常见问题

### 问题 1: 日志中显示 "No saved configurations to recover"

**原因**：
- 启用 Auto Recovery 后，没有初始化任何壁纸
- 或者在启用 Auto Recovery **之前** 就已经初始化了壁纸

**解决方案**：
```dart
// 方案 A: 在应用启动时立即启用（推荐）
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await AnyWPEngine.enableAutoRecovery(true);  // ✅ 优先启用
  runApp(MyApp());
}

// 方案 B: 如果壁纸已初始化，重新初始化以保存配置
await AnyWPEngine.stopWallpaperOnMonitor(0);
await AnyWPEngine.initializeWallpaperOnMonitor(url: url, monitorIndex: 0);
```

---

### 问题 2: 日志中显示 "Auto recovery disabled, skipping"

**原因**：
- `enableAutoRecovery(true)` 未被调用
- 或调用失败

**解决方案**：
1. 确认代码中有 `await AnyWPEngine.enableAutoRecovery(true);`
2. 检查是否有异常日志
3. 确认没有调用 `enableAutoRecovery(false)` 禁用它

---

### 问题 3: 恢复失败（日志显示 "Failed to recover"）

**可能原因**：
1. **URL 无效或无法访问**
   - 检查 URL 是否正确
   - 如果是本地文件，确认路径存在

2. **显示器配置变化**
   - Explorer 重启前后显示器数量或索引发生变化
   - 壁纸尝试恢复到不存在的显示器

3. **WebView2 Runtime 问题**
   - WebView2 未正确安装或损坏

**解决方案**：
- 查看详细错误日志（在 `test_logs/` 目录或 Debug 控制台）
- 尝试手动初始化壁纸，确认 URL 和显示器索引正确

---

### 问题 4: 恢复后壁纸位置或大小不正确

**原因**：
- 系统还未完全稳定时就开始恢复
- 显示器配置在 Explorer 重启时发生变化

**当前行为**：
- 插件会等待 1 秒让系统稳定
- 然后依次恢复每个壁纸

**如果仍有问题**：
- 可以延长等待时间（需修改 C++ 代码）
- 或在恢复后手动调整壁纸位置

---

## 📊 完整调试日志示例

### ✅ 成功案例

```
[APP] Auto recovery enabled
[AnyWP] [AutoRecovery] Auto recovery ENABLED
[AnyWP] [AutoRecovery] No active wallpapers. Configurations will be saved when wallpapers are initialized.

[AnyWP] [Plugin] Initializing Wallpaper on Monitor 0 - URL: https://example.com, Transparent: true
[AnyWP] [Plugin] Initialization Complete (Monitor 0)
[AnyWP] [AutoRecovery] Saved configuration for monitor 0 - URL: https://example.com, Transparent: true (Total saved: 1)

// ... (用户杀死 Explorer)

[AnyWP] [WorkerW Recovery] Explorer restart detected, windows were destroyed
[AnyWP] [WorkerW Recovery] Auto recovery enabled, triggering automatic wallpaper recovery
[AnyWP] [AutoRecovery] Starting auto recovery for 1 wallpaper(s)
[AnyWP] [AutoRecovery] Will recover: Monitor 0, URL: https://example.com, Transparent: true
[AnyWP] [AutoRecovery] Waiting 1 second for system to stabilize...
[AnyWP] [AutoRecovery] Stopping existing wallpapers before recovery
[AnyWP] [AutoRecovery] Found 0 existing wallpaper(s) to stop
[AnyWP] [AutoRecovery] Waiting 500ms for cleanup to complete...
[AnyWP] [AutoRecovery] Starting wallpaper recovery...
[AnyWP] [AutoRecovery] Recovering wallpaper on monitor 0 - URL: https://example.com
[AnyWP] [Plugin] Initializing Wallpaper on Monitor 0 - URL: https://example.com, Transparent: true
[AnyWP] [Plugin] Initialization Complete (Monitor 0)
[AnyWP] [AutoRecovery] Saved configuration for monitor 0 - URL: https://example.com, Transparent: true (Total saved: 1)
[AnyWP] [AutoRecovery] Successfully recovered wallpaper on monitor 0
[AnyWP] [AutoRecovery] Auto recovery completed - Success: 1, Failed: 0
```

### ❌ 失败案例（未启用）

```
// 没有 "Auto recovery ENABLED" 日志
[AnyWP] [Plugin] Initializing Wallpaper on Monitor 0 - URL: https://example.com, Transparent: true
[AnyWP] [Plugin] Initialization Complete (Monitor 0)
[AnyWP] [AutoRecovery] Auto recovery disabled, not saving configuration for monitor 0  // ❌ 问题在这里

// ... (用户杀死 Explorer)

[AnyWP] [WorkerW Recovery] Explorer restart detected, windows were destroyed
[AnyWP] [WorkerW Recovery] Auto recovery disabled, waiting for manual Flutter-side recovery  // ❌ 未启用
// 壁纸不会自动恢复
```

---

## 💡 最佳实践

### ✅ 推荐做法

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 1. 尽早启用 Auto Recovery
  await AnyWPEngine.enableAutoRecovery(true);
  print('[APP] Auto recovery enabled');
  
  // 2. 其他初始化...
  await AnyWPEngine.setApplicationName('MyApp');
  
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  // ...
  
  Future<void> initWallpaper() async {
    // 3. 正常初始化壁纸
    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: 'https://example.com',
      monitorIndex: 0,
    );
    
    if (success) {
      print('[APP] Wallpaper initialized');
      // ✅ 配置会自动保存
    }
  }
}
```

### ❌ 错误做法

```dart
// 错误 1: 在初始化壁纸后才启用
await AnyWPEngine.initializeWallpaperOnMonitor(url: url, monitorIndex: 0);
await AnyWPEngine.enableAutoRecovery(true);  // ❌ 太晚了

// 错误 2: 每次初始化前都调用（会导致重复日志）
await AnyWPEngine.enableAutoRecovery(true);
await AnyWPEngine.initializeWallpaperOnMonitor(url: url, monitorIndex: 0);
// ... 稍后
await AnyWPEngine.enableAutoRecovery(true);  // ❌ 重复调用（虽然不会出错，但没必要）
await AnyWPEngine.initializeWallpaperOnMonitor(url: newUrl, monitorIndex: 0);
```

---

## 🛠️ 高级调试

### 查看完整日志

**Debug 模式**：
- 日志输出到控制台和文件
- 文件位置：`test_logs/debug_run.log`

**查看日志命令**：
```powershell
# 实时监控日志
.\scripts\monitor_log.bat

# 或手动查看
Get-Content test_logs\debug_run.log -Tail 50
```

### 过滤 AutoRecovery 相关日志

```powershell
Get-Content test_logs\debug_run.log | Select-String "AutoRecovery"
```

---

## 📞 获取帮助

如果按照以上步骤仍无法解决问题：

1. **收集信息**：
   - 完整的日志文件（包含启用 → 初始化 → Explorer 重启 → 恢复尝试）
   - Flutter 应用代码片段（`main()` 和壁纸初始化部分）
   - 系统环境（Windows 版本、显示器配置）

2. **提交 Issue**：
   - 访问: https://github.com/zhaibin/AnyWallpaper-Engine/issues
   - 标题: `[Auto Recovery] 简短描述问题`
   - 内容: 包含上述收集的信息

3. **参考文档**：
   - `docs/FOR_FLUTTER_DEVELOPERS.md` - 完整 API 文档
   - `CHANGELOG_CN.md` - v2.4.0 自动恢复功能说明

---

**Version**: 2.4.0  
**Updated**: 2025-11-18




