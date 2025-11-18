# 生命周期优化 - v2.3.2

## 概述

本次优化解决了壁纸引擎在启动、运行和退出整个生命周期中的重复逻辑和资源管理问题。

## 修改文件

### 1. 新增文件
- `windows/anywp_engine_plugin_lifecycle.cpp` - 生命周期辅助方法实现
- `docs/LIFECYCLE_OPTIMIZATION.md` - 详细的生命周期分析文档
- `LIFECYCLE_CHANGES.md` - 本文档

### 2. 修改文件
- `windows/anywp_engine_plugin.h` - 添加 `GetActiveInstanceCount()` 方法声明，标记 `is_initialized_` 为 deprecated
- `windows/anywp_engine_plugin.cpp` - 优化生命周期管理逻辑
- `windows/CMakeLists.txt` - 添加 lifecycle 实现文件到编译列表

## 核心优化

### 1. MouseHook 管理 (✅ 完成)

**问题**: MouseHook 在多个地方被安装和卸载，可能导致重复操作。

**修改**:
- `SetupMouseHook()` - 添加 `IsInstalled()` 检查，如果已安装则跳过
- `RemoveMouseHook()` - 添加 `IsInstalled()` 检查，如果未安装则跳过

**文件**: `windows/anywp_engine_plugin.cpp` (Line 1538-1608)

```cpp
// v2.3.2+: Added IsInstalled() check to prevent duplicate installation
void AnyWPEnginePlugin::SetupMouseHook() {
  if (!mouse_hook_manager_) return;
  
  // Skip if already installed
  if (mouse_hook_manager_->IsInstalled()) {
    std::cout << "[AnyWP] [Lifecycle] MouseHook already installed, skipping" << std::endl;
    return;
  }
  
  // Install hook...
}
```

### 2. WorkerW 健康监控管理 (✅ 完成)

**问题**: 
- 单监视器模式启动监控，多监视器模式不启动
- 停止时可能重复停止（StopWallpaper + 析构函数）

**修改**:
- `InitializeWallpaper()` - 添加 `IsMonitoring()` 检查，只在未监控时启动
- `InitializeWallpaperOnMonitor()` - 添加相同的监控启动逻辑
- `StopWallpaperOnMonitor()` - 只在最后一个实例停止时停止监控
- 析构函数 - 不再手动停止监控（由 StopWallpaper 处理）

**文件**: `windows/anywp_engine_plugin.cpp`

**单监视器** (Line 1767-1781):
```cpp
// v2.3.2+: Only start if not already monitoring
if (workerw_health_monitor_ && worker_w_hwnd_) {
  if (!workerw_health_monitor_->IsMonitoring()) {
    std::cout << "[AnyWP] [Lifecycle] Starting WorkerW health monitoring (first instance)..." << std::endl;
    workerw_health_monitor_->StartMonitoring(worker_w_hwnd_, 5000);
  }
}
```

**多监视器** (Line 2217-2230):
```cpp
// v2.3.2+: Start WorkerW health monitoring if this is the first instance
if (workerw_health_monitor_ && new_instance.worker_w_hwnd) {
  if (!workerw_health_monitor_->IsMonitoring()) {
    std::cout << "[AnyWP] [Lifecycle] Starting WorkerW health monitoring (first multi-monitor instance)..." << std::endl;
    workerw_health_monitor_->StartMonitoring(new_instance.worker_w_hwnd, 5000);
  }
}
```

### 3. 多监视器实例管理 (✅ 完成)

**问题**: `StopWallpaperOnMonitor()` 停止单个监视器时不检查是否为最后一个实例，导致全局资源未清理。

**新增方法**: `GetActiveInstanceCount()` - 返回当前活动的壁纸实例数量（单监视器 + 多监视器）

**文件**: `windows/anywp_engine_plugin_lifecycle.cpp`

```cpp
size_t AnyWPEnginePlugin::GetActiveInstanceCount() const {
  size_t count = 0;
  
  // Count multi-monitor instances
  {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(instances_mutex_));
    count += wallpaper_instances_.size();
  }
  
  // Count legacy single-monitor instance
  if (webview_host_hwnd_ != nullptr && is_initialized_) {
    count++;
  }
  
  return count;
}
```

**修改**: `StopWallpaperOnMonitor()` - 添加实例计数检查

**文件**: `windows/anywp_engine_plugin.cpp` (Line 2236-2260)

```cpp
// v2.3.2+: Check if this is the last active instance
size_t active_count = GetActiveInstanceCount();

if (active_count == 0) {
  std::cout << "[AnyWP] [Lifecycle] Last instance stopped, cleaning up global resources..." << std::endl;
  
  // Stop WorkerW health monitoring
  if (workerw_health_monitor_ && workerw_health_monitor_->IsMonitoring()) {
    workerw_health_monitor_->StopMonitoring();
  }
  
  // Remove MouseHook
  RemoveMouseHook();
  
  // Clear default URL
  default_wallpaper_url_.clear();
}
```

### 4. 析构函数优化 (✅ 完成)

**问题**: 析构函数在多个地方重复清理资源：
- 手动停止 WorkerWHealthMonitor (Line 446)
- 手动卸载 MouseHook (Line 526)
- 调用 StopWallpaper() (Line 603) - 会再次执行上述操作

**优化方案**:
1. 先调用 `StopWallpaper()`（一次性处理所有清理）
2. 然后只 reset 模块指针（不再重复调用停止方法）

**文件**: `windows/anywp_engine_plugin.cpp` (Line 439-612)

