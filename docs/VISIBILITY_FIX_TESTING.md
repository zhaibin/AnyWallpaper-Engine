# 可见性失效问题修复测试指南

## 问题描述

在锁屏时，可见性测试页（`test_fullscreen_pause.html`）中的：
- 动画继续播放（应该暂停）
- 暂停/恢复计数器没有变化（应该在锁屏时 +1）

## 根本原因

`WallpaperLifecycleManager` 的暂停/恢复脚本中只调用了 `onPause()` 和 `onResume()` 回调，没有调用 `_notifyVisibilityChange()` 方法，导致测试页面的 `onVisibilityChange` 回调不被触发。

## 修复内容

### 1. `PauseWallpaper()` 修复

**文件**: `windows/modules/wallpaper_lifecycle_manager.cpp`

**修改前**:
```cpp
pause_script << L"(function() {"
             << L"  if (typeof window.AnyWP !== 'undefined' && window.AnyWP.onPause) {"
             << L"    window.AnyWP.onPause();"
             << L"  }"
             << L"  document.dispatchEvent(new CustomEvent('anywp:pause'));"
             << L"  if (typeof requestAnimationFrame === 'function') {"
             << L"    window.__anywp_cancelAllAnimations = true;"
             << L"  }"
             << L"})();";
```

**修改后**:
```cpp
pause_script << L"(function() {"
             << L"  if (typeof window.AnyWP !== 'undefined') {"
             << L"    if (window.AnyWP.onPause) {"
             << L"      window.AnyWP.onPause();"
             << L"    }"
             << L"    if (typeof window.AnyWP._notifyVisibilityChange === 'function') {"
             << L"      window.AnyWP._notifyVisibilityChange(false);"  // ✅ 新增
             << L"    }"
             << L"  }"
             << L"  document.dispatchEvent(new CustomEvent('anywp:pause'));"
             << L"  if (typeof requestAnimationFrame === 'function') {"
             << L"    window.__anywp_cancelAllAnimations = true;"
             << L"  }"
             << L"})();";
```

### 2. `ResumeWallpaper()` 修复

**修改前**:
```cpp
resume_script << L"(function() {"
              << L"  if (typeof window.AnyWP !== 'undefined' && window.AnyWP.onResume) {"
              << L"    window.AnyWP.onResume();"
              << L"  }"
              << L"  document.dispatchEvent(new CustomEvent('anywp:resume'));"
              << L"  if (typeof requestAnimationFrame === 'function') {"
              << L"    window.__anywp_cancelAllAnimations = false;"
              << L"  }"
              << L"})();";
```

**修改后**:
```cpp
resume_script << L"(function() {"
              << L"  if (typeof window.AnyWP !== 'undefined') {"
              << L"    if (window.AnyWP.onResume) {"
              << L"      window.AnyWP.onResume();"
              << L"    }"
              << L"    if (typeof window.AnyWP._notifyVisibilityChange === 'function') {"
              << L"      window.AnyWP._notifyVisibilityChange(true);"  // ✅ 新增
              << L"    }"
              << L"  }"
              << L"  document.dispatchEvent(new CustomEvent('anywp:resume'));"
              << L"  if (typeof requestAnimationFrame === 'function') {"
              << L"    window.__anywp_cancelAllAnimations = false;"
              << L"  }"
              << L"})();";
```

## 测试步骤

### 1. 构建应用
```bash
cd example
flutter build windows --debug
```

