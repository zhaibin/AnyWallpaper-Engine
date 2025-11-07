# 远程桌面支持测试指南

## 修复内容

修复了远程桌面切换后互动壁纸消失的问题。现在系统会自动检测并处理以下场景：

### 支持的会话切换事件

| 事件 | 触发场景 | 动作 | 日志标记 |
|------|---------|------|---------|
| **WTS_CONSOLE_CONNECT** | 从远程桌面回到本地控制台 | ✅ 恢复壁纸 | `CONSOLE CONNECTED` |
| **WTS_CONSOLE_DISCONNECT** | 从本地切换到远程桌面 | ⏸️ 暂停壁纸（省电） | `CONSOLE DISCONNECTED` |
| **WTS_REMOTE_CONNECT** | 远程桌面连接 | ⏸️ 暂停壁纸 | `REMOTE DESKTOP CONNECTED` |
| **WTS_REMOTE_DISCONNECT** | 远程桌面断开 | ✅ 恢复壁纸 | `REMOTE DESKTOP DISCONNECTED` |
| **WTS_SESSION_LOCK** | 系统锁屏 | ⏸️ 暂停壁纸 | `System LOCKED` |
| **WTS_SESSION_UNLOCK** | 解锁系统 | ✅ 恢复壁纸 | `System UNLOCKED` |

---

## 测试场景

### 场景 1：本地 → 远程桌面 → 本地

**步骤**：
1. 启动应用并激活互动壁纸
2. 打开"远程桌面连接"：`mstsc /v:localhost` 或 `mstsc /v:127.0.0.1`
3. 登录后观察（远程桌面内看不到桌面壁纸）
4. 断开远程桌面连接
5. ✅ **预期结果**：回到本地后，壁纸自动恢复

**日志验证**：
```
[AnyWP] [PowerSaving] CONSOLE DISCONNECTED (switched to remote desktop)
[AnyWP] [PowerSaving] ========== PAUSING WALLPAPER ==========
[AnyWP] [PowerSaving] Reason: Console disconnected

[AnyWP] [PowerSaving] CONSOLE CONNECTED (returned from remote desktop)
[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER ==========
[AnyWP] [PowerSaving] Reason: Console connected
```

---

### 场景 2：从另一台电脑远程桌面（更真实）

**准备工作**：
1. 在本机（电脑 A）启动应用并激活壁纸
2. 确保电脑 A 允许远程桌面连接：
   - `Win + R` → `SystemPropertiesRemote`
   - 勾选"允许远程连接到此计算机"
3. 记下电脑 A 的 IP 地址：`ipconfig`

**测试步骤**：
1. 从另一台电脑（电脑 B）远程桌面连接到电脑 A
2. 在远程桌面中工作一段时间
3. 断开远程桌面
4. 回到电脑 A 的物理控制台
5. ✅ **预期结果**：壁纸自动恢复

**日志验证**（在电脑 A 的日志文件中）：
```
[AnyWP] [PowerSaving] REMOTE DESKTOP CONNECTED
[AnyWP] [PowerSaving] ========== PAUSING WALLPAPER ==========

[AnyWP] [PowerSaving] REMOTE DESKTOP DISCONNECTED
[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER ==========
[AnyWP] [PowerSaving] Reason: Remote desktop disconnected
```

---

### 场景 3：锁屏后远程桌面（组合测试）

**步骤**：
1. 启动应用并激活壁纸
2. 锁定系统：`Win + L`
3. 从另一台电脑远程桌面连接
4. 在远程桌面中解锁
5. 工作一段时间
6. 断开远程桌面
7. 回到本地物理控制台
8. ✅ **预期结果**：壁纸自动恢复

---

## 自动恢复机制

当检测到会话切换事件时，`ResumeWallpaper()` 方法会自动执行以下检查：

### 1. 窗口有效性检查
```cpp
if (!IsWindow(webview_host_hwnd_) || !IsWindowVisible(webview_host_hwnd_))
```
- 检查窗口句柄是否仍然有效
- 检查窗口是否可见

### 2. 父窗口关系检查
```cpp
HWND parent = GetParent(webview_host_hwnd_);
if (parent != worker_w_hwnd_ || !IsWindow(worker_w_hwnd_))
```
- 验证壁纸窗口仍然是 WorkerW 的子窗口
- 检查 WorkerW 窗口本身是否有效

### 3. 自动重建机制
如果发现任何问题：
1. 保存当前 URL 和显示器配置
2. 清理无效窗口
3. 重新嵌入壁纸到桌面层级
4. 恢复之前的状态

**日志输出**：
```
[AnyWP] [PowerSaving] WARNING: Wallpaper window invalid or hidden!
[AnyWP] [PowerSaving] ========== RESTORING LOST WALLPAPER ==========
[AnyWP] [PowerSaving] Re-initializing wallpaper with URL: ...
[AnyWP] [PowerSaving] Wallpaper restoration complete
```

---

## 查看日志

### 实时日志监控
```bash
# PowerShell
Get-Content debug_run.log -Wait -Tail 20
```

