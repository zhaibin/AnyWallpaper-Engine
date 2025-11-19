# Explorer 重启修复文档

## 版本信息
- **版本**: v2.4.1
- **修复日期**: 2025-11-19
- **优先级**: 🔴 高（关键可靠性修复）

---

## 问题描述

### 现象
在 v2.4.0 及之前版本中，当 Windows Explorer 进程重启后，壁纸无法自动恢复显示。

### 复现步骤
1. 启动应用并初始化壁纸
2. 执行 `taskkill /F /IM explorer.exe`
3. 手动启动 `explorer.exe`
4. **结果**: 桌面图标恢复，但壁纸不显示

### 日志特征
```log
[AnyWP] [DesktopWallpaperHelper] SendMessageTimeout failed, error: 1400
[AnyWP] [DesktopWallpaperHelper] No SHELLDLL_DefView found, using first WorkerW as fallback: 5048272
[AnyWP] [DesktopWallpaperHelper] GetWallpaperParent called but WorkerW not valid
[AnyWP] [InitCoordinator] Initialization failed: Failed to find WorkerW window
```

---

## 根本原因分析

### 技术背景：Windows 桌面窗口层级
```
Desktop Hierarchy:
├─ Progman (Program Manager)
│  └─ SHELLDLL_DefView (Desktop Icons) ← 需要找到这个
│     └─ SysListView32
├─ WorkerW #1 (Icon Layer)
│  └─ SHELLDLL_DefView
└─ WorkerW #2 (Wallpaper Layer) ← 我们的目标
```

### 问题1：竞态条件
**代码路径**: `DesktopWallpaperHelper::TriggerWorkerWCreation()`

```cpp
// 问题代码（v2.4.0）
for (int i = 0; i < 3; i++) {
  SendMessageTimeoutW(info_.progman, ...);  // info_.progman 可能被 Reset() 清空
  sleep(150ms);
}
```

**触发条件**:
- `TriggerWorkerWCreation()` 正在发送消息（循环第2次）
- 显示配置变更事件触发 `HandleDisplayChange()`
- `HandleDisplayChange()` 调用 `Reset()` 清空 `info_.progman`
- 循环第3次向 `nullptr` 发送消息 → 错误 1400

**日志证据** (debug_run.log:1732-1747):
```
1731: Sent 0x052C to Progman successfully (attempt 1/3)
1732: [MonitorManager] Display configuration changed!    ← 并发事件
1736: [DesktopWallpaperHelper] Resetting cached WorkerW  ← Reset() 清空缓存
1747: SendMessageTimeout failed, error: 1400             ← 向无效句柄发送
```

### 问题2：Explorer 重启后桌面结构未就绪
**现象**: 发送 0x052C 消息成功，但 SHELLDLL_DefView 不存在

**分析**:
- Explorer 重启后，Progman 窗口立即创建
- 但 SHELLDLL_DefView（桌面图标容器）需要额外时间初始化
- 当前代码发现 SHELLDLL_DefView 缺失时，使用 Fallback 2 策略
- Fallback 2 选择第一个 WorkerW，但标记 `found_shelldll=false`
- `IsValid()` 因为 `found_shelldll=false` 返回 false
- 最终初始化失败

### 问题3：多次发送 0x052C 消息
**代码**: 发送 3 次消息（每次间隔150ms）

**Lively 验证**: 只发送 **1 次** 消息最可靠

**原因**:
- Progman 处理 0x052C 消息是**异步**的
- 多次发送可能混淆 Progman 状态机
- 导致 WorkerW 创建失败或结构异常

### 问题4：缺少延迟重试
**现状**: 首次失败后立即放弃

**Lively 策略**: 失败后延迟 500ms 重试 1 次

**原因**: Explorer 重启后桌面结构需要时间，延迟重试成功率更高

---

## 解决方案

### 优化1：修复竞态条件
**位置**: `desktop_wallpaper_helper.cpp:TriggerWorkerWCreation()`

