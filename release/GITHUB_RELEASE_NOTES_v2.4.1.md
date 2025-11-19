# AnyWP Engine v2.4.1 - Release Notes

**发布日期**: 2025-11-19
**版本**: 2.4.1

---


## ✅ 测试验证结果

**测试时间**: 2025-11-19 10:25  
**测试环境**: Windows 10/11, Flutter 3.0+  
**测试结果**: 🎉 **所有功能验证通过！**

### 自动化测试结果

| 测试项 | 结果 | 说明 |
|--------|------|------|
| `enableAutoRecovery(true)` | ✅ 通过 | C++ 日志确认已启用 |
| 配置自动保存 | ✅ 通过 | Monitor 0 配置已保存 |
| Explorer 重启 | ✅ 通过 | taskkill + 重新启动 |
| 壁纸自动恢复 | ✅ 通过 | 1 个显示器成功恢复 |
| 恢复时间 | ✅ <3秒 | 快速无感知恢复 |

### 关键日志证明

```log
[AutoRecovery] Auto recovery ENABLED
[AutoRecovery] Saved configuration for monitor 0 - URL: https://..., Transparent: true (Total saved: 1)
[WorkerW Recovery] Health monitoring started
[PowerSaving] Wallpaper restoration complete
```

**结论**: Auto Recovery 功能完全符合设计预期，开发者只需 2 行代码即可实现零维护的壁纸自动恢复。

---

## ⚡ 增强：Auto Recovery 配置控制（autoSave 参数）

### 新增功能

为了解决开发者反馈的两个关键问题：
1. **文档缺失基础集成步骤** - 开发者不知道需要调用 `initializeWallpaperOnMonitor`
2. **轮播/交互式壁纸频繁变化** - 每次变化都保存配置会导致性能浪费

新增了精细化的配置保存控制：

##### 1. `initializeWallpaperOnMonitor` 新增 `autoSave` 参数 ⭐
- **默认值**: `true`（保持简单场景的易用性）
- **使用场景**:
  - `autoSave: true` - 简单静态壁纸，初始化后立即保存
  - `autoSave: false` - 轮播/交互式壁纸，延迟到合适时机保存

##### 2. 新增 `saveCurrentWallpaperConfiguration()` API ⭐
- 手动保存当前壁纸配置
- 支持指定显示器或保存所有显示器
- 配合 `autoSave: false` 使用，精确控制保存时机

##### 3. 新增完整指南文档
- 创建 `docs/AUTO_RECOVERY_GUIDE.md` - 120+ 行完整指南
- 包含 3 种典型场景的完整代码示例
- 包含故障排查和最佳实践

### 典型使用场景

**场景 1：简单静态壁纸（默认行为）**
```dart
// 自动保存，无需额外代码
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com/wallpaper.jpg',
  monitorIndex: 0,
);
```

**场景 2：轮播壁纸（控制保存时机）**
```dart
// Step 1: 初始化轮播 HTML（不自动保存）
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///C:/carousel.html',
  monitorIndex: 0,
  autoSave: false,  // 关键：避免频繁保存
);

// Step 2: 发送图片列表
await AnyWPEngine.sendMessage({
  'type': 'updateCarousel',
  'data': {'images': [...], 'interval': 60000},
});

// Step 3: 现在保存配置（一次性）
await AnyWPEngine.saveCurrentWallpaperConfiguration();

// Step 4: 切换图片时无需重新保存
await AnyWPEngine.sendMessage({'type': 'next'});
```

**场景 3：交互式壁纸（用户确认后保存）**
```dart
// 初始化时不保存
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///C:/interactive.html',
  monitorIndex: 0,
  autoSave: false,
);

// 用户点击"应用"按钮后保存
onApplyClick() async {
  await AnyWPEngine.sendMessage({
    'type': 'updateSettings',
    'data': userSettings,
  });
  
  // 保存配置
  final success = await AnyWPEngine.saveCurrentWallpaperConfiguration();
  if (success) {
    showSnackBar('设置已保存');
  }
}
```

### C++ 实现

- `InitializeWallpaperOnMonitor` 新增 `auto_save` 参数（默认 `true`）
- `SaveWallpaperConfigurationManually` 新方法，支持手动保存指定显示器
- FlutterBridge 注册 `saveWallpaperConfiguration` 方法处理器

### 文档更新

- ✅ `docs/AUTO_RECOVERY_GUIDE.md` - 全新 120+ 行完整指南
- ✅ `lib/anywp_engine.dart` - 更新 API 文档，包含 3 个详细示例
- ✅ `CHANGELOG_CN.md` - 详细记录新功能和使用场景

### 向后兼容性

- ✅ 完全向后兼容 - `autoSave` 默认为 `true`
- ✅ 现有代码无需修改即可正常工作
- ✅ 开发者可选择性使用新参数获得更精细的控制

---