### 2. 运行应用
```bash
.\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

### 3. 加载测试页面
点击 **"🎯 Fullscreen Pause"** 按钮加载测试页面

### 4. 验证初始状态
- ✅ 看到动画圆圈在移动
- ✅ 暂停次数: 0
- ✅ 恢复次数: 0
- ✅ 状态: "壁纸可见（正常运行）"

### 5. 测试锁屏暂停
1. 按 **Win+L** 锁屏
2. 等待 2-3 秒
3. 解锁返回桌面
4. 观察计数器变化

**预期结果**:
- ⏸️ 暂停次数: 1（锁屏时 +1）
- ▶️ 恢复次数: 1（解锁时 +1）
- ✅ 状态显示正确切换

### 6. 测试全屏应用暂停
1. 打开浏览器按 **F11** 进入全屏
2. 观察计数器
3. 按 **F11** 退出全屏
4. 观察计数器

**预期结果**:
- ⏸️ 暂停次数: 2（进入全屏 +1）
- ▶️ 恢复次数: 2（退出全屏 +1）

### 7. 重复测试
多次锁屏/解锁和全屏/退出全屏，确保计数器稳定增加

## 日志验证

### 锁屏时的日志
```
[PowerSaving] ========== SESSION CHANGE EVENT ==========
[PowerSaving] Event code: 7
[PowerSaving] Event: System LOCKED
[PowerManager] Session lock state changed: 1
[PowerManager] State changed: 0 -> 3  // ACTIVE -> LOCKED
[PowerManager] Callback: Pause requested (PowerManager: screen_locked)
[WallpaperLifecycleManager] Pausing wallpaper (reason: PowerManager: screen_locked)
[WallpaperLifecycleManager] Wallpaper paused successfully - last frame frozen
```

### 解锁时的日志
```
[PowerSaving] Event code: 8
[PowerSaving] Event: System UNLOCKED
[PowerManager] Session lock state changed: 0
[PowerManager] State changed: 3 -> 0  // LOCKED -> ACTIVE
[PowerManager] Callback: Resume requested (PowerManager: screen_unlocked)
[WallpaperLifecycleManager] Resuming wallpaper (reason: PowerManager: screen_unlocked, force_reinit: false)
[WallpaperLifecycleManager] Wallpaper resumed successfully - animations restarted
```

## 测试页面日志

测试页面的浏览器控制台应该显示：

### 锁屏时
```javascript
[SDK] AnyWP detected, version: 2.1.10
[SDK] _notifyVisibilityChange method exists
[SDK] Visibility callback registered
========== VISIBILITY CALLBACK TRIGGERED ==========
[Callback] Visible: false
⏸️ 暂停 +1 (当前: 1)
[Animation] Stopping...
```

### 解锁时
```javascript
========== VISIBILITY CALLBACK TRIGGERED ==========
[Callback] Visible: true
✅ 恢复 +1 (当前: 1)
[Animation] Restarting...
```

## 常见问题

### 问题 1: 计数器没有变化
**原因**: SDK 未正确加载或回调未注册

**解决**:
1. 按 F12 打开浏览器控制台
2. 检查是否有 SDK 初始化日志
3. 手动调用测试: `window.AnyWP._notifyVisibilityChange(false)`

### 问题 2: 动画没有停止
**原因**: 测试页面使用了不支持暂停的动画类型

**解决**:
使用提供的测试页面 `test_fullscreen_pause.html`，它使用 `requestAnimationFrame` 实现动画

### 问题 3: 锁屏后解锁计数器不变
**原因**: PowerManager 没有接收到解锁事件

**解决**:
1. 检查日志中是否有 "System UNLOCKED" 消息
2. 确保 `WTSRegisterSessionNotification` 调用成功
3. 重启应用程序

## 技术细节

### 工作流程
1. **锁屏触发**: `WM_WTSSESSION_CHANGE` → `WTS_SESSION_LOCK`
2. **状态更新**: `PowerManager::SetSessionLocked(true)`
3. **状态检测**: `PowerManager::UpdatePowerState()` → 检测到 `LOCKED` 状态
4. **触发暂停**: `PowerManager::Pause("PowerManager: screen_locked")`
5. **执行脚本**: `WallpaperLifecycleManager::PauseWallpaper()`
   - 调用 `_notifyVisibilityChange(false)` ✅
   - 触发 `onVisibilityChange` 回调 ✅
   - 更新测试页面计数器 ✅

### 解锁流程
1. **解锁触发**: `WTS_SESSION_UNLOCK`
2. **状态更新**: `PowerManager::SetSessionLocked(false)`
3. **触发恢复**: `PowerManager::Resume("PowerManager: screen_unlocked")`
4. **执行脚本**: `WallpaperLifecycleManager::ResumeWallpaper()`
   - 调用 `_notifyVisibilityChange(true)` ✅
   - 触发 `onVisibilityChange` 回调 ✅
   - 更新测试页面计数器 ✅

## 相关文件

- `windows/modules/wallpaper_lifecycle_manager.cpp` - 修复的核心文件
- `windows/modules/power_manager.cpp` - 状态检测和触发
- `windows/anywp_engine_plugin.cpp` - WTS 事件处理
- `windows/anywp_sdk.js` - SDK 实现
- `examples/test_fullscreen_pause.html` - 测试页面

---

**修复版本**: v2.4.1+  
**修复日期**: 2025-11-19  
**修复分支**: `refactor/modularization-improvement`

