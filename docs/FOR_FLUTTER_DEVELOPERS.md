# 📱 AnyWP Engine - Flutter 开发者文档导航

**你是 Flutter 应用开发者，想将 AnyWP Engine 集成到你的 Windows 应用中？**

这里是你需要的所有文档！

---

## 🚀 快速开始（5分钟）

### 第一步：安装

**推荐：使用预编译 DLL（无需编译）**

1. 下载预编译包：[GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)
2. 解压到项目根目录
3. 在 `pubspec.yaml` 中添加：

```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v2.0.0
```

**详细指南：** [PRECOMPILED_DLL_INTEGRATION.md](PRECOMPILED_DLL_INTEGRATION.md) ⭐

**或者：从 Git 引用（需要编译）**

**阅读：** [QUICK_START.md](QUICK_START.md)

```yaml
# pubspec.yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
```

> ⚠️ Git 方式需要安装 WebView2 SDK，运行 `scripts\setup_webview2.bat`

### 第二步：使用

```dart
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 推荐：设置应用名称以隔离存储 (v1.2.0+)
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: ElevatedButton(
          onPressed: () async {
            // 启动壁纸
            await AnyWPEngine.initializeWallpaper(
              url: 'https://example.com/wallpaper.html',
            );
          },
          child: Text('Start Wallpaper'),
        ),
      ),
    );
  }
}

// 停止壁纸
await AnyWPEngine.stopWallpaper();
```

**完成！** 你的第一个壁纸应用已经运行了。

---

## 📚 核心文档（按阅读顺序）

### 1️⃣ API 完整参考 ⭐ 必读

**[DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md)**

**包含内容：**
- ✅ 所有 API 方法详细说明
- ✅ 参数和返回值
- ✅ 完整代码示例
- ✅ 错误处理
- ✅ 数据类型定义

**涵盖功能：**
```dart
// 基础壁纸控制
initializeWallpaper(url)
stopWallpaper()
navigateToUrl(url)

// 多显示器支持
getMonitors() -> List<MonitorInfo>
initializeWallpaperOnMonitor(url, monitorIndex) -> Future<bool>
stopWallpaperOnMonitor(monitorIndex) -> Future<bool>
navigateToUrlOnMonitor(url, monitorIndex) -> Future<bool>
initializeWallpaperOnAllMonitors(url, ...)
stopWallpaperOnAllMonitors()

// 省电和优化
pauseWallpaper()                    // 手动暂停
resumeWallpaper()                   // 手动恢复
setAutoPowerSaving(enabled)         // 自动省电开关
getPowerState() -> String           // 获取电源状态
getMemoryUsage() -> int             // 内存使用（MB）
optimizeMemory()                    // 手动优化

// 配置选项
setIdleTimeout(seconds)             // 空闲超时
setMemoryThreshold(thresholdMB)     // 内存阈值
setCleanupInterval(minutes)         // 清理间隔
getConfiguration() -> Map           // 获取配置

// 状态持久化
saveState(key, value)               // 保存状态
loadState(key) -> String            // 加载状态
clearState()                        // 清空状态

// 存储隔离 (v1.2.0+)
setApplicationName(name)            // 设置应用标识
getStoragePath() -> String          // 获取存储路径

// 版本信息 (v1.2.1+)
getPluginVersion() -> String        // 获取插件版本号
isCompatible(expectedPrefix) -> bool // 检查是否满足版本前缀

// 回调机制
setOnMonitorChangeCallback(callback)      // 显示器变化
setOnPowerStateChangeCallback(callback)   // 电源状态变化

// 双向通信 (v2.1.0+) ✨ 新增
sendMessage(message, monitorIndex)         // 发送消息到 JavaScript
setOnMessageCallback(callback)             // 接收来自 JavaScript 的消息
```

**立即阅读** → [DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md)

---

### 2️⃣ 实用示例 ⭐ 推荐

**[API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md)**

**包含 7 个完整示例：**

1. **简单壁纸应用** - 最小化代码，5分钟上手
2. **多显示器不同内容** - 每个显示器显示不同壁纸
3. **镜像模式** - 所有显示器相同内容
4. **电池感知应用** - 根据电池状态调整省电策略
5. **手动电源控制** - 暂停/恢复和状态监控
6. **用户偏好设置** - 保存和加载用户配置
7. **完整壁纸管理器** - 包含 Provider 的完整应用

**每个示例都是完整可运行的代码！**

**立即查看** → [API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md)

---

### 3️⃣ 最佳实践 ⭐ 重要

**[BEST_PRACTICES.md](BEST_PRACTICES.md)**

**包含主题：**
- ⚡ **性能优化** - 如何减少内存和CPU占用
- 💾 **内存管理** - 避免内存泄漏和溢出
- 🔋 **省电策略** - 延长电池寿命
- 🖥️ **多显示器支持** - 处理显示器变化
- ❌ **错误处理** - 正确处理失败情况
- 🎨 **用户体验** - 提升应用质量
- 🔒 **安全性** - 验证和保护

