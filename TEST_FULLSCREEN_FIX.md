# 全屏暂停功能测试指南

## 🎯 测试目标

验证全屏应用检测与暂停/恢复功能是否正常工作。

## 📋 测试步骤

### 1. 启动应用并加载测试页面

```bash
cd example
flutter run -d windows
```

或直接运行：
```bash
.\example\build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

在应用界面中：
1. 点击 **"🎯 Fullscreen Pause"** 按钮
2. 或点击 **"👁️ Visibility"** 按钮
3. 等待页面加载完成（看到动画圆圈）

### 2. 测试锁屏场景（已验证 ✅）

**操作**：
1. 按 **Win + L** 锁屏
2. 解锁回到桌面

**预期结果**：
- ✅ 锁屏时：暂停计数器 **+1**
- ✅ 解锁时：恢复计数器 **+1**
- ✅ 日志显示：`[ScriptExecution] Result: "PAUSED"` 和 `"RESUMED"`

**原理**：锁屏时 WebView 保持活跃，脚本正常执行

---

### 3. 测试全屏场景（需验证 ⚠️）

**操作**：
1. 打开 Edge 浏览器（或 Chrome、Firefox）
2. 按 **F11** 进入全屏
3. 等待 2-3 秒
4. 按 **F11** 或 **ESC** 退出全屏

**预期结果**（v2.1.7+ 修复后）：
- ✅ 进入全屏：暂停计数器 **+1**（延迟通知）
- ✅ 退出全屏：恢复计数器 **+1**（延迟通知）
- ⚡ 日志显示：
  ```
  [PowerSaving] Fullscreen detected - WebView auto-suspended by Windows
  [PowerSaving] Skipping pause scripts (already power-saving)
  [PowerSaving] Detected fullscreen exit - will delay script execution
  [PowerSaving] Scheduling delayed resume (500ms)...
  [ScriptExecution] Result: "RESUMED"
  ```

**原理**：
- 全屏时 Windows 自动挂起 WebView（已省电），跳过暂停脚本
- 退出全屏后延迟 500ms 执行恢复脚本（确保 WebView 已唤醒）

---

## 🔍 查看日志

日志文件位置：`test_logs/debug_run.log`

**关键日志标识**：

### 锁屏场景
```log
[PowerManager] State changed: 0 -> 3  // ACTIVE -> LOCKED
[PowerSaving] ========== PAUSING WALLPAPER ==========
[PowerManager] Executing pause scripts...
[ScriptExecution] Result: "PAUSED"
[ScriptExecution] Result: "NOTIFY_PAUSE_OK"

[PowerManager] State changed: 3 -> 0  // LOCKED -> ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========
[PowerManager] Executing resume scripts...
[ScriptExecution] Result: "RESUMED"
[ScriptExecution] Result: "NOTIFY_RESUME_OK"
```

### 全屏场景（v2.1.7+）
```log
[PowerManager] Fullscreen app detected: "Microsoft Edge" (Class: Chrome_WidgetWin_1)
[PowerManager] State changed: 0 -> 4  // ACTIVE -> FULLSCREEN_APP
[PowerSaving] ========== PAUSING WALLPAPER ==========
[PowerSaving] Fullscreen detected - WebView auto-suspended by Windows
[PowerSaving] Skipping pause scripts (already power-saving)

[PowerManager] State changed: 4 -> 0  // FULLSCREEN_APP -> ACTIVE
[PowerSaving] ========== RESUMING WALLPAPER ==========
[PowerSaving] Detected fullscreen exit - will delay script execution
[PowerSaving] Scheduling delayed resume (500ms)...
[PowerSaving] Executing delayed resume scripts...
[ScriptExecution] Result: "RESUMED"
[ScriptExecution] Result: "NOTIFY_RESUME_OK"
```

---

## ✅ 验证清单

- [x] **锁屏测试**：暂停 +1，恢复 +1
- [ ] **全屏测试**：暂停 +1（延迟通知），恢复 +1（延迟通知）
- [ ] **日志验证**：全屏时跳过暂停脚本，恢复时延迟执行

---

## 📊 性能对比

| 场景 | 旧版本 (v2.1.6) | 新版本 (v2.1.7) |
|------|----------------|----------------|
| **锁屏暂停** | 执行脚本 ✅ | 执行脚本 ✅ |
| **锁屏恢复** | 执行脚本 ✅ | 执行脚本 ✅ |
| **全屏暂停** | 执行脚本 ❌ (被阻塞) | 跳过脚本 ⚡ (优化) |
| **全屏恢复** | 执行脚本 ❌ (被阻塞) | 延迟执行 ✅ (修复) |
| **计数器** | 锁屏正常，全屏不变 | 全部正常 |

---

## 🐛 已知问题

### 问题：全屏时计数器不变（v2.1.6 及之前）

**原因**：
1. Windows 挂起后台 WebView 的 JS 引擎（省电）
2. 暂停脚本被阻塞，无法执行
3. 恢复脚本也被阻塞，回调永远不触发

**解决**：
1. 全屏暂停：跳过脚本（系统已自动省电）
2. 全屏恢复：延迟 500ms 执行（等待 WebView 唤醒）

---

## 💡 技术细节

### 为什么全屏时不需要暂停脚本？

当全屏应用在前台时，Windows 会自动挂起后台 WebView：

✅ **CPU**: JavaScript 引擎停止运行，CPU 使用率 → 0%  
✅ **GPU**: WebView 渲染管线暂停，GPU 闲置  
✅ **动画**: `requestAnimationFrame`、CSS 动画、视频播放自动停止  
✅ **内存**: 内存冻结，不再分配

**结论**：系统自动挂起已达到省电目的，执行暂停脚本是多余的（而且也执行不了）

### 为什么恢复时需要延迟？

WebView 从挂起状态恢复需要时间：
- JS 引擎重新启动
- 渲染管线重新激活
- 事件循环恢复运行

如果立即执行脚本，可能被再次阻塞。延迟 500ms 确保 WebView 完全唤醒。

---

**最后更新**: 2025-11-14  
**相关文档**: [FULLSCREEN_PAUSE_TEST_GUIDE.md](docs/FULLSCREEN_PAUSE_TEST_GUIDE.md)