```cpp
AnyWPEnginePlugin::~AnyWPEnginePlugin() {
  Logger::Instance().Info("Plugin", "Destructor - starting cleanup");
  
  // v2.3.2+: Stop all wallpaper instances first (will stop WorkerW monitoring and remove MouseHook)
  std::cout << "[AnyWP] [Lifecycle] Destructor: Stopping wallpaper instances..." << std::endl;
  StopWallpaper();
  
  // v2.3.2+: Cleanup modules (no need to call Stop/Uninstall, already done)
  if (workerw_health_monitor_) {
    workerw_health_monitor_.reset();  // Just reset, don't call StopMonitoring()
  }
  
  // ... other modules ...
  
  if (mouse_hook_manager_) {
    mouse_hook_manager_.reset();  // Just reset, don't call Uninstall()
  }
  
  // ResourceTracker cleanup
  ResourceTracker::Instance().CleanupAll();
}
```

## 生命周期流程图

### 优化前

```
初始化:
  InitializeWallpaper()
    ├─ StopWallpaper() (如果已初始化)
    ├─ InitializeWallpaperCommon()
    │  └─ SetupMouseHook() ⚠️ 每次都调用
    └─ StartMonitoring() ⚠️ 每次都调用

停止:
  StopWallpaper()
    ├─ StopMonitoring() ✓
    └─ RemoveMouseHook() ✓

析构:
  ~AnyWPEnginePlugin()
    ├─ StopMonitoring() ⚠️ 重复
    ├─ Uninstall() ⚠️ 重复
    └─ StopWallpaper() ⚠️ 又一次停止和移除
```

### 优化后

```
初始化:
  InitializeWallpaper() / InitializeWallpaperOnMonitor()
    ├─ Check existing instance → Stop if needed
    ├─ InitializeWallpaperCommon()
    │  └─ SetupMouseHook()
    │     └─ if (!IsInstalled()) → Install ✓
    └─ if (!IsMonitoring()) → StartMonitoring() ✓

停止单个监视器:
  StopWallpaperOnMonitor(index)
    ├─ CleanupInstance(index)
    └─ if (GetActiveInstanceCount() == 0):
       ├─ StopMonitoring() ✓
       ├─ RemoveMouseHook() ✓
       └─ Clear global state

停止所有:
  StopWallpaper()
    ├─ if (IsMonitoring()) → StopMonitoring() ✓
    ├─ if (IsInstalled()) → RemoveMouseHook() ✓
    └─ Cleanup all instances

析构:
  ~AnyWPEnginePlugin()
    ├─ StopWallpaper() (一次性清理)
    └─ Reset all modules (无重复操作) ✓
```

## 测试结果

### 编译测试

✅ **编译成功**: `flutter build windows --debug`

```
Building Windows application...                                    43.7s
√ Built build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

### 功能测试

需要测试的场景：

1. **单监视器模式**
   - [  ] 初始化 → 验证 MouseHook 和 WorkerW 监控启动
   - [  ] 重复初始化 → 验证不会重复安装
   - [  ] 停止 → 验证资源正确清理
   
2. **多监视器模式**
   - [  ] 初始化第一个监视器 → 验证全局资源启动
   - [  ] 初始化第二个监视器 → 验证不会重复启动
   - [  ] 停止第一个监视器 → 验证全局资源保持
   - [  ] 停止最后一个监视器 → 验证全局资源清理

3. **生命周期完整性**
   - [  ] 程序正常退出 → 验证无重复清理警告
   - [  ] 程序异常退出 → 验证析构函数正常运行

## 优化效果

### 性能提升
- 减少重复的系统调用（Hook 安装/卸载、线程启动/停止）
- 避免不必要的状态检查和锁竞争
- 减少日志输出噪音

### 代码质量
- 生命周期管理更加清晰
- 状态管理更加一致
- 减少潜在的资源泄漏风险

### 可维护性
- 单一职责原则：每个方法只负责一件事
- 清晰的调用关系：避免交叉依赖
- 完善的状态检查：防止重复操作

## 向后兼容性

- ✅ 保留 `is_initialized_` 标志（标记为 deprecated，但仍然工作）
- ✅ 所有公共 API 签名保持不变
- ✅ 行为语义保持一致（只是内部实现优化）

## 未来改进

1. 完全移除 `is_initialized_` 标志，统一使用 `GetActiveInstanceCount()`
2. 考虑使用状态机模式管理插件生命周期
3. 添加更多的生命周期事件回调（onBeforeInit, onAfterStop 等）

## 变更日志

**版本**: v2.3.2  
**分支**: optimize-lifecycle  
**日期**: 2025-11-18

### Added
- 新增 `GetActiveInstanceCount()` 方法 - 统一的实例计数
- 新增 `windows/anywp_engine_plugin_lifecycle.cpp` 文件

### Changed
- 优化 `SetupMouseHook()` - 添加重复安装检查
- 优化 `RemoveMouseHook()` - 添加重复卸载检查
- 优化 `InitializeWallpaper()` - 添加 WorkerW 监控状态检查
- 优化 `InitializeWallpaperOnMonitor()` - 添加 WorkerW 监控支持
- 优化 `StopWallpaperOnMonitor()` - 添加最后实例检查
- 优化 `~AnyWPEnginePlugin()` - 移除重复清理逻辑

### Deprecated
- `is_initialized_` 标志 - 建议使用 `GetActiveInstanceCount()` 替代

### Fixed
- 修复 MouseHook 可能被重复安装的问题
- 修复 WorkerW 监控可能被重复启动/停止的问题
- 修复多监视器模式下全局资源未正确清理的问题
- 修复析构函数中的重复清理操作