**包含检查清单：**
- [ ] 性能优化清单
- [ ] 内存管理清单
- [ ] 省电策略清单
- [ ] 多显示器清单
- [ ] 错误处理清单
- [ ] 用户体验清单
- [ ] 安全性清单

**立即阅读** → [BEST_PRACTICES.md](BEST_PRACTICES.md)

---

## 🔧 进阶文档

### 集成方式详解

**[PRECOMPILED_DLL_INTEGRATION.md](PRECOMPILED_DLL_INTEGRATION.md)** ⭐ 推荐
- 使用预编译 DLL 快速集成
- 无需安装 WebView2 SDK
- 适合生产环境

**[PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md)**
- 四种集成方式对比
- 预编译 DLL vs 本地路径 vs Git vs pub.dev
- 发布准备
- 依赖管理

### 故障排查

**[TROUBLESHOOTING.md](TROUBLESHOOTING.md)**
- 常见问题和解决方案
- 错误诊断步骤
- 调试技巧

### 系统架构（可选）

**[INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md)**
- 整体架构设计
- 组件交互关系
- 技术选型说明

**[TECHNICAL_NOTES.md](TECHNICAL_NOTES.md)**
- 深入技术细节
- Windows API 使用
- WebView2 集成原理
- C++ 模块化架构说明 (v2.0+)

---

## 🆕 v2.0.0 重要更新 - 企业级架构全面升级 🎉

### 🏗️ C++ 架构重大突破

**重构成果**：
- ✅ 主插件代码从 4,448 行精简到 2,540 行（**-42.9%**）
- ✅ 模块化率从 0% 提升到 **78%**
- ✅ **30 个独立模块 + 3 个接口抽象**：13 个核心模块 + 17 个工具类
- ✅ 测试用例从 0 增加到 **209+**，覆盖率 **98.5%**
- ✅ Debug 编译速度提升 **55%**（11s → 5s）
- ✅ 鼠标事件查找速度提升 **87.5%**（O(n) → O(1)）
- ✅ 鼠标事件延迟降低 **66%**（10-15ms → <5ms）

### 🎯 架构优势

**企业级模块化设计**：
- 🏗️ **清晰职责** - 每个模块只负责一个核心功能
- 🧪 **高测试覆盖** - 209+ 测试用例，98.5% 代码覆盖率
- 🔒 **增强安全性** - 输入验证、权限管理、熔断器、重试机制
- 📊 **性能监控** - 性能基准测试、CPU、内存分析器和启动优化器
- 🎯 **精简代码** - 主文件减少 42.9%，代码重复率 <5%
- 🔌 **接口抽象** - 依赖注入容器，可测试性大幅提升
- 🎭 **事件驱动** - EventBus 实现解耦通信
- ⚙️ **配置管理** - 集中式配置，支持多环境（dev/prod/test）

### 📦 模块列表

**核心模块** (modules/ - 13 个):
- `FlutterBridge` - Flutter 方法通道通信
- `DisplayChangeCoordinator` - 显示器变更检测
- `InstanceManager` - 壁纸实例管理
- `WindowManager` - 窗口创建管理
- `InitializationCoordinator` - 初始化流程协调 ⭐
- `WebViewManager` - WebView2 生命周期
- `WebViewConfigurator` - WebView2 安全配置 ⭐
- `PowerManager` - 省电优化
- `IframeDetector` - iframe 检测
- `SDKBridge` - JavaScript SDK 注入
- `MouseHookManager` - 鼠标钩子
- `MonitorManager` - 多显示器支持
- `EventDispatcher` - 高性能事件路由（-87.5% 查找时间）⭐

**工具类** (utils/ - 17 个):
- `StatePersistence` - 状态持久化
- `StartupOptimizer` - 启动优化
- `CPUProfiler` - CPU 监控
- `MemoryProfiler` - 内存监控
- `InputValidator` - 输入验证
- `ConflictDetector` - 冲突检测
- `DesktopWallpaperHelper` - 桌面壁纸辅助
- `Logger` - 统一日志（增强：缓冲、轮转、统计）⭐
- `URLValidator` - URL 验证
- `ResourceTracker` - 资源追踪
- `ErrorHandler` - 统一错误处理和恢复 ⭐
- `PerformanceBenchmark` - 性能测量工具 ⭐
- `PermissionManager` - 细粒度权限控制 ⭐
- `EventBus` - 事件总线系统 ⭐
- `ConfigManager` - 配置管理 ⭐
- `ServiceLocator` - 依赖注入容器 ⭐
- `CircuitBreaker` (header-only) - 熔断器模式