```cpp
// v2.4.1+ 修复
HWND progman_handle = info_.progman;  // 使用局部变量快照

// 验证句柄有效性
if (!progman_handle || !IsWindow(progman_handle)) {
  Logger::Instance().Error("...", "Progman handle became invalid, re-finding...");
  if (!FindProgman()) return false;
  progman_handle = info_.progman;
}

// 发送消息
LRESULT ret = SendMessageTimeoutW(progman_handle, 0x052C, 0xD, 0x1, ...);

// 处理错误 1400
if (ret == 0 && GetLastError() == ERROR_INVALID_WINDOW_HANDLE) {
  Logger::Instance().Info("...", "Progman handle is stale (Explorer may have restarted)");
  FindProgman();  // 重新查找
  // 重试一次
  progman_handle = info_.progman;
  ret = SendMessageTimeoutW(progman_handle, 0x052C, 0xD, 0x1, ...);
}
```

**效果**:
- ✅ 避免向无效句柄发送消息
- ✅ 自动检测 Explorer 重启（错误 1400）
- ✅ 自动重新查找 Progman 并重试

### 优化2：只发送一次消息（Lively 风格）
**修改前**:
```cpp
for (int i = 0; i < 3; i++) {
  SendMessageTimeoutW(...);
  sleep(150ms);
}
sleep(500ms);  // 总计 500ms 等待
```

**修改后**:
```cpp
// 发送一次
SendMessageTimeoutW(...);

// 等待 Progman 处理（Lively 策略）
sleep(1000ms);
```

**优势**:
- ✅ 避免混淆 Progman
- ✅ 更长的等待时间让桌面结构完整初始化
- ✅ Lively Wallpaper 验证的最佳实践

### 优化3：延迟重试机制
**新增**: `FindWorkerWWithRetry()` 方法

```cpp
bool FindWorkerW(int timeout_ms) {
  return FindWorkerWWithRetry(timeout_ms, false);  // 首次尝试
}

bool FindWorkerWWithRetry(int timeout_ms, bool is_retry) {
  // ... 尝试创建 WorkerW ...
  
  // 检测 SHELLDLL_DefView 缺失
  if (info_.workerw_count > 0 && !info_.found_shelldll) {
    if (!is_retry && retry_count >= 5) {
      Logger::Instance().Info("...", 
        "SHELLDLL_DefView not found after multiple attempts, will retry after delay");
      break;  // 触发下面的延迟重试
    }
  }
  
  // v2.4.1+ 延迟重试（Lively 策略）
  if (!is_retry) {
    Logger::Instance().Info("...", 
      "First attempt failed, retrying WorkerW creation after 500ms delay...");
    sleep(500ms);
    return FindWorkerWWithRetry(timeout_ms, true);  // 递归重试
  }
  
  return false;
}
```

**重试条件**:
- SHELLDLL_DefView 不存在
- 已尝试多次（≥5次）
- 尚未重试过（`is_retry=false`）

**重试策略**:
- 延迟 500ms（Lively 验证的最佳间隔）
- 最多重试 1 次（避免无限等待）

### 优化4：优先使用 `found_shelldll` 判断
**修改前**:
```cpp
if (info_.wallpaper_layer && info_.wallpaper_layer != info_.progman) {
  return true;  // 优先判断 wallpaper_layer
}
```

**修改后**:
```cpp
if (info_.found_shelldll) {
  Logger::Instance().Info("...", 
    "WorkerW found successfully (SHELLDLL_DefView found)");
  return true;  // 最可靠的成功标志
}

// Fallback: 如果有 wallpaper_layer 也接受
if (info_.wallpaper_layer) {
  Logger::Instance().Warning("...", 
    "WorkerW found but SHELLDLL_DefView not found (fallback mode)");
  return true;
}
```

**优势**:
- ✅ `found_shelldll` 是最可靠的成功指标
- ✅ 避免使用不完整的桌面结构
- ✅ 触发 SHELLDLL_DefView 等待重试逻辑

---

## 参考文献

### Lively Wallpaper 源码
**文件**: `src\Lively\Lively\Core\WinDesktopCore.cs`

**SetupDesktopLayer() - 只发送一次消息**:
```csharp
// Line 155-161
NativeMethods.SendMessageTimeout(progman,
                       0x052C,
                       new IntPtr(0xD),
                       new IntPtr(0x1),
                       NativeMethods.SendMessageTimeoutFlags.SMTO_NORMAL,
                       1000,
                       out _);
```

