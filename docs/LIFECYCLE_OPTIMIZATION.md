# 生命周期优化分析

## 当前生命周期问题

### 1. 初始化流程

**单监视器模式 (`InitializeWallpaper`)**:
```
1. 检查 is_initialized_ → 如果已初始化，调用 StopWallpaper()
2. 清理残留 iframe 数据
3. 调用 InitializeWallpaperCommon()
   └─ PeriodicCleanup()
   └─ InitializationCoordinator::Initialize()
   └─ SetupMouseHook() ⚠️ 每次都调用
4. ShowWindow / UpdateWindow
5. SetupWebView2WithManager()
   └─ 设置 is_initialized_ = true (仅 legacy 模式)
6. 启动 WorkerWHealthMonitor ⚠️ 每次都启动
```

**多监视器模式 (`InitializeWallpaperOnMonitor`)**:
```
1. 获取监视器列表
2. 查找目标监视器
3. 检查该监视器是否已有实例 → StopWallpaperOnMonitor()
4. 创建新实例
5. 调用 InitializeWallpaperCommon()
   └─ SetupMouseHook() ⚠️ 每次都调用
6. SetupWebView2WithManager()
   └─ ⚠️ is_initialized_ 不会被设置
7. ⚠️ WorkerWHealthMonitor 不会为多监视器实例启动
```

### 2. 停止流程

**StopWallpaper()**:
```
1. 停止 WorkerWHealthMonitor ✓
2. RemoveMouseHook() → MouseHookManager::Uninstall() ✓
3. 清理多监视器实例（遍历 wallpaper_instances_）
4. 清理单监视器实例（webview_controller_, webview_host_hwnd_）
5. 清理 iframe 数据
6. 设置 is_initialized_ = false
```

**StopWallpaperOnMonitor()**:
```
1. 委托给 InstanceManager::CleanupInstance()
2. 重建 EventDispatcher 缓存
3. ⚠️ 不检查是否是最后一个实例，不清理全局资源
```

### 3. 析构流程

**~AnyWPEnginePlugin()**:
```
1. 停止 WorkerWHealthMonitor ⚠️ 重复
2. 清理 MemoryOptimizer
3. 清理 PowerManager
4. 清理 MonitorManager
5. 清理 MouseHookManager → Uninstall() ⚠️ 重复
6. 清理其他模块
7. 调用 StopWallpaper() ⚠️ 会再次停止 WorkerW 和 MouseHook
8. 清理所有追踪资源
```

## 发现的问题

### 问题 1: 重复停止 WorkerWHealthMonitor
- **位置**: StopWallpaper() + 析构函数
- **影响**: 可能导致重复调用，虽然有防护，但不优雅
- **解决**: 在 StopWallpaper() 停止即可，析构函数只需清理

### 问题 2: 重复卸载 MouseHook
- **位置**: 
  - StopWallpaper() 调用 RemoveMouseHook()
  - 析构函数调用 mouse_hook_manager_->Uninstall()
  - InstanceManager 回调也可能调用 RemoveMouseHook()
- **影响**: 多次卸载同一个 Hook
- **解决**: 统一管理，添加状态检查

### 问题 3: is_initialized_ 状态不一致
- **问题**: 
  - 只在 legacy 模式下设置为 true
  - 多监视器模式下永远不会设置
  - InitializeWallpaper 开始时检查该标志
- **影响**: 多监视器模式下重复初始化检查失效
- **解决**: 使用实例数量判断，而不是单一标志

### 问题 4: MouseHook 重复安装
- **问题**: 每次 InitializeWallpaperCommon 都调用 SetupMouseHook()
- **影响**: 可能重复安装
- **解决**: 添加已安装检查

### 问题 5: WorkerW 监控仅单监视器
- **问题**: 
  - 只在 InitializeWallpaper (单监视器) 中启动
  - InitializeWallpaperOnMonitor (多监视器) 不启动
