# WorkerW Recovery Implementation Notes

## 实现细节与注意事项（v2.3.1）

本文档说明 WorkerW 异常自动恢复机制的实现细节、最佳实践和注意事项。

---

## 🚨 安全性注意事项

### 1. **绝不销毁系统窗口**

**❌ 危险操作（永远不要这样做）**：
```cpp
// DANGEROUS: Do NOT do this!
HWND workerw = FindWindowW(L"WorkerW", nullptr);
DestroyWindow(workerw);  // ❌ Can crash Explorer!

HWND progman = FindWindowW(L"Progman", nullptr);
DestroyWindow(progman);  // ❌ Can crash Explorer!
```

**✅ 安全实现（我们的方式）**：
```cpp
// Safe: Only destroy our own temporary window
HWND tmp = CreateWindowExW(...);  // Create our window
if (tmp) {
    DestroyWindow(tmp);  // Safe: destroying our own window
}
```

**原理**：
- WorkerW、Progman、SHELLDLL_DefView 是系统窗口
- 销毁这些窗口可能导致 Explorer 崩溃或桌面无响应
- 我们只触发刷新和重建，不销毁系统窗口

### 2. **临时窗口刷新技巧**

我们创建并立即销毁一个临时窗口来触发桌面层级刷新（Lively Wallpaper 技术）：

```cpp
HWND tmp = CreateWindowExW(WS_EX_TOOLWINDOW, L"STATIC", L"TempWorkerWTrigger",
                            WS_POPUP | WS_VISIBLE, 0, 0, 1, 1,
                            nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
if (tmp) {
    ShowWindow(tmp, SW_HIDE);
    DestroyWindow(tmp);  // 安全：销毁我们自己创建的窗口
}
```

**这是安全的**，因为：
- 窗口是我们自己创建的
- 不是系统窗口
- 仅用于触发桌面刷新

---

## 🔐 权限相关

### Explorer 权限和策略

**问题**：
- 部分企业环境或安全策略可能阻止层级修改
- UAC 限制可能影响 `SetParent` 操作
- 某些防病毒软件可能拦截桌面修改

**测试建议**：
1. **普通用户权限测试**：
   ```bash
   # 以普通用户身份运行
   flutter run
   ```

2. **管理员权限测试**：
   ```bash
   # 以管理员身份运行（测试兼容性）
   flutter run --release
   ```

3. **检查日志**：
   ```
   [WorkerW Recovery] Access denied - may require administrator privileges
   ```

**错误码检测**：
```cpp
HWND old_parent = SetParent(webview_hwnd, new_workerw);
if (!old_parent && GetLastError() == ERROR_ACCESS_DENIED) {
    // 权限被拒绝，可能需要管理员权限或策略阻止
    Logger::Instance().Warning("Access denied - may require admin privileges");
}
```

---

## 🧵 WebView2 线程同步

### 父窗口变更后的 UI 刷新

**问题**：
- WebView2 可能在不同线程创建
- `SetParent` 后可能不会立即更新显示
- 需要强制刷新才能确保正确渲染

**✅ 正确实现**：
```cpp
// 1. 重新挂载到新 WorkerW
HWND old_parent = SetParent(webview_host_hwnd, new_workerw);

// 2. 修复 Z-order
SetWallpaperZOrder(webview_host_hwnd, new_workerw);

// 3. 强制 UI 刷新（v2.3.1+ 重要）
UpdateWindow(webview_host_hwnd);         // 立即重绘
InvalidateRect(webview_host_hwnd, nullptr, TRUE);  // 标记整个区域失效
```

**为什么需要这样做**：
- `UpdateWindow()` 强制立即处理 WM_PAINT 消息
- `InvalidateRect()` 标记区域需要重绘
- 确保 WebView2 内容正确显示在新的父窗口中

---

## 🖥️ 多显示器支持

### 当前实现

我们的实现**已完全支持多显示器**：

```cpp
// 单显示器模式
if (webview_host_hwnd_) {
    SetParent(webview_host_hwnd_, new_workerw);
    UpdateWindow(webview_host_hwnd_);
}

// 多显示器模式
for (auto& instance : wallpaper_instances_) {
    if (instance.webview_host_hwnd) {
        SetParent(instance.webview_host_hwnd, new_workerw);
        UpdateWindow(instance.webview_host_hwnd);
    }
}
```

### 多显示器注意事项

1. **WorkerW 是全局的**：
   - Windows 只有一个 WorkerW 壁纸层
   - 所有显示器共享同一个 WorkerW
   - 每个显示器的壁纸窗口都挂到同一个 WorkerW 下

2. **位置和大小**：
   - 每个显示器的壁纸窗口有自己的位置和大小
   - 通过 `MonitorInfo` 管理每个显示器的几何信息

3. **恢复流程**：
   - 找到新的 WorkerW（全局唯一）
   - 遍历所有显示器实例
   - 逐个重新挂载并刷新

---

