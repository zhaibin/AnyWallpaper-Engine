# 全屏应用暂停功能测试指南

## 功能说明

AnyWP Engine 会自动检测全屏应用（游戏、视频播放器、全屏浏览器等），并在检测到全屏应用时自动暂停壁纸渲染以节省资源。当全屏应用关闭后，壁纸会自动恢复。

## 测试步骤

### 1. 启动示例应用

```bash
cd example
flutter run -d windows
```

或直接运行编译好的程序：
```bash
.\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

### 2. 加载测试壁纸

在应用界面中：

1. 点击 **"🎯 Fullscreen Pause"** 按钮加载专用测试页面
2. 或选择其他任意测试页面（如 "👁️ Visibility"）
3. 等待壁纸加载完成（看到动画或内容）

### 3. 测试全屏暂停

#### 方法 A：使用浏览器全屏（推荐）

1. 打开任意浏览器（Edge、Chrome、Firefox等）
2. 按 **F11** 进入全屏模式
3. 观察：
   - 示例应用的 UI 应显示 "Power State: FULLSCREEN_APP"
   - 如果加载了 `test_fullscreen_pause.html`，暂停计数器应 +1
4. 按 **F11** 或 **ESC** 退出全屏
5. 观察：
   - 示例应用显示 "Power State: ACTIVE"
   - 恢复计数器应 +1

#### 方法 B：使用视频播放器

1. 打开任意视频播放器（如 VLC、PotPlayer等）
2. 点击全屏按钮或按 **F** 键
3. 观察状态变化（同上）
4. 退出全屏
5. 观察恢复状态

#### 方法 C：使用游戏

1. 启动任意全屏游戏
2. 观察状态变化
3. 退出游戏或切换到窗口模式
4. 观察恢复状态

### 4. 查看日志

日志文件位于：`test_logs/debug_run.log`

关键日志标识：
```
[PowerManager] Fullscreen app detected: "应用名称" (Class: 窗口类名)
[PowerManager] State changed: 0 -> 4  // 0=ACTIVE, 4=FULLSCREEN_APP
[PowerManager] Pause requested: PowerManager: fullscreen_app
[PowerSaving] Wallpaper paused - last frame frozen
[PowerManager] State changed: 4 -> 0  // 恢复
[PowerManager] Resume requested: PowerManager: fullscreen_closed
[PowerSaving] Wallpaper resumed - animations restarted
```

## 预期行为

### 暂停时 (Fullscreen App Active)

✅ **C++ 端**：
- 检测到全屏应用
- 触发状态变化：ACTIVE → FULLSCREEN_APP
- 调用 `PauseWallpaper()`
- 执行暂停脚本

✅ **Web 端**（如果壁纸支持）：
- 所有 CSS 动画暂停
- 所有 video/audio 元素暂停
- `requestAnimationFrame` 被阻止
- 收到 `AnyWP:visibility` 事件（`visible: false`）
- `AnyWP.onVisibilityChange()` 回调被触发

✅ **Dart 端**：
- 收到 power state change 事件
- UI 显示 "Power State: FULLSCREEN_APP"

### 恢复时 (Fullscreen App Closed)

✅ **C++ 端**：
- 检测到全屏关闭
- 触发状态变化：FULLSCREEN_APP → ACTIVE
- 调用 `ResumeWallpaper()`
- 执行恢复脚本

✅ **Web 端**：
- CSS 动画恢复
- video/audio 元素恢复播放（如果之前在播放）
- `requestAnimationFrame` 恢复
- 收到 `AnyWP:visibility` 事件（`visible: true`）
- `AnyWP.onVisibilityChange()` 回调被触发

✅ **Dart 端**：
- 收到 power state change 事件
- UI 显示 "Power State: ACTIVE"

## 常见问题排查

### 问题 1：暂停计数器不变化

**可能原因**：
1. 壁纸页面还未加载完成
2. 壁纸页面没有注册 visibility 回调
3. SDK 未正确注入

**解决方法**：
1. 确保壁纸已成功加载（能看到内容/动画）
2. 使用专用测试页面 `test_fullscreen_pause.html`
3. 按 F12 打开 DevTools（如果可用）查看控制台输出

### 问题 2：Power State 始终是 ACTIVE

**可能原因**：
1. 全屏窗口未完全覆盖屏幕
2. 全屏窗口是桌面或系统窗口（被过滤）
3. PowerManager 未启用

**解决方法**：
1. 确保应用真正全屏（覆盖整个显示器，包括任务栏）
2. 使用真实应用（浏览器、游戏）而非系统窗口
3. 检查日志中是否有 "PowerManager Enabled" 消息

### 问题 3：恢复后动画不动

**可能原因**：
1. 测试页面的动画实现不支持恢复
2. 浏览器限制（如 tab 不可见时）

**解决方法**：
1. 使用提供的测试页面（已正确实现暂停/恢复）
2. 确保测试页面使用 `requestAnimationFrame` 或 CSS 动画

## 性能影响

启用全屏应用检测后：

- **CPU 使用**: 检测线程每 2 秒轮询一次，影响极小（< 0.1%）
- **内存影响**: 约 1-2 MB（线程和状态管理）
- **省电效果**: 全屏时壁纸完全暂停渲染，可节省 50-90% CPU/GPU 使用

## 技术细节

### 检测机制

```cpp
bool PowerManager::IsFullscreenAppActive() {
  // 1. 获取前台窗口
  HWND foreground = GetForegroundWindow();
  
  // 2. 检查窗口是否覆盖整个显示器
  // 3. 排除桌面和系统窗口
  // 4. 记录应用名称和类名
  
  return is_fullscreen;
}
```

### 检测间隔

- 默认：每 2 秒检测一次
- 位置：`PowerManager::StartFullscreenDetection()`
- 可配置：修改 `std::this_thread::sleep_for(std::chrono::seconds(2))`

### 过滤的窗口类

以下窗口类会被忽略（不视为全屏应用）：
- `Progman` - 桌面程序管理器
- `WorkerW` - 桌面工作线程
- `Shell_TrayWnd` - 任务栏

## 相关文档

- [API 参考](DEVELOPER_API_REFERENCE.md)
- [技术笔记](TECHNICAL_NOTES.md)
- [Web 开发者指南](WEB_DEVELOPER_GUIDE_CN.md)
- [Visibility API](WEB_DEVELOPER_GUIDE_CN.md#visibility-api)

## 版本历史

- **v2.0.0**: 首次引入全屏应用暂停功能
- **v2.1.7**: 增强日志输出，记录全屏应用名称和类名

---

**最后更新**: 2025-11-14


