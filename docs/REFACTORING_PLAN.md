# C++ Deep Refactoring Plan

## 现状分析

**主文件**: `windows/anywp_engine_plugin.cpp` - **4176行** (过大！)

### 已有模块（v1.3.2）
- `windows/modules/iframe_detector.h/cpp` - iframe 检测
- `windows/modules/sdk_bridge.h/cpp` - JavaScript SDK 桥接
- `windows/modules/mouse_hook_manager.h/cpp` - 鼠标钩子
- `windows/modules/monitor_manager.h/cpp` - 显示器管理
- `windows/modules/power_manager.h/cpp` - 省电管理
- `windows/utils/state_persistence.h/cpp` - 状态持久化
- `windows/utils/url_validator.h/cpp` - URL 验证
- `windows/utils/logger.h/cpp` - 日志工具

### 主文件中仍需拆分的部分

#### 1. **WebView 生命周期管理** (~600行)
- `SetupWebView2()` - 400+行，负责创建和初始化 WebView2
- `ConfigurePermissions()` - 权限配置
- `SetupSecurityHandlers()` - 安全处理
- `InjectAnyWallpaperSDK()` - SDK 注入
- `LoadSDKScript()` - SDK 加载
- `SetupMessageBridge()` - 消息桥接
- 环境管理（`shared_environment_`）

#### 2. **窗口管理** (~400行)
- `CreateWebViewHostWindow()` - 创建 WebView 宿主窗口
- 查找 WorkerW 窗口逻辑
- 窗口消息处理 (`DisplayChangeWndProc`, `PowerSavingWndProc`)
- 资源跟踪 (`ResourceTracker` 类)

#### 3. **壁纸实例管理** (~350行)
- `InitializeWallpaperOnMonitor()`
- `StopWallpaperOnMonitor()`
- `NavigateToUrlOnMonitor()`
- `GetInstanceForMonitor()`
- `IsOurWindow()`
- 多实例存储和同步

#### 4. **显示器变更处理** (~300行)
- `SetupDisplayChangeListener()`
- `HandleDisplayChange()`
- `HandleMonitorCountChange()`
- `UpdateWallpaperSizes()`
- `NotifyMonitorChange()`
- `SafeNotifyMonitorChange()`

#### 5. **冲突检测** (~200行)
- `WallpaperConflictDetector` 类
- 检测其他壁纸软件进程

#### 6. **方法调用处理** (~300行)
- `HandleMethodCall()` - 处理 Dart 调用（300+行）
- 方法路由和参数解析

---

## 重构策略

### 阶段 1️⃣：创建新模块（渐进式，保障稳定）

#### 模块 1: `webview_lifecycle_manager.h/cpp`
**职责**: WebView2 完整生命周期管理
- 环境创建和共享
- WebView2 创建
- 权限和安全配置
- SDK 注入
- 导航管理
- 内存优化

**接口**:
```cpp
class WebViewLifecycleManager {
  static void InitializeEnvironment(const std::wstring& user_data_folder);
  void CreateWebView(HWND hwnd, const std::string& url, Callback callback);
  bool Navigate(ICoreWebView2* webview, const std::string& url);
  void ConfigureWebView(ICoreWebView2* webview);
  void InjectSDK(ICoreWebView2* webview);
};
```

#### 模块 2: `window_manager.h/cpp`
**职责**: Windows API 窗口管理
- WorkerW 窗口查找和管理
- WebView 宿主窗口创建
- 窗口消息处理
- 资源跟踪

**接口**:
```cpp
class WindowManager {
  static HWND FindWorkerW();
  HWND CreateWebViewHost(bool transparent, const MonitorInfo* monitor);
  void SetupMessageListener(HWND hwnd, WNDPROC proc);
  void TrackWindow(HWND hwnd);
  void CleanupAll();
};
```

#### 模块 3: `wallpaper_instance_manager.h/cpp`
**职责**: 多显示器壁纸实例管理
- 实例创建、销毁、查询
- 实例同步和线程安全
- 批量操作（暂停/恢复）

**接口**:
```cpp
class WallpaperInstanceManager {
  bool CreateInstance(int monitor_index, const std::string& url);
  bool DestroyInstance(int monitor_index);
  WallpaperInstance* GetInstance(int monitor_index);
  void PauseAll();
  void ResumeAll();
  void ExecuteScriptToAll(const std::wstring& script);
};
```

#### 模块 4: `display_change_handler.h/cpp`
**职责**: 显示器变更检测和响应
- 监听显示器变更
- 自动调整壁纸大小
- 新显示器自动启动壁纸
- 通知 Dart 层