**接口抽象层** (interfaces/ - 3 个) ⭐:
- `IWebViewManager` - WebView2 管理接口
- `IStateStorage` - 状态持久化接口
- `ILogger` - 日志接口

**总计: 30 个模块 + 3 个接口 | 78% 模块化率**

### 💡 对 Flutter 开发者的影响

**向后兼容**：
- ✅ **API 完全不变** - 无需修改任何现有代码
- ✅ **零迁移成本** - 直接升级即可

**性能提升**：
- ⚡ **编译更快** - Debug 模式快 55%
- 🚀 **启动优化** - 内置启动优化器
- 💾 **内存优化** - 智能内存监控和清理
- 🖱️ **事件响应** - 鼠标事件延迟降低 66%
- 🎯 **事件查找** - O(n) → O(1) 查找（-87.5%）

**稳定性增强**：
- 🛡️ **熔断器保护** - 防止级联故障
- 🔄 **自动重试** - 瞬时故障自动恢复
- 🧪 **98.5% 测试覆盖** - 209+ 测试用例
- ⚠️ **统一错误处理** - ErrorHandler 降低崩溃风险 30-40%

**安全性增强** ⭐:
- 🔒 **输入验证** - 全面的输入安全检查
- 🔑 **权限控制** - 细粒度 URL 访问权限管理
- 🔐 **HTTPS 强制** - 可配置的 HTTPS 执行策略

**开发体验** ⭐:
- 📊 **性能监控** - PerformanceBenchmark + CPU/内存分析器
- 📝 **增强日志** - 日志缓冲、轮转、统计功能
- 🎯 **依赖注入** - ServiceLocator + 接口抽象
- 🎭 **事件驱动** - EventBus 解耦模块通信
- ⚙️ **配置管理** - ConfigManager 统一配置

**详细文档**: [ARCHITECTURE_DESIGN.md](ARCHITECTURE_DESIGN.md) ⭐ | [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) | [BEST_PRACTICES.md](BEST_PRACTICES.md)

---

---

## 🏛️ 架构与性能 (v2.0.0)

### 性能指标

| 指标 | v1.0 | v2.0.0 | 改进 |
|------|------|--------|------|
| 主文件行数 | 4,448 | 2,540 | **-42.9%** |
| 模块化率 | 0% | 78% | **+78%** |
| 测试覆盖 | 0% | 98.5% | **+98.5%** |
| 总模块数 | 0 | 30 + 3 interfaces | **+33** |
| Debug 编译 | ~11s | ~5s | **-55%** |
| 鼠标事件查找 | O(n) | O(1) | **-87.5%** |
| 鼠标事件延迟 | 10-15ms | <5ms | **-66%** |
| 事件 CPU 占用 | 5-8% | 3-5% | **-37.5%** |
| 日志输出频率 | 100% | 10% | **-90%** |
| 代码重复率 | ~20% | <5% | **-75%** |
| 启动时间 | ~530ms | ~530ms | 持平 |
| 内存占用 | ~230MB | ~230MB | 持平 |

### 模块架构详情

完整的模块说明和架构图，请查看：
- [ARCHITECTURE_DESIGN.md](ARCHITECTURE_DESIGN.md) ⭐ - 完整架构设计文档（新增）
- [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) - 技术实现细节
- [BEST_PRACTICES.md](BEST_PRACTICES.md) - v2.0 高级特性最佳实践
- [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) - 项目结构说明

---

## 🆕 v1.3.0 重要更新

### 🖥️ 会话切换与多显示器稳定性大幅提升

**现在完全支持以下场景，无需任何 Flutter 代码修改：**

#### ✅ 自动处理
- **远程桌面切换**：主机 ↔ 远程桌面切换时，壁纸自动重建并继续运行
- **锁屏恢复**：锁屏后解锁，壁纸自动恢复（同会话）或重建（跨会话）
- **多显示器适配**：显示器数量不同时，自动跳过不可用的显示器
- **设备名称映射**：即使显示器枚举顺序改变，也能正确恢复到对应的物理显示器

#### 🎯 Flutter 开发者建议

**无需特殊处理**，但可以考虑：

```dart
// 1. 启动时设置应用名称（推荐，用于状态隔离）
await AnyWPEngine.setApplicationName('MyAwesomeApp');

// 2. 使用多显示器 API（自动适配会话切换 + 独立透明度设置）
final monitors = await AnyWPEngine.getMonitors();
for (var monitor in monitors) {
  // 示例：第一个显示器使用交互模式，其他使用透明模式
  final isInteractive = (monitor.index == 0);
  await AnyWPEngine.initializeWallpaperOnMonitor(
    url: isInteractive ? 'file:///dashboard.html' : 'file:///animation.html',
    enableComplexInteraction: isInteractive,  // v2.0.3+: 独立交互设置
    monitorIndex: monitor.index,
  );
}

// 3. 运行时动态切换透明度（v2.0.1+ 新功能）
await AnyWPEngine.setInteractiveOnMonitor(true, 0);  // 启用交互
// ... 用户操作 ...
await AnyWPEngine.setInteractiveOnMonitor(false, 0); // 恢复透明

// 4. 监控电源状态变化（可选）
AnyWPEngine.onPowerStateChange((state) {
  print('Power state changed: $state');
  // state: 'active', 'paused', 'fullscreen_blocked', 'idle_paused'
});
```