### 关键日志过滤
```bash
# 查看所有会话切换事件
Select-String -Path debug_run.log -Pattern "CONSOLE|REMOTE|LOCK|UNLOCK"

# 查看恢复事件
Select-String -Path debug_run.log -Pattern "RESUMING|RESTORING"
```

---

## 预期日志示例

### ✅ 正常恢复（窗口仍然有效）
```
[AnyWP] [PowerSaving] CONSOLE CONNECTED (returned from remote desktop)
[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER ==========
[AnyWP] [PowerSaving] Reason: Console connected
[AnyWP] [PowerSaving] Window valid, resuming WebView rendering...
[AnyWP] [PowerSaving] Resume complete
```

### ✅ 自动重建（窗口丢失）
```
[AnyWP] [PowerSaving] CONSOLE CONNECTED (returned from remote desktop)
[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER ==========
[AnyWP] [PowerSaving] Reason: Console connected
[AnyWP] [PowerSaving] WARNING: Wallpaper window invalid or hidden!
[AnyWP] [PowerSaving] IsWindow: 0, IsVisible: 0
[AnyWP] [PowerSaving] ========== RESTORING LOST WALLPAPER ==========
[AnyWP] [PowerSaving] Stopping single-monitor wallpaper...
[AnyWP] [PowerSaving] Re-initializing wallpaper with URL: file:///...
[AnyWP] [PowerSaving] Wallpaper restoration complete
```

---

## 故障排除

### 问题：壁纸没有自动恢复

**检查步骤**：
1. 确认日志中有会话切换事件：
   ```
   Select-String -Path debug_run.log -Pattern "CONSOLE CONNECTED|REMOTE DISCONNECT"
   ```
   
2. 检查是否有错误信息：
   ```
   Select-String -Path debug_run.log -Pattern "ERROR|WARNING"
   ```

3. 查看窗口状态：
   ```
   [AnyWP] [PowerSaving] IsWindow: 0, IsVisible: 0
   ```
   - 如果都是 0，说明窗口已丢失，应该触发重建

### 问题：日志中没有会话切换事件

**可能原因**：
- 应用未正确注册会话通知
- 检查初始化日志：
  ```
  Select-String -Path debug_run.log -Pattern "Power listener window created"
  ```

**预期输出**：
```
[AnyWP] [PowerSaving] Power listener window created: 00000000011A0420
```

如果没有此日志，说明监听器未创建。

---

## 技术细节

### 代码位置
`windows/anywp_engine_plugin.cpp` → `PowerSavingWndProc()`

### 关键修改
```cpp
case WM_WTSSESSION_CHANGE:
  switch (wParam) {
    case WTS_CONSOLE_CONNECT:
      display_change_instance_->ResumeWallpaper("Console connected");
      break;
    case WTS_CONSOLE_DISCONNECT:
      display_change_instance_->PauseWallpaper("Console disconnected");
      break;
    case WTS_REMOTE_CONNECT:
      display_change_instance_->PauseWallpaper("Remote desktop connected");
      break;
    case WTS_REMOTE_DISCONNECT:
      display_change_instance_->ResumeWallpaper("Remote desktop disconnected");
      break;
  }
```

### Windows API
- `WTSRegisterSessionNotification()` - 注册会话通知
- `WM_WTSSESSION_CHANGE` - 会话状态改变消息
- `WTS_CONSOLE_CONNECT` / `WTS_CONSOLE_DISCONNECT` - 控制台切换
- `WTS_REMOTE_CONNECT` / `WTS_REMOTE_DISCONNECT` - 远程桌面连接/断开

---

## 测试清单

- [ ] **场景 1**：本地 → 远程桌面(localhost) → 本地 ✅
- [ ] **场景 2**：从另一台电脑远程桌面 ✅
- [ ] **场景 3**：锁屏 + 远程桌面组合 ✅
- [ ] **日志验证**：看到 CONSOLE CONNECTED 事件 ✅
- [ ] **日志验证**：看到 RESUMING WALLPAPER ✅
- [ ] **窗口验证**：壁纸正常显示在桌面图标后方 ✅
- [ ] **交互验证**：壁纸可以响应鼠标点击（如果启用交互）✅

---

## 优势

1. ✅ **零用户干预**：完全自动化，无需手动重启壁纸
2. ✅ **智能恢复**：自动检测窗口状态并重建
3. ✅ **省电优化**：远程桌面期间自动暂停（本地看不到）
4. ✅ **统一机制**：与锁屏恢复共享同一套逻辑
5. ✅ **详细日志**：便于诊断问题

---

## 已知限制

- 远程桌面**会话内部**看不到互动壁纸（正常行为，桌面在远程桌面中不可见）
- 仅在**回到本地物理控制台**后壁纸才会恢复
- 依赖 Windows Terminal Services 通知机制

---

**测试人员**：@Developer  
**测试日期**：2025-11-07  
**版本**：v1.2.2-dev  
**状态**：✅ 编译成功，等待远程桌面测试