**接口**:
```cpp
class DisplayChangeHandler {
  void StartListening();
  void StopListening();
  void HandleChange();
  void NotifyMonitorChange();
};
```

#### 模块 5: `conflict_detector.h/cpp` (独立工具类)
**职责**: 检测壁纸软件冲突
- 进程枚举
- 已知软件检测
- 冲突报告

**接口**:
```cpp
class ConflictDetector {
  static bool DetectConflicts(std::vector<ConflictInfo>& conflicts);
  static void ReportConflicts();
};
```

#### 模块 6: `method_call_router.h/cpp`
**职责**: 方法调用路由和参数解析
- 方法名路由
- 参数验证
- 结果返回

**接口**:
```cpp
class MethodCallRouter {
  using MethodHandler = std::function<void(const MethodCall&, MethodResult*)>;
  void RegisterHandler(const std::string& method, MethodHandler handler);
  void Route(const MethodCall& call, MethodResult* result);
};
```

---

### 阶段 2️⃣：重构主插件文件

**目标**: 将 `anywp_engine_plugin.cpp` 从 **4176行** 减少到 **<1000行**

**保留内容**:
- 插件注册 (`RegisterWithRegistrar`)
- 构造/析构函数
- 模块协调逻辑
- 简化的方法调用入口（委托给路由器）
- 遗留 API 的兼容性层

**移除内容**:
- 所有 WebView2 创建和配置代码 → `WebViewLifecycleManager`
- 窗口创建和 WorkerW 逻辑 → `WindowManager`
- 多实例管理逻辑 → `WallpaperInstanceManager`
- 显示器变更处理 → `DisplayChangeHandler`
- 冲突检测 → `ConflictDetector`
- 方法调用路由逻辑 → `MethodCallRouter`

---

### 阶段 3️⃣：测试和验证

**测试清单**:
- [ ] 编译成功（无警告）
- [ ] 单显示器场景
- [ ] 多显示器场景
- [ ] 显示器热插拔
- [ ] 拖拽功能
- [ ] 点击功能
- [ ] 状态持久化
- [ ] 省电功能
- [ ] 内存占用 (<300 MB)

---

## 重构原则

1. **渐进式**: 每次只拆分一个模块，立即测试
2. **向后兼容**: 保留所有公共 API 行为
3. **单一职责**: 每个模块只负责一个核心功能
4. **最小化依赖**: 模块之间通过接口交互
5. **错误处理**: 所有新模块使用 try-catch 和 Logger
6. **文档注释**: 所有公共接口添加完整文档

---

## 实施时间表

### Sprint 1 (预计 4 小时)
- 创建 `WebViewLifecycleManager` (2 小时)
- 创建 `WindowManager` (1 小时)
- 编译测试 (1 小时)

### Sprint 2 (预计 3 小时)
- 创建 `WallpaperInstanceManager` (1.5 小时)
- 创建 `DisplayChangeHandler` (1 小时)
- 编译测试 (0.5 小时)

### Sprint 3 (预计 2 小时)
- 创建 `ConflictDetector` (0.5 小时)
- 创建 `MethodCallRouter` (1 小时)
- 编译测试 (0.5 小时)

### Sprint 4 (预计 3 小时)
- 重构主插件文件 (2 小时)
- 完整功能测试 (1 小时)

**总计**: 约 12 小时

---

## 风险和缓解

### 风险 1: COM 对象生命周期管理复杂
**缓解**: 使用 `Microsoft::WRL::ComPtr` 智能指针，确保引用计数正确

### 风险 2: 多线程同步问题
**缓解**: 
- 所有跨模块调用使用 `std::lock_guard`
- 回调使用 Windows 消息队列

### 风险 3: 向后兼容性破坏
**缓解**: 
- 保留所有公共 API
- 在主插件文件中添加兼容性层
- 遗留代码路径保持不变

### 风险 4: 测试覆盖不足
**缓解**: 
- 每个模块添加单元测试
- 集成测试覆盖所有场景
- 性能基准测试（内存、CPU）

---

## 成功标准

- ✅ 主文件行数 <1000
- ✅ 所有测试通过
- ✅ 内存占用 <300 MB
- ✅ 无性能退化
- ✅ 代码可维护性显著提升
- ✅ 新增单元测试 ≥95% 覆盖率

---

**版本**: 1.0  
**更新**: 2025-11-09  
**状态**: 准备实施

