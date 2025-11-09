# C++ 稳定重构方案（Stable Refactoring Plan）

## 现状分析

### 主文件状况
- **文件**: `windows/anywp_engine_plugin.cpp`
- **行数**: **4176 行**（过大，需要重构）
- **问题**: 所有功能耦合在一起，难以维护和测试

### 已有模块（部分实现，未集成）

#### 1. `PowerManager` (windows/modules/power_manager.h/cpp)
**实现状态**: ✅ 已实现 (~290 行)
**集成状态**: ❌ **未集成到主文件**

**已实现功能**:
- 全屏应用检测
- 用户会话锁定检测
- 电源状态管理
- 状态变更回调

**主文件中的重复代码**:
- `SetupPowerSavingMonitoring()` (~55 行)
- `CleanupPowerSavingMonitoring()` (~20 行)
- `UpdatePowerState()` (~42 行)
- `IsFullscreenAppActive()` (~46 行)
- `StartFullscreenDetection()` (~42 行)
- `StopFullscreenDetection()` (~12 行)
- `PauseWallpaper()` (~94 行)
- `ResumeWallpaper()` (~252 行)
- 相关成员变量和枚举（~50 行）

**总计可减少**: ~613 行

#### 2. `MonitorManager` (windows/modules/monitor_manager.h/cpp)
**实现状态**: ✅ 已实现 (~179 行)
**集成状态**: ❌ **未集成到主文件**

**已实现功能**:
- 显示器枚举
- 热插拔监听
- 显示器信息缓存
- 变更通知

**主文件中的重复代码**:
- `GetMonitors()` (~21 行)
- `MonitorEnumProc()` (~39 行)
- `SetupDisplayChangeListener()` (~44 行)
- `CleanupDisplayChangeListener()` (~8 行)
- `HandleDisplayChange()` (~50 行)
- `NotifyMonitorChange()` (~35 行)
- `SafeNotifyMonitorChange()` (~17 行)
- `HandleMonitorCountChange()` (~89 行)
- `UpdateWallpaperSizes()` (~68 行)

**总计可减少**: ~371 行

#### 3. `MouseHookManager` (windows/modules/mouse_hook_manager.h/cpp)
**实现状态**: ✅ 已实现 (~214 行)
**集成状态**: ❌ **未集成到主文件**

**已实现功能**:
- 鼠标钩子安装/卸载
- 点击事件回调
- iframe 检测回调
- 拖拽取消

**主文件中的重复代码**:
- `SetupMouseHook()` (~20 行)
- `RemoveMouseHook()` (~8 行)
- `LowLevelMouseProc()` (~200+ 行，复杂逻辑)
- `CancelActiveDrag()` (~20 行)
- 相关成员变量（~10 行）

**总计可减少**: ~258 行

#### 4. `SDKBridge` (windows/modules/sdk_bridge.h/cpp)
**实现状态**: ⚠️ **仅有头文件**
**集成状态**: ❌ 未实现

**需要实现的功能**:
- SDK 脚本加载
- SDK 注入
- 消息桥接
- Web 消息处理

#### 5. `IframeDetector` (windows/modules/iframe_detector.h/cpp)
**实现状态**: ⚠️ **仅有头文件**
**集成状态**: ❌ 未实现

**需要实现的功能**:
- iframe 数据解析
- iframe 边界检测
- 点击坐标映射

---

## 稳定重构策略

### 原则
1. **渐进式**: 一次只重构一个模块
2. **小步快跑**: 每个步骤后立即编译测试
3. **向后兼容**: 保留所有公共 API
4. **功能等价**: 重构前后行为完全一致
5. **无破坏性**: 出现问题立即回滚

### 步骤详解

---

## Step 1: 完善并集成 PowerManager（优先级最高）

### 1.1 完善 PowerManager
- ✅ 已有的功能不需要修改
- ➕ 添加 `Pause()` 和 `Resume()` 方法
- ➕ 添加 WebView 实例管理接口

### 1.2 在主文件中集成
```cpp
// anywp_engine_plugin.h
#include "modules/power_manager.h"

class AnyWPEnginePlugin {
private:
  std::unique_ptr<PowerManager> power_manager_;
  
  // 删除这些成员变量（移到 PowerManager）:
  // PowerState power_state_;
  // PowerState last_power_state_;
  // bool auto_power_saving_enabled_;
  // HWND power_listener_hwnd_;
  // 等等...
};
```

### 1.3 替换主文件中的调用
```cpp
// 旧代码
SetupPowerSavingMonitoring();
UpdatePowerState();

// 新代码
power_manager_ = std::make_unique<PowerManager>();
power_manager_->Enable(true);
power_manager_->SetStateChangeCallback([this](auto old_state, auto new_state) {
  // 处理状态变更
});
```