**ResetWallpaperAsync() - 延迟重试**:
```csharp
// Line 566-573
SetupDesktopLayer();
if (workerW == IntPtr.Zero)
{
    Logger.Info("Retry creating WorkerW after delay..");
    await Task.Delay(500);  // 延迟 500ms
    SetupDesktopLayer();    // 重试
}
```

---

## 测试方法

### 自动化测试脚本
```bash
.\scripts\test_explorer_restart.bat
```

**测试步骤**:
1. 启动示例应用
2. 等待 10 秒初始化
3. Kill Explorer (`taskkill /F /IM explorer.exe`)
4. 重启 Explorer (`start explorer.exe`)
5. 监控 15 秒观察自动恢复

**预期结果**:
- ✅ 壁纸自动重新显示
- ✅ 日志包含 "Retrying WorkerW creation after 500ms delay"
- ✅ 无 "Failed to find WorkerW window" 错误

### 手动测试
```powershell
# 1. 启动应用
cd example\build\windows\x64\runner\Release
.\anywallpaper_engine_example.exe

# 2. 重启 Explorer
taskkill /F /IM explorer.exe
start explorer.exe

# 3. 观察壁纸是否自动恢复
```

### 验证日志关键字
```
✅ 成功标志:
[DesktopWallpaperHelper] Retrying WorkerW creation after delay
[DesktopWallpaperHelper] WorkerW found successfully (SHELLDLL_DefView found)
[InitCoordinator] Initialization successful

❌ 失败标志:
[DesktopWallpaperHelper] Failed to find WorkerW after all retry attempts
[InitCoordinator] Initialization failed: Failed to find WorkerW window
```

---

## 向后兼容性

### API 兼容性
- ✅ 无公共 API 变更
- ✅ 开发者无需修改代码
- ✅ 预编译包可直接替换

### 行为变更
- **初始化时间**: 增加约 500ms（延迟重试）
- **日志输出**: 新增重试相关日志
- **错误恢复**: 自动处理 Explorer 重启（之前失败）

### 配置变更
无需任何配置变更。

---

## 性能影响

### 正常场景（Explorer 未重启）
- **首次初始化**: ~1-2 秒（与之前相同）
- **消息发送**: 1 次（减少 2 次不必要的发送）
- **CPU 使用**: 略微降低（减少消息发送）

### Explorer 重启场景
- **首次尝试**: ~2-3 秒（失败）
- **延迟重试**: +500ms
- **重试成功**: ~2-3 秒
- **总计**: ~5-6 秒（之前直接失败）

### 结论
- ✅ 正常场景性能无影响
- ✅ 异常场景提升可靠性（从失败→成功）
- ✅ 轻微增加恢复时间是可接受的权衡

---

## 已知限制

### 1. 重试次数限制
- **限制**: 最多重试 1 次
- **原因**: 避免无限等待
- **影响**: 如果 Explorer 初始化非常慢（>3秒），仍可能失败

### 2. 不支持多次快速重启
- **场景**: 30 秒内多次重启 Explorer
- **原因**: Lively 也有此限制（防止频繁重试）
- **影响**: 会弹出错误提示，建议用户手动重建

### 3. 仅检测 WM_TASKBARCREATED 事件
- **依赖**: Windows 任务栏重建事件
- **限制**: 如果 Explorer 重启但任务栏未重建，不会触发恢复
- **概率**: 极低（正常 Explorer 重启都会触发）

---

## 后续优化方向

### 短期（可选）
1. **增加超时保护**: 设置 TaskbarCrashTimeOutDelay 配置项
2. **统计重试成功率**: 收集用户反馈和日志

### 长期（架构）
1. **Explorer PID 监控**: 主动检测进程变化（Lively 策略）
2. **状态机管理**: 引入初始化状态枚举
3. **EnumWindows 枚举**: 更可靠的 WorkerW 查找（Lively 使用）

---

## 相关文档
- [CHANGELOG_CN.md](../CHANGELOG_CN.md) - 详细更新日志
- [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) - 技术实现细节
- [AUTO_RECOVERY_GUIDE.md](AUTO_RECOVERY_GUIDE.md) - 自动恢复机制

---

**维护者**: AnyWP Engine Team  
**最后更新**: 2025-11-19