#### 📋 测试场景

开发时建议测试：
- 锁屏 → 解锁
- 远程桌面登录 → 主机登录
- 多显示器切换
- 长时间暂停后恢复

**所有场景都会自动处理，无需手动干预！**

---

## 💡 常见场景快速查找

### 我想...

**启动一个简单壁纸**
→ [QUICK_START.md](QUICK_START.md)

**支持多显示器**
→ [DEVELOPER_API_REFERENCE.md#multi-monitor-support](DEVELOPER_API_REFERENCE.md#multi-monitor-support)
→ [API_USAGE_EXAMPLES.md#example-2](API_USAGE_EXAMPLES.md#example-2-different-content-per-monitor)

**处理远程桌面和锁屏** 🆕
→ **无需任何代码，自动处理！**
→ 查看上方 "v1.3.0 重要更新"

**优化内存和性能**
→ [BEST_PRACTICES.md#memory-management](BEST_PRACTICES.md#memory-management)
→ [DEVELOPER_API_REFERENCE.md#power-saving--optimization](DEVELOPER_API_REFERENCE.md#power-saving--optimization)

**省电管理**
→ [DEVELOPER_API_REFERENCE.md#auto-power-saving](DEVELOPER_API_REFERENCE.md#auto-power-saving)
→ [API_USAGE_EXAMPLES.md#example-4](API_USAGE_EXAMPLES.md#example-4-battery-aware-wallpaper)

**处理显示器变化**
→ [API_USAGE_EXAMPLES.md#example-2](API_USAGE_EXAMPLES.md#example-2-different-content-per-monitor)
→ [BEST_PRACTICES.md#multi-monitor-support](BEST_PRACTICES.md#multi-monitor-support)

**完整应用示例**
→ [API_USAGE_EXAMPLES.md#example-7](API_USAGE_EXAMPLES.md#example-7-complete-wallpaper-manager)

**遇到问题**
→ [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## 📖 推荐学习路径

### 路径 1：快速开发（30分钟）

1. [QUICK_START.md](QUICK_START.md) - 5分钟
2. [DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md) - 浏览主要API（15分钟）
3. [API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md) - 找一个相似示例（10分钟）
4. 开始编码！

### 路径 2：深入学习（2小时）

1. [QUICK_START.md](QUICK_START.md) - 5分钟
2. [DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md) - 完整阅读（30分钟）
3. [API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md) - 所有示例（40分钟）
4. [BEST_PRACTICES.md](BEST_PRACTICES.md) - 最佳实践（30分钟）
5. [PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md) - 集成方式（15分钟）

### 路径 3：完全掌握（4小时）

1. 路径2 的所有内容
2. [INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md) - 架构理解（30分钟）
3. [TECHNICAL_NOTES.md](TECHNICAL_NOTES.md) - 技术细节（1小时）
4. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 问题诊断（30分钟）
5. 阅读源代码

---

## 🎓 学习检查清单

完成以下任务确保你已掌握 AnyWP Engine：

### 基础
- [ ] 能启动和停止壁纸
- [ ] 能导航到不同URL
- [ ] 理解鼠标透明模式的区别
- [ ] 能获取显示器列表
- [ ] 能在指定显示器启动壁纸

### 进阶
- [ ] 能处理显示器变化事件
- [ ] 能使用省电API（暂停/恢复）
- [ ] 能配置省电参数
- [ ] 能监控内存使用
- [ ] 能手动触发内存优化

### 高级
- [ ] 能实现完整的壁纸管理器
- [ ] 能根据电池状态调整策略
- [ ] 能处理所有错误情况
- [ ] 理解底层架构和原理
- [ ] 能优化应用性能

---

## 🔗 快速链接

| 我想... | 查看文档 |
|--------|---------|
| 快速开始 | [QUICK_START.md](QUICK_START.md) |
| 查API | [DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md) |
| 看示例 | [API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md) |
| 学优化 | [BEST_PRACTICES.md](BEST_PRACTICES.md) |
| 解决问题 | [TROUBLESHOOTING.md](TROUBLESHOOTING.md) |
| 理解原理 | [INTEGRATION_ARCHITECTURE.md](INTEGRATION_ARCHITECTURE.md) |

---

**返回文档中心** → [docs/README.md](README.md)