### 1.4 删除主文件中的重复代码
- 删除 `SetupPowerSavingMonitoring()`
- 删除 `CleanupPowerSavingMonitoring()`
- 删除 `UpdatePowerState()`
- 删除 `IsFullscreenAppActive()`
- 删除 `StartFullscreenDetection()`
- 删除 `StopFullscreenDetection()`
- 删除 `PauseWallpaper()`（移到 PowerManager）
- 删除 `ResumeWallpaper()`（移到 PowerManager）

**预期减少**: ~613 行

---

## Step 2: 完善并集成 MonitorManager

### 2.1 完善 MonitorManager
- ✅ 已有的功能基本完整
- ➕ 添加更多回调支持

### 2.2 在主文件中集成
```cpp
// anywp_engine_plugin.h
#include "modules/monitor_manager.h"

class AnyWPEnginePlugin {
private:
  std::unique_ptr<MonitorManager> monitor_manager_;
  
  // 删除这些成员变量:
  // std::vector<MonitorInfo> monitors_;
  // HWND display_listener_hwnd_;
};
```

### 2.3 替换主文件中的调用
```cpp
// 旧代码
monitors_ = GetMonitors();
SetupDisplayChangeListener();

// 新代码
monitor_manager_ = std::make_unique<MonitorManager>();
monitor_manager_->StartMonitoring();
monitor_manager_->SetChangeCallback([this](const auto& monitors) {
  HandleMonitorCountChange(monitors);
});
```

### 2.4 删除主文件中的重复代码
- 删除 `GetMonitors()`
- 删除 `MonitorEnumProc()`
- 删除 `SetupDisplayChangeListener()`
- 删除 `CleanupDisplayChangeListener()`
- 删除 `HandleDisplayChange()`
- 删除 `NotifyMonitorChange()`
- 删除 `SafeNotifyMonitorChange()`

**预期减少**: ~371 行

---

## Step 3: 完善并集成 MouseHookManager

### 3.1 完善 MouseHookManager
- ✅ 基本实现完整
- ➕ 添加更完善的回调机制

### 3.2 在主文件中集成
```cpp
// anywp_engine_plugin.h
#include "modules/mouse_hook_manager.h"

class AnyWPEnginePlugin {
private:
  std::unique_ptr<MouseHookManager> mouse_hook_manager_;
  
  // 删除:
  // HHOOK mouse_hook_;
  // static AnyWPEnginePlugin* hook_instance_;
};
```

### 3.3 替换主文件中的调用
```cpp
// 旧代码
SetupMouseHook();

// 新代码
mouse_hook_manager_ = std::make_unique<MouseHookManager>();
mouse_hook_manager_->SetClickCallback([this](int x, int y, const char* event) {
  SendClickToWebView(x, y, event);
});
mouse_hook_manager_->Install();
```

### 3.4 删除主文件中的重复代码
- 删除 `SetupMouseHook()`
- 删除 `RemoveMouseHook()`
- 删除 `LowLevelMouseProc()`（~200 行）
- 删除 `CancelActiveDrag()`

**预期减少**: ~258 行

---

## Step 4: 创建 WindowManager 模块

### 4.1 创建新模块
```cpp
// windows/modules/window_manager.h
class WindowManager {
public:
  static HWND FindWorkerW();
  HWND CreateWebViewHost(bool transparent, const MonitorInfo* monitor);
  void TrackWindow(HWND hwnd);
  void CleanupAll();
};
```

### 4.2 实现模块
- 从主文件提取 `CreateWebViewHostWindow()` (~100 行)
- 从主文件提取 WorkerW 查找逻辑 (~50 行)
- 集成 `ResourceTracker` 类 (~50 行)

### 4.3 在主文件中集成
```cpp
window_manager_ = std::make_unique<WindowManager>();
HWND hwnd = window_manager_->CreateWebViewHost(transparent, monitor);
```

**预期减少**: ~200 行

---

## Step 5: 创建 WebViewLifecycleManager 模块

### 5.1 创建新模块
```cpp
// windows/modules/webview_lifecycle_manager.h
class WebViewLifecycleManager {
public:
  static void InitializeEnvironment(const std::wstring& user_data_folder);
  void CreateWebView(HWND hwnd, const std::string& url, Callback callback);
  bool Navigate(ICoreWebView2* webview, const std::string& url);
  void ConfigureWebView(ICoreWebView2* webview);
};
```

### 5.2 实现模块
- 从主文件提取 `SetupWebView2()` (~400 行)
- 从主文件提取 `ConfigurePermissions()` (~30 行)
- 从主文件提取 `SetupSecurityHandlers()` (~35 行)
- 从主文件提取环境管理逻辑 (~50 行)

### 5.3 在主文件中集成
```cpp
webview_manager_ = std::make_unique<WebViewLifecycleManager>();
webview_manager_->CreateWebView(hwnd, url, [this](auto controller, auto webview) {
  // 保存引用
});
```

**预期减少**: ~515 行

---

## Step 6: 完善 SDKBridge 和 IframeDetector