## ⚙️ 可配置参数

### 强制刷新间隔

**v2.3.1+** 支持配置定期强制刷新的频率：

```cpp
// 默认：每 30 次检查（约 150 秒）
workerw_health_monitor_->SetForceRefreshInterval(30);

// 更频繁（每 20 次检查 = 约 100 秒）
workerw_health_monitor_->SetForceRefreshInterval(20);

// 更少频繁（每 60 次检查 = 约 300 秒）
workerw_health_monitor_->SetForceRefreshInterval(60);
```

**建议值**：
- **游戏场景**：20-30（更频繁刷新）
- **办公场景**：30-60（减少性能影响）
- **长期运行**：30（默认，平衡性能和稳定性）

### 检查间隔

```cpp
// 默认：每 5 秒检查一次
workerw_health_monitor_->StartMonitoring(workerw, 5000);

// 更频繁（每 3 秒）
workerw_health_monitor_->StartMonitoring(workerw, 3000);

// 更少频繁（每 10 秒）
workerw_health_monitor_->StartMonitoring(workerw, 10000);
```

---

## 📊 性能考虑

### CPU 开销

```
检查频率：每 5 秒
CPU 占用：< 0.1%（大部分时间睡眠）
内存开销：< 1MB（监控线程）
```

### 强制刷新开销

```
频率：每 150 秒（默认）
操作：查找 WorkerW + 验证结构
耗时：< 100ms
影响：几乎无感知
```

### 恢复开销

```
触发条件：WorkerW 失效或 Explorer 重启
操作：重新查找 + 重新挂载 + UI 刷新
耗时：2-5 秒
影响：短暂延迟，自动恢复
```

---

## 📝 日志记录

### 关键日志级别

```cpp
// Info - 正常操作
Logger::Instance().Info("WorkerW Recovery", "Recovery completed successfully");

// Warning - 可恢复问题
Logger::Instance().Warning("WorkerW Recovery", "Access denied - may require admin privileges");

// Error - 严重问题
Logger::Instance().Error("WorkerW Recovery", "Failed to find WorkerW after 3 attempts");
```

### 推荐日志查看

**初始化阶段**：
```
[DesktopWallpaperHelper] Triggering WorkerW creation (aggressive mode)
[DesktopWallpaperHelper] Found SHELLDLL_DefView via window enumeration
```

**健康监控**：
```
[WorkerWHealthMonitor] Monitor thread started
[WorkerWHealthMonitor] Explorer restart detected! PID changed: 12345 -> 67890
```

**恢复流程**：
```
[WorkerW Recovery] ========== Starting WorkerW Recovery ==========
[WorkerW Recovery] Re-finding WorkerW with aggressive strategy...
[WorkerW Recovery] Re-parented 2/2 wallpapers
[WorkerW Recovery] ========== Recovery Completed ==========
```

---

## 🐛 调试建议

### 1. 启用详细日志

```cpp
Logger::Instance().SetMinLevel(Logger::Level::DEBUG);
```

### 2. 检查窗口层级

使用 Spy++ 或类似工具查看窗口层级：
```
Desktop
  └─ Progman
      └─ SHELLDLL_DefView (图标层)
  └─ WorkerW (壁纸层) ← 我们的窗口应该在这里
      └─ Your WebView Window
```

### 3. 验证 WorkerW 结构

```cpp
HWND workerw = DesktopWallpaperHelper::Instance().GetWallpaperParent();
HWND parent = GetParent(webview_hwnd);

if (parent != workerw) {
    // 问题：父窗口不匹配
}
```

### 4. 测试 Explorer 重启

```cmd
# 手动重启 Explorer（测试恢复机制）
taskkill /f /im explorer.exe
explorer.exe
```

---

## ✅ 最佳实践总结

1. **✅ DO**：
   - 使用激进创建策略（多次消息 + 临时窗口）
   - 多重 fallback 查找策略
   - 监控 Explorer 进程 PID
   - 定期强制刷新
   - SetParent 后调用 UpdateWindow
   - 详细的日志记录
   - 错误码检查（ERROR_ACCESS_DENIED）

2. **❌ DON'T**：
   - 销毁系统窗口（WorkerW/Progman）
   - 过于频繁的强制刷新（< 10 秒）
   - 忽略线程同步问题
   - 假设所有环境都有相同权限

3. **🔍 MONITOR**：
   - Explorer 进程 PID 变化
   - SetParent 返回值和错误码
   - WorkerW 窗口有效性
   - 桌面结构完整性（SHELLDLL_DefView）

---

## 📚 参考资料

- **Lively Wallpaper**：开源桌面壁纸引擎，本实现基于其策略优化
- **Windows Desktop Architecture**：了解 Progman/WorkerW/SHELLDLL_DefView 结构
- **WebView2 Documentation**：理解 WebView2 线程模型

---

**最后更新**：2025-11-18  
**版本**：v2.3.1  
**状态**：已完成并测试