- **影响**: 多监视器模式下缺少健康监控
- **解决**: 统一管理，第一个实例启动，最后一个实例停止

### 问题 6: 多监视器清理不完整
- **问题**: StopWallpaperOnMonitor 不检查是否是最后一个实例
- **影响**: 最后一个监视器停止后，MouseHook 等全局资源未清理
- **解决**: 添加实例计数检查

## 优化方案

### 1. 统一初始化状态管理
```cpp
// 移除 is_initialized_ 标志
// 使用 GetActiveInstanceCount() 判断是否有活动实例
bool IsAnyWallpaperActive() const {
  return GetActiveInstanceCount() > 0;
}
```

### 2. 优化 MouseHook 生命周期
```cpp
void AnyWPEnginePlugin::SetupMouseHook() {
  if (mouse_hook_manager_ && !mouse_hook_manager_->IsInstalled()) {
    mouse_hook_manager_->Install();
  }
}

void AnyWPEnginePlugin::RemoveMouseHook() {
  if (mouse_hook_manager_ && mouse_hook_manager_->IsInstalled()) {
    mouse_hook_manager_->Uninstall();
  }
}
```

### 3. 优化 WorkerW 监控生命周期
```cpp
// 在第一个实例初始化时启动
void AnyWPEnginePlugin::StartWorkerWMonitoring(HWND worker_w) {
  if (workerw_health_monitor_ && !workerw_health_monitor_->IsMonitoring()) {
    workerw_health_monitor_->StartMonitoring(worker_w, 5000);
  }
}

// 在最后一个实例停止时停止
void AnyWPEnginePlugin::StopWorkerWMonitoring() {
  if (workerw_health_monitor_ && workerw_health_monitor_->IsMonitoring()) {
    if (GetActiveInstanceCount() == 0) {  // 只在没有活动实例时停止
      workerw_health_monitor_->StopMonitoring();
    }
  }
}
```

### 4. 简化析构函数
```cpp
~AnyWPEnginePlugin() {
  // 停止壁纸（会处理 WorkerW 和 MouseHook）
  StopWallpaper();
  
  // 清理各个模块（只需 reset，不需要重复停止）
  workerw_health_monitor_.reset();
  mouse_hook_manager_.reset();
  // ... 其他模块
  
  // 清理追踪资源
  ResourceTracker::Instance().CleanupAll();
}
```

### 5. 完善多监视器清理
```cpp
bool AnyWPEnginePlugin::StopWallpaperOnMonitor(int monitor_index) {
  instance_manager_->CleanupInstance(monitor_index);
  
  // 检查是否还有其他实例
  if (GetActiveInstanceCount() == 0) {
    // 最后一个实例，清理全局资源
    RemoveMouseHook();
    StopWorkerWMonitoring();
    default_wallpaper_url_.clear();
  }
  
  return true;
}
```

## 优化后的生命周期

### 初始化
```
1. 检查实例数量 → 如果目标位置已有实例，先停止
2. 清理残留数据
3. 初始化窗口和 WebView
4. 第一个实例时：
   - 安装 MouseHook（带状态检查）
   - 启动 WorkerWHealthMonitor（带状态检查）
5. 保存 URL
```

### 停止
```
1. 清理目标实例
2. 如果是最后一个实例：
   - 卸载 MouseHook（带状态检查）
   - 停止 WorkerWHealthMonitor（带状态检查）
   - 清理全局状态
```

### 析构
```
1. 调用 StopWallpaper()（处理所有清理）
2. 重置所有模块（不需要重复停止）
3. 清理追踪资源
```

## 实施步骤

1. ✅ 创建优化分支
2. ⬜ 添加实例计数辅助方法
3. ⬜ 为 MouseHook 添加状态检查
4. ⬜ 统一 WorkerW 监控管理
5. ⬜ 移除 is_initialized_ 标志
6. ⬜ 简化析构函数
7. ⬜ 完善多监视器清理
8. ⬜ 测试所有场景