### 6.1 实现 SDKBridge
- 提取 `LoadSDKScript()` (~45 行)
- 提取 `InjectAnyWallpaperSDK()` (~40 行)
- 提取 `SetupMessageBridge()` (~33 行)
- 提取 `HandleWebMessage()` (~335 行)

**预期减少**: ~453 行

### 6.2 实现 IframeDetector
- 提取 `HandleIframeDataMessage()` (~187 行)
- 提取 `GetIframeAtPoint()` (~50 行)

**预期减少**: ~237 行

---

## Step 7: 创建 WallpaperInstanceManager

### 7.1 创建新模块
```cpp
// windows/modules/wallpaper_instance_manager.h
class WallpaperInstanceManager {
public:
  bool CreateInstance(int monitor_index, const std::string& url);
  bool DestroyInstance(int monitor_index);
  WallpaperInstance* GetInstance(int monitor_index);
  WallpaperInstance* GetInstanceAtPoint(int x, int y);
  void ExecuteScriptToAll(const std::wstring& script);
};
```

### 7.2 实现模块
- 从主文件提取实例管理逻辑 (~350 行)
- 从主文件提取 `GetInstanceForMonitor()` (~20 行)
- 从主文件提取 `GetInstanceAtPoint()` (~30 行)
- 从主文件提取 `IsOurWindow()` (~30 行)
- 从主文件提取 `ExecuteScriptToAllInstances()` (~45 行)

**预期减少**: ~475 行

---

## Step 8: 重构主文件

### 8.1 删除已拆分的代码
- 所有模块相关的实现代码
- 重复的成员变量
- 辅助函数

### 8.2 保留的内容（目标 <1000 行）
- 插件注册 (`RegisterWithRegistrar`) (~75 行)
- 构造/析构函数 (~50 行)
- 方法调用处理 (`HandleMethodCall`) (~300 行，简化后)
- 遗留 API 兼容层 (~200 行)
- 模块协调逻辑 (~200 行)
- 必要的辅助函数 (~175 行)

**预计最终行数**: ~1000 行

---

## 重构效果预估

### 行数减少
| 步骤 | 模块 | 减少行数 | 累计减少 |
|------|------|---------|---------|
| Step 1 | PowerManager | 613 | 613 |
| Step 2 | MonitorManager | 371 | 984 |
| Step 3 | MouseHookManager | 258 | 1242 |
| Step 4 | WindowManager | 200 | 1442 |
| Step 5 | WebViewLifecycleManager | 515 | 1957 |
| Step 6 | SDKBridge + IframeDetector | 690 | 2647 |
| Step 7 | WallpaperInstanceManager | 475 | 3122 |

**主文件**: 4176 - 3122 = **~1054 行** ✅

---

## 实施时间表

### Phase 1: 已有模块集成（3-4 小时）
- Step 1: PowerManager 集成 (1 小时)
- Step 2: MonitorManager 集成 (1 小时)
- Step 3: MouseHookManager 集成 (1 小时)
- 编译测试 (0.5 小时)

### Phase 2: 新模块创建（4-5 小时）
- Step 4: WindowManager (1.5 小时)
- Step 5: WebViewLifecycleManager (2 小时)
- Step 6: SDKBridge + IframeDetector (1.5 小时)
- 编译测试 (0.5 小时)

### Phase 3: 实例管理和整合（2-3 小时）
- Step 7: WallpaperInstanceManager (1.5 小时)
- Step 8: 主文件重构 (1 小时)
- 编译测试 (0.5 小时)

**总计**: 约 9-12 小时

---

## 测试策略

### 每个步骤后的测试清单
- [ ] 编译成功（无错误）
- [ ] 无 Linter 警告
- [ ] 基础功能测试（启动/停止）
- [ ] 内存检查（无泄漏）

### 完整测试清单（最终）
- [ ] 单显示器场景
- [ ] 多显示器场景
- [ ] 显示器热插拔
- [ ] 拖拽功能
- [ ] 点击功能
- [ ] 状态持久化
- [ ] 省电功能
- [ ] 全屏应用检测
- [ ] 内存优化
- [ ] 性能基准（与重构前对比）

---

## 风险缓解

### 风险 1: 集成失败
**缓解**: 
- 每步集成立即编译测试
- 保留 Git 提交历史
- 失败立即回滚

### 风险 2: 行为不一致
**缓解**:
- 保留原有逻辑不变
- 只做代码搬移，不修改算法
- 详细的对比测试

### 风险 3: 性能退化
**缓解**:
- 使用智能指针避免额外开销
- 避免不必要的拷贝
- 性能基准测试

---

## 成功标准

- ✅ 主文件 ≤1000 行
- ✅ 所有功能测试通过
- ✅ 内存占用 <300 MB
- ✅ 无性能退化（±5%）
- ✅ 代码可维护性显著提升
- ✅ 模块化架构清晰

---

**版本**: 1.0  
**更新**: 2025-11-09  
**状态**: 准备实施  
**预计完成**: 9-12 小时

