# 更新日志

所有重要的项目变更都将记录在此文件中。

## [1.3.2] - 2025-11-08 - 🚀 稳定版本发布

### 📦 版本说明

此版本是 AnyWP Engine 的重要稳定版本，整合了 C++ 模块化重构、TypeScript SDK 完整重写、文档规范化等多项重大改进。

### ✨ 主要特性

#### C++ 模块化架构
- **模块化设计**：核心插件从 4000+ 行拆分为功能独立的模块
- **5大功能模块**：IframeDetector, SDKBridge, MouseHookManager, MonitorManager, PowerManager
- **3大工具类**：StatePersistence, URLValidator, Logger
- **单元测试框架**：轻量级 C++ 测试框架，支持自动注册和丰富断言
- **完善错误处理**：所有模块添加 try-catch 保护和详细日志

#### TypeScript SDK 重写
- **100% TypeScript**：完整类型安全的 SDK 实现
- **模块化架构**：核心、事件、拖拽、点击、存储、SPA、动画等独立模块
- **单元测试覆盖**：118 个测试用例，96.6% 通过率，~71% 代码覆盖
- **现代构建流程**：使用 Rollup 构建，支持 ES Module 和 UMD

#### 文档规范化
- **文档验证系统**：自动化验证文档准确性和完整性
- **7大验证类型**：语言规范、文件引用、版本一致性、链接、代码示例、脚本、发布包
- **全英文规范**：根目录文档全部使用英文，技术文档提供中英双语
- **脚本规范**：所有脚本使用英文编写，禁止特殊字符

### 🔧 技术改进

#### 日志系统
- **多级别日志**：DEBUG, INFO, WARNING, ERROR
- **线程安全**：使用 std::mutex 保护并发写入
- **双输出支持**：控制台和文件同时输出
- **毫秒级时间戳**：精确的时间记录

#### 错误处理
- **全面异常捕获**：关键操作包装在 try-catch 中
- **回调保护**：回调函数添加额外异常保护层
- **状态回滚**：操作失败时自动恢复状态
- **详细错误信息**：记录完整错误堆栈和 Windows API 错误码

#### 测试体系
- **C++ 单元测试**：轻量级测试框架，支持快速验证
- **TypeScript 测试**：Jest 测试框架，完整覆盖所有模块
- **集成测试脚本**：自动化测试流程，性能监控

### 📚 文档更新

- ✅ **C++ 模块文档**：完整的模块化架构说明
- ✅ **TypeScript SDK 文档**：类型定义和 API 参考
- ✅ **测试框架文档**：单元测试使用指南
- ✅ **发版流程文档**：详细的发布检查清单
- ✅ **脚本参考文档**：所有脚本的使用说明

### 🎯 使用示例

#### C++ 模块使用
```cpp
// 使用日志系统
Logger::Instance().Info("MyModule", "Operation started");

// 使用状态持久化
StatePersistence state("MyApp");
state.SaveValue("key", "value");
std::string value = state.LoadValue("key");
```

#### TypeScript SDK 使用
```typescript
// 类型安全的 API 调用
import type { AnyWPClickEvent } from './types';

AnyWP.onClick(element, (event: AnyWPClickEvent) => {
  console.log('Clicked at:', event.x, event.y);
});

// 拖拽功能
AnyWP.makeDraggable(element, {
  onDragStart: (e) => console.log('Drag started'),
  onDragEnd: (e) => console.log('Drag ended')
});
```

### 🔄 升级指南

从 v1.3.1 升级到 v1.3.2：

1. **更新依赖**：
   ```bash
   flutter pub upgrade
   ```

2. **重新构建**：
   ```bash
   flutter clean
   flutter build windows --release
   ```

3. **无需代码修改**：此版本完全向后兼容

### 🐛 已知问题

无重大已知问题。如有问题请访问 [GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)。

---

## [1.3.1] - 2025-11-08 - 📝 文档规范与脚本优化

### ✨ 新增功能

#### 文档验证系统
- **验证脚本**：创建 `scripts/verify_docs.bat` 全面验证文档
- **7大验证**：语言规范、文件引用、版本一致性、链接、代码示例、脚本、发布包
- **自动化检查**：发布前自动验证文档准确性和完整性

### 📚 文档更新

#### 规范完善
- **脚本规范**：强制全英文编写，禁止 emoji（文档中可用）
- **语言要求**：根目录 README.md 必须全英文
- **发版检查**：新增文档准确性与完整性检查清单

#### 内容优化
- **README.md**：全部翻译为英文
- **脚本输出**：统一使用简单英文格式
- **验证模板**：提供批处理脚本验证模板

### 🔧 技术细节

#### PowerShell 脚本规范
- **禁用 Here-String**：移除所有 `@" ... "@` 语法
- **安全提交方式**：使用 `echo >` 重定向处理中文
- **批处理优先**：复杂逻辑使用 .bat 脚本

#### 验证脚本功能
- 语言合规性检查（中文检测）
- 文件引用有效性验证
- 版本号一致性检查
- 链接正确性验证
- 代码示例可用性检查
- 脚本语言规范检查
- 发布包完整性验证

### 🎯 使用示例

```bash
# 发布前运行验证
.\scripts\verify_docs.bat

# 输出示例：
# [Section 1] Language Compliance Check
#   [OK] README.md is English only
#   [OK] QUICK_INTEGRATION.md is English only
# [Section 2] File Reference Validation
#   [OK] All referenced files exist
# ... (7 sections total)
```

---

## [4.9.0] - 2025-11-08 - 🛡️ C++ 模块优化与测试框架

### ✨ 新增功能

#### 单元测试框架
- **测试框架**：创建轻量级 C++ 单元测试框架
- **自动注册**：使用宏自动注册测试用例
- **丰富断言**：ASSERT_TRUE, ASSERT_FALSE, ASSERT_EQUAL 等
- **清晰输出**：彩色输出测试结果（✅/❌）

#### 日志系统增强
- **多级别日志**：DEBUG, INFO, WARNING, ERROR
- **线程安全**：使用 std::mutex 保护日志写入
- **双输出**：支持控制台和文件输出
- **精确时间戳**：毫秒级时间戳
- **可配置**：可设置最小日志级别

### 🔧 重构改进

#### 错误处理完善
- **异常捕获**：所有关键模块添加 try-catch
- **状态回滚**：失败时自动回滚状态
- **详细日志**：记录完整错误信息和堆栈
- **错误码**：Windows API 调用记录错误码

#### 回调机制优化
- **异常保护**：回调执行包装在 try-catch 中
- **错误隔离**：回调异常不影响主流程
- **详细日志**：记录回调执行状态

### 📚 文档更新

- ✅ `docs/OPTIMIZATION_COMPLETE.md` - 优化完成报告
- ✅ `windows/test/test_framework.h` - 测试框架文档
- ✅ `windows/test/run_tests.bat` - 测试运行脚本

### 🔧 技术细节

#### 测试框架实现
```cpp
TEST_SUITE(PowerManager) {
  TEST_CASE(initialization) {
    PowerManager manager;
    ASSERT_FALSE(manager.IsEnabled());
    ASSERT_EQUAL(PowerManager::PowerState::ACTIVE, 
                 manager.GetCurrentState());
  }
}
```

#### 日志系统使用
```cpp
// 使用宏记录日志
ANYWP_LOG_INFO("Component", "Operation completed");
ANYWP_LOG_ERROR("Component", "Critical error");

// 配置日志
Logger::Instance().SetMinLevel(Logger::Level::DEBUG);
Logger::Instance().EnableFileLogging("debug.log");
```

#### 错误处理模式
```cpp
try {
  // 执行操作
  if (callback_) {
    try {
      callback_(result);
    } catch (const std::exception& e) {
      Logger::Instance().Error("Module", 
        "Callback failed: " + std::string(e.what()));
    }
  }
} catch (const std::exception& e) {
  Logger::Instance().Error("Module", 
    "Operation failed: " + std::string(e.what()));
  return false;
}
```

### 🎯 使用示例

#### 运行单元测试
```bash
cd windows\test
run_tests.bat
```

#### 自定义日志
```cpp
// 初始化时配置
Logger::Instance().SetMinLevel(Logger::Level::INFO);
Logger::Instance().EnableFileLogging("app.log");
Logger::Instance().EnableConsoleLogging(true);
```

### 📁 新增文件

- `windows/test/test_framework.h` - 测试框架头文件
- `windows/test/unit_tests.cpp` - 示例测试用例
- `windows/test/run_tests.bat` - 测试构建脚本
- `docs/OPTIMIZATION_COMPLETE.md` - 优化完成报告

### 🏆 质量提升

- **健壮性**: 所有关键路径添加异常处理
- **可调试性**: 详细的错误日志和错误码
- **可测试性**: 完整的单元测试框架
- **可维护性**: 统一的错误处理模式

---

## [4.8.0] - 2025-11-07 - 🎯 显示器热插拔完整实现

### ✨ 新增功能

#### 显示器配置记忆
- **智能保存**：拔掉显示器前自动保存 URL 和运行状态
- **精确恢复**：插回显示器时恢复原有配置（基于设备名称识别）
- **状态感知**：只在壁纸运行状态下才自动恢复

#### URL 失败回退机制
- **智能回退**：URL 加载失败时自动尝试主显示器的 URL
- **防死循环**：只使用已成功运行的 URL 做回退源
- **多级保护**：主显示器失败则停止尝试，避免无限循环

#### 窗口位置保存
- **位置记忆**：使用 `window_manager` 包保存窗口位置
- **自动恢复**：显示器变化后 500ms 自动恢复到原位置
- **防止跳动**：解决 Windows 系统在显示器变化时窗口跳动问题

### 🔧 技术实现

#### 配置保存时机优化
**关键修复**：在 `_loadMonitors()` 清理 controllers 之前保存配置
```dart
// 1. 获取新显示器列表（不更新 state）
final newMonitors = await AnyWPEngine.getMonitors();

// 2. 立即保存移除显示器的配置（controllers 还存在）
for (final removedIndex in removedIndices) {
  final url = _monitorUrlControllers[removedIndex]!.text;
  _monitorConfigMemory[deviceName] = MonitorConfig(...);
}

// 3. 然后才刷新列表（清理 controllers）
await _loadMonitors();
```

#### 智能恢复策略
```dart
// 优先级 1：恢复保存的配置（基于 deviceName）
if (_monitorConfigMemory.containsKey(newMonitor.deviceName)) {
  if (savedConfig.wasRunning) {
    urlToUse = savedConfig.url;  // 使用保存的 URL
  }
}

// 优先级 2：使用当前活跃的壁纸
else if (hasActiveWallpaper) {
  urlToUse = activeWallpaperUrl;
}

// 优先级 3：不自动启动
else {
  // 不应用壁纸
}
```

#### 窗口位置管理
```dart
// 监听窗口移动
@override
void onWindowMoved() async {
  _savedWindowPosition = await windowManager.getPosition();
}

// 显示器变化时恢复位置
Future.delayed(Duration(milliseconds: 500), () async {
  await windowManager.setPosition(_savedWindowPosition!);
});
```

### 🎯 使用示例

#### 基础使用（自动化）

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 初始化窗口管理器（防止跳动）
  await windowManager.ensureInitialized();
  
  // 设置应用名称
  await AnyWPEngine.setApplicationName('MyApp');
  
  runApp(MyApp());
}

class _MyAppState extends State<MyApp> with WindowListener {
  Map<String, MonitorConfig> _monitorConfigMemory = {};
  
  @override
  void initState() {
    super.initState();
    
    // 注册窗口监听器
    windowManager.addListener(this);
    
    // 启动显示器轮询（每 3 秒）
    Timer.periodic(Duration(seconds: 3), (timer) {
      _checkMonitorChanges();
    });
  }
}
```

#### 完整场景演示

**场景 1：双显示器不同内容**
```dart
// 主显示器运行 Simple
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_simple.html',
  monitorIndex: 0,
);

// 副显示器运行 Draggable
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_draggable.html',
  monitorIndex: 1,
);

// 🔌 拔掉副显示器
// → 自动保存：💾 Saved config for \\.\DISPLAY2: URL=test_draggable.html, Running=true

// 🔌 插回副显示器
// → 自动恢复：📂 Found saved config, ✅ Will RESTORE: test_draggable.html
// → 副显示器显示 Draggable（不是 Simple）✓
```

**场景 2：URL 失败回退**
```dart
// 主显示器运行正常页面
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///test_simple.html',
  monitorIndex: 0,  // 成功 ✓
);

// 保存了错误的副显示器配置
_monitorConfigMemory['\\.\DISPLAY2'] = MonitorConfig(
  url: 'file:///error.html',  // 不存在的文件
  wasRunning: true,
);

// 🔌 插回副显示器
// → 尝试：file:///error.html → ❌ FAILED
// → 回退：file:///test_simple.html → ✅ SUCCESS
// → 副显示器使用主显示器的 URL ✓
```

**场景 3：窗口位置保持**
```dart
// 双显示器状态，窗口在位置 A
// 🔌 拔掉副显示器 → 单显示器
// 用户拖动窗口到位置 B
// 🔌 插回副显示器
// → Windows 尝试移动窗口回位置 A
// → 500ms 后自动恢复到位置 B ✓
```

### 📚 新增依赖

```yaml
dependencies:
  window_manager: ^0.3.7  # 窗口位置管理
```

### 🔍 调试日志

完整的 emoji 标记日志系统：

```
💾 Saved config for \\.\DISPLAY2:
   URL: file:///副屏页面.html
   Running: true

📂 Found saved config for \\.\DISPLAY2:
   Saved URL: file:///副屏页面.html
   Was Running: true
   Last Seen: 2025-11-07 ...

✅ Will RESTORE previous wallpaper on monitor 1

▶️ Starting wallpaper on monitor 1
   Device: \\.\DISPLAY2
   URL: file:///副屏页面.html
   Controller updated with URL
   Result: ✅ SUCCESS

🔄 Using active wallpaper URL from monitor 0: ...
⚠️ No active wallpaper found to copy
❌ No saved config found for ...
🔍 No saved config or wasn't running, checking for active wallpapers...
```

### ⚠️ 注意事项

1. **显示器识别**：基于 `deviceName`（如 `\\.\DISPLAY1`）而非索引
2. **配置持久化**：仅在内存中保存，应用重启后需重新学习
3. **窗口位置**：需要 `window_manager` 包，500ms 延迟避免与 Windows 冲突
4. **回退保护**：只使用运行成功的 URL 做回退源，防止死循环
5. **轮询间隔**：3 秒检查一次，平衡响应速度和性能

### 🎬 测试场景

| 场景 | 预期行为 | 状态 |
|------|----------|------|
| 双显示器不同内容 | 拔插后各自恢复原内容 | ✅ |
| URL 失败回退 | 自动尝试主显示器 URL | ✅ |
| 主副都失败 | 停止尝试，不死循环 | ✅ |
| 窗口位置保持 | 拔插后位置不变 | ✅ |
| 无运行壁纸插入 | 不自动启动 | ✅ |

### 🐛 修复的问题

1. **副屏配置丢失**：修复了保存时机，确保在 controllers 清理前保存
2. **URL 回退死循环**：只使用成功运行的 URL 做回退
3. **窗口位置跳动**：使用 `window_manager` 自动恢复位置
4. **配置混淆**：使用 `deviceName` 精确匹配，不依赖索引

---

## [4.7.0] - 2025-11-07 - 🔥 显示器热插拔自动化

### ✨ 新增功能

#### 显示器热插拔自动化
- **自动检测显示器接入**：系统接入新显示器时，自动检测并通知应用层
- **自动应用壁纸**：新显示器接入后，自动在其上启动壁纸（使用当前活跃的 URL）
- **自动清理资源**：显示器移除时，自动停止该显示器上的壁纸并清理资源
- **统一内容展示**：新显示器自动显示与主显示器一致的内容

### 🔧 技术改进

#### C++ 层修复
- **启用 InvokeMethod 回调**：修复了之前被注释掉的 `onMonitorChange` 回调机制
- **线程安全通信**：使用 `SafeNotifyMonitorChange()` 和 `WM_NOTIFY_MONITOR_CHANGE` 消息机制，确保跨线程安全
- **实时通知**：显示器配置变化时，C++ 层立即通过 MethodChannel 通知 Dart 层

#### Dart 层增强
- **智能 URL 检测**：自动查找当前运行的壁纸 URL，优先使用活跃显示器的 URL
- **增量更新逻辑**：通过 Set 差集计算新增和移除的显示器，精确识别变化
- **自动启动策略**：检测到新显示器后，自动调用 `initializeWallpaperOnMonitor()` 启动壁纸
- **用户友好提示**：所有操作都有清晰的日志和 UI 提示

### 🎯 使用示例

#### 显示器热插拔场景（全自动）

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 1. 注册显示器变化回调（一次性设置）
  AnyWPEngine.setOnMonitorChangeCallback(() {
    print('显示器配置已变化，正在自动处理...');
  });
  
  runApp(MyApp());
}

// 在主显示器上启动壁纸
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  monitorIndex: 0,
  enableMouseTransparent: true,
);

// 🔌 用户接入第二个显示器
// → 系统自动检测到新显示器 ✓
// → 应用自动在新显示器上启动相同的壁纸 ✓
// → 用户看到提示："Auto-started wallpaper on 1 new monitor(s)" ✓

// 🔌 用户移除第二个显示器
// → 系统自动检测到显示器移除 ✓
// → 应用自动清理该显示器的资源 ✓
// → 用户看到提示："Display configuration changed" ✓
```

#### 示例应用中的实现

示例应用 (`example/lib/main.dart`) 已经实现了完整的自动化逻辑：

```dart
// 在 initState 中注册回调
AnyWPEngine.setOnMonitorChangeCallback(() {
  _handleMonitorChange();
});

// 自动处理逻辑
Future<void> _handleMonitorChange() async {
  // 1. 检测新增显示器
  final addedIndices = newIndices.difference(oldIndices);
  
  if (addedIndices.isNotEmpty) {
    // 2. 查找当前活跃的壁纸 URL
    String? activeUrl;
    for (final entry in _monitorWallpapers.entries) {
      if (entry.value == true) {
        activeUrl = _monitorUrlControllers[entry.key]!.text;
        break;
      }
    }
    
    // 3. 在新显示器上自动启动壁纸
    for (final index in addedIndices) {
      await AnyWPEngine.initializeWallpaperOnMonitor(
        url: activeUrl,
        monitorIndex: index,
        enableMouseTransparent: _mouseTransparent,
      );
    }
  }
}
```

### 📚 用户体验提升

**之前的流程**：
1. 接入新显示器
2. 手动点击"Refresh"按钮刷新显示器列表
3. 手动在新显示器的 URL 输入框中输入或选择 URL
4. 手动点击"Start"按钮启动壁纸

**现在的流程**：
1. 接入新显示器 → **完全自动，无需任何操作** ✓

**提升效果**：
- ✅ **零操作**：用户无需手动干预，插入显示器即可使用
- ✅ **即时响应**：显示器接入后立即自动应用壁纸
- ✅ **内容一致**：新显示器自动显示与主显示器相同的内容
- ✅ **智能清理**：显示器移除时自动清理资源，避免内存泄漏

### 🔍 技术细节

#### 显示器变化检测流程

```
[系统层] Windows 发送 WM_DISPLAYCHANGE 消息
    ↓
[C++ 窗口过程] DisplayChangeWndProc 接收消息
    ↓
[C++ HandleDisplayChange] 重新枚举显示器，计算变化
    ↓
[C++ SafeNotifyMonitorChange] 发送 WM_NOTIFY_MONITOR_CHANGE 到消息队列
    ↓
[C++ NotifyMonitorChange] 在安全线程上调用 InvokeMethod
    ↓
[Dart MethodChannel] 接收 "onMonitorChange" 回调
    ↓
[Dart _handleMonitorChange] 计算新增/移除的显示器
    ↓
[Dart 自动应用] 在新显示器上调用 initializeWallpaperOnMonitor
```

#### 线程安全保证

1. **WM_DISPLAYCHANGE**：来自 Windows 消息循环，可能不在 Flutter UI 线程
2. **PostMessage**：将通知请求放入消息队列，延迟到窗口过程处理
3. **WM_NOTIFY_MONITOR_CHANGE**：在窗口过程中处理，与 Flutter 引擎同步
4. **InvokeMethod**：在正确的线程上下文中调用，避免崩溃

### ⚠️ 注意事项

1. **首次启动**：应用首次启动时，没有活跃的壁纸 URL，需要手动启动第一个显示器
2. **URL 匹配**：自动应用使用当前任何一个运行中的显示器的 URL（优先使用运行状态的）
3. **兼容性**：该功能完全向后兼容，不影响现有代码

### 🎬 演示场景

**场景 1：双显示器扩展**
- 用户在主显示器上启动壁纸
- 接入第二个显示器 → 第二个显示器自动显示相同壁纸 ✓

**场景 2：笔记本+外接显示器**
- 用户在笔记本屏幕启动壁纸
- 连接外接显示器 → 外接显示器自动显示壁纸 ✓
- 断开外接显示器 → 系统自动清理资源 ✓

**场景 3：会议室投影**
- 用户在办公桌启动壁纸（单显示器）
- 进入会议室连接投影仪 → 投影仪自动显示壁纸 ✓
- 离开会议室断开投影仪 → 笔记本屏幕继续显示壁纸 ✓

---

## [4.6.0] - 2025-11-07 - 🖥️ 会话切换与多显示器稳定性大幅提升

### ✨ 新增功能

#### 跨会话稳定性
- **完整的远程桌面支持**：主机 ↔ 远程桌面切换时自动重建壁纸，保持持续可见
- **锁屏智能恢复**：锁屏后再解锁，壁纸自动恢复；跨会话锁屏也能正确重建
- **多显示器持久化**：使用设备名称而非索引标识显示器，即使枚举顺序改变也能正确恢复

#### 增强的多显示器支持
- **设备名称映射**：自动将保存的设备名称（如 `\\.\DISPLAY1`）映射到当前会话的索引
- **智能跳过**：重建时自动跳过当前会话不存在的显示器，避免初始化错误
- **完整恢复**：所有显示器的壁纸都能在会话切换后正确恢复

### 🔄 重构改进

#### 会话管理系统完善
- **统一决策函数**：`ShouldWallpaperBeActive()` 基于锁屏状态决定壁纸是否应该激活
- **状态追踪机制**：使用 atomic 标志 (`is_session_locked_`, `is_remote_session_`) 实时追踪系统状态
- **智能重建检测**：解锁时自动检测 `wallpaper_instances_` 是否为空，决定是普通恢复还是强制重建

#### 多显示器架构优化
- **持久化配置**：引入 `original_monitor_devices_` 保存用户最初的显示器配置（设备名称）
- **动态枚举**：重建前重新枚举当前会话的显示器，确保使用最新的显示器列表
- **自动映射**：将设备名称动态映射到当前索引，适配不同会话的显示器配置

### 🐛 Bug 修复

#### 会话切换修复（15个迭代修复）
1. ✅ **远程桌面壁纸消失**：允许远程会话运行壁纸，切换时强制重建
2. ✅ **锁屏后壁纸不恢复**：锁屏时暂停，解锁时检测是否需要重建
3. ✅ **WebView2 初始化失败**：避免连续两次 `StopWallpaper()` 导致 COM 对象冲突
4. ✅ **副显示器消失**：持久化保存所有显示器配置，重建时恢复所有显示器
5. ✅ **显示器索引混乱**：使用设备名称而非索引标识，解决枚举顺序不一致问题

#### 跨会话锁屏场景修复
- **远程桌面锁屏 → 主机解锁**：壁纸正确重建 ✓
- **主机锁屏 → 远程桌面解锁**：壁纸正确重建 ✓
- **不锁屏切换**：每次切换都能正确显示 ✓

#### 多显示器场景修复
- **显示器枚举顺序变化**：自动映射设备名称到当前索引 ✓
- **远程桌面显示器数量不同**：智能跳过不存在的显示器 ✓
- **主显示器和副显示器映射错误**：使用稳定的设备名称标识 ✓

### 🎯 使用示例

#### 跨会话使用（无需任何额外代码）

```dart
// 在主机上启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'file:///path/to/wallpaper.html',
  enableMouseTransparent: true,
);

// 切换到远程桌面 → 壁纸自动重建 ✓
// 锁屏 → 壁纸自动暂停 ✓
// 解锁 → 壁纸自动恢复或重建 ✓
// 切换回主机 → 壁纸自动重建 ✓

// 所有场景都自动处理，无需手动干预
```

#### 多显示器场景（自动适配）

```dart
// 在两个显示器上启动壁纸
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  enableMouseTransparent: true,
  monitorIndex: 0,  // 主显示器
);

await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  enableMouseTransparent: true,
  monitorIndex: 1,  // 副显示器
);

// 切换到远程桌面（可能只有1个显示器）
// → 插件自动跳过不存在的显示器1 ✓
// → 显示器0正常显示 ✓

// 切换回主机（2个显示器）
// → 插件自动恢复两个显示器 ✓
// → 即使枚举顺序改变也能正确映射 ✓
```

### 🔧 技术细节

#### 会话管理核心逻辑

**状态追踪**：
```cpp
std::atomic<bool> is_session_locked_;   // 锁屏状态
std::atomic<bool> is_remote_session_;   // 远程会话状态

bool ShouldWallpaperBeActive() {
  return !is_session_locked_.load();  // 只要不锁屏就激活
}
```

**事件处理**：
- `WTS_SESSION_LOCK` → 暂停壁纸
- `WTS_SESSION_UNLOCK` → 检测是否需要重建，然后恢复或重建
- `WTS_CONSOLE_CONNECT/DISCONNECT` → 强制重建（跨会话窗口不可见）
- `WTS_REMOTE_CONNECT/DISCONNECT` → 强制重建

#### 多显示器持久化

**保存配置**：
```cpp
// 使用设备名称而非索引
std::vector<std::string> original_monitor_devices_;  // ["\\.\DISPLAY1", "\\.\DISPLAY2"]
```

**恢复配置**：
```cpp
// 1. 重新枚举当前会话的显示器
GetMonitors();  // 更新 monitors_

// 2. 将设备名称映射到当前索引
for (const auto& device_name : original_monitor_devices_) {
  for (const auto& monitor : monitors_) {
    if (monitor.device_name == device_name) {
      saved_monitor_indices.push_back(monitor.index);
      break;
    }
  }
}

// 3. 只在存在的显示器上初始化
for (int monitor_index : saved_monitor_indices) {
  bool exists = /* 检查是否存在 */;
  if (exists) {
    InitializeWallpaperOnMonitor(..., monitor_index);
  } else {
    std::cout << "Skipping monitor (not available)" << std::endl;
  }
}
```

### 📚 文档更新

- 更新 `docs/FOR_FLUTTER_DEVELOPERS.md`：添加会话管理和多显示器最佳实践
- 更新 `docs/WEB_DEVELOPER_GUIDE_CN.md`：补充跨会话场景的注意事项
- 新增 `SESSION_LOGIC_ANALYSIS.md`：完整的会话切换逻辑分析

### 🧪 测试验证

#### 会话切换场景（全部通过）
- ✅ 主机锁屏 → 主机解锁（动画暂停再继续）
- ✅ 远程桌面锁屏 → 远程桌面解锁（动画暂停再继续）
- ✅ 主机锁屏 → 远程桌面解锁（壁纸重建）
- ✅ 远程桌面锁屏 → 主机解锁（壁纸重建）
- ✅ 主机不锁屏 → 远程桌面进入（壁纸重建）
- ✅ 远程桌面不锁屏 → 主机进入（壁纸重建）

#### 多显示器场景（全部通过）
- ✅ 主机2显示器 → 远程1显示器 → 主机（所有显示器恢复）
- ✅ 显示器枚举顺序改变（设备名称映射正确）
- ✅ 多次往返切换（无资源泄漏）

### ⚠️ 重要说明

#### 系统要求
- **Windows 10/11**：完整支持所有会话切换场景
- **WebView2 Runtime**：确保安装最新版本

#### 已知限制
- **远程桌面显示器数量**：如果远程桌面显示器数量少于主机，部分显示器会被跳过（正常行为）
- **DPI 缩放**：不同会话的 DPI 设置可能影响壁纸尺寸，但会自动适配

### 🎉 升级建议

从 v1.2.x 升级到 v1.3.0：
- ✅ **完全向后兼容**：无需修改现有代码
- ✅ **自动获益**：会话切换和多显示器功能自动生效
- ✅ **零配置**：所有优化都在插件层自动处理

---

## [4.5.0] - 2025-11-06 - 🔧 预编译包集成体验升级

### ✨ 新增功能

- Dart API 新增 `AnyWPEngine.getPluginVersion()` 与 `AnyWPEngine.isCompatible()`，便于在应用层检测插件版本

### 🔄 重构改进

- 预编译包结构标准化：`lib/anywp_engine.dart`、`lib/anywp_engine_plugin.lib`、`windows/anywp_sdk.js` 均位于标准位置
- 发布脚本支持同时打包 C++ 源码（`windows/src/`）以及 WebView2 NuGet 依赖，默认优先使用预编译 DLL，缺失时自动回退源码构建
- `CMakeLists.txt` 自动检测预编译/源码模式，兼容 Flutter 的标准插件构建流程

### 📚 文档与脚本

- 更新 `PRECOMPILED_README.md`，新增自动化安装、验证与示例运行说明
- 新增 `setup_precompiled.bat`、`verify_precompiled.bat`、`generate_pubspec_snippet.bat` 三个辅助脚本
- 预编译包包含最小可运行示例 `example_minimal/`

### 🔧 技术细节

- 发布脚本新增关键文件校验、模板渲染以及模板目录支持
- C++ 层改进 WebView2 初始化失败的提示信息，指导安装运行时
- 新增 `getVersion` 原生接口，返回插件版本号（v1.2.1）

### 🧪 验证

- 运行 `verify_precompiled.bat` 确认 8 个关键文件齐全
- 使用 `setup_precompiled.bat` 在全新 Flutter 工程内完成自动集成
- `example_minimal` 在 Windows 桌面环境验证壁纸启动/停止流程

## [4.4.1] - 2025-11-06 - 🔧 预编译包修复

### 🐛 Bug 修复

#### 预编译包完整性修复
- **修复缺少 .lib 文件**：添加 `anywp_engine_plugin.lib` 到发布包，解决链接器错误 `LNK1104`
- **修复 Dart 文件位置**：同时提供 `lib/anywp_engine.dart` 和 `lib/dart/anywp_engine.dart`（向后兼容）
- **改进构建脚本**：将 .lib 文件复制失败从警告改为错误，确保发布包完整

**修复的问题**：
- ❌ **v1.2.0 原问题**：缺少 `lib/anywp_engine_plugin.lib`，导致集成时链接失败
- ❌ **v1.2.0 原问题**：Dart 文件只在 `lib/dart/` 而非标准的 `lib/` 目录
- ✅ **v1.2.1 修复**：完整的预编译包结构

**完整的预编译包内容**（v1.2.1）：
```
anywp_engine_v1.2.0/
├── bin/
│   ├── anywp_engine_plugin.dll  ✅ 运行时 DLL
│   └── WebView2Loader.dll       ✅ WebView2 运行时
├── lib/
│   ├── anywp_engine_plugin.lib  ✅ 链接库（新增）
│   ├── anywp_engine.dart        ✅ Dart 源码（标准位置）
│   └── dart/
│       └── anywp_engine.dart    ✅ 向后兼容
├── include/                     ✅ C++ 头文件
├── sdk/anywp_sdk.js            ✅ JavaScript SDK
└── windows/CMakeLists.txt      ✅ CMake 配置
```

### 🔧 技术细节

**构建脚本改进**（`scripts/build_release_v2.bat`）：
- 第 96-103 行：强制要求 .lib 文件存在，否则中止构建
- 第 113-125 行：确保 Dart 文件同时复制到 `lib/` 和 `lib/dart/`

### 📦 升级说明

**如果您使用 v1.2.0 遇到链接错误**：
1. 删除旧的 `packages/anywp_engine_v1.2.0/`
2. 下载并解压新的 v1.2.0 包（实际版本 v1.2.1）
3. 重新运行 `flutter pub get`
4. 重新构建：`flutter build windows --release`

**验证修复**：
```powershell
# 检查关键文件是否存在
Test-Path "packages/anywp_engine_v1.2.0/lib/anywp_engine_plugin.lib"  # 应返回 True
Test-Path "packages/anywp_engine_v1.2.0/lib/anywp_engine.dart"        # 应返回 True
```

---

## [4.4.0] - 2025-11-06 - 🗂️ 应用级存储隔离 + 🎨 测试 UI 优化

### ✨ 新增功能

#### 应用级存储隔离机制
- **独立存储目录**：每个应用使用专属子目录 `%LOCALAPPDATA%\AnyWPEngine\[AppName]\`
- **无残留卸载**：卸载应用时可单独删除其数据目录，不影响其他应用
- **多应用隔离**：完美支持多个应用同时使用引擎，数据互不干扰
- **自动名称清理**：应用名称自动过滤非法字符，确保文件系统安全

**新增 API**：
```dart
// 设置应用唯一标识
await AnyWPEngine.setApplicationName('MyAwesomeApp');

// 获取存储路径
final path = await AnyWPEngine.getStoragePath();
```

#### 测试界面优化
- **8 个快捷测试按钮**：一键加载测试页面，效率提升 12 倍
- **表情图标标识**：直观识别测试页面类型
- **自动换行布局**：响应式设计，适配不同屏幕宽度
- **双入口设计**：快捷按钮 + 自定义 URL 输入框并存

**快捷测试页面**：
- 🎨 Simple - 基础壁纸测试
- 🖱️ Draggable - 拖拽演示（鼠标钩子）
- ⚙️ API Test - 完整 API 测试
- 👆 Click Test - 点击检测测试
- 👁️ Visibility - 可见性/省电测试
- ⚛️ React / 💚 Vue - SPA 框架测试
- 📺 iFrame Ads - 广告检测测试

### 🔄 重构改进

#### 存储系统升级
**v1.0**: 注册表存储 → 卸载残留  
**v1.1**: JSON 文件存储 → 多应用共享  
**v1.2**: 应用隔离存储 → ✅ 完美解决

**技术改进**：
- 修改 `GetAppDataPath()` 支持应用名称参数
- 更新 `LoadStateFile()` / `SaveStateFile()` 传递应用名称
- 添加应用名称清理和验证逻辑
- 切换应用时自动清空内存缓存

### 📚 文档更新

#### 文档优化
- **README.md**：整合存储隔离完整指南
  - 配置说明和 API 参考
  - 多应用隔离优势
  - 卸载清理最佳实践（多种方案）
  - 从旧版本迁移说明
- **开发者文档同步更新**：所有 API 参考和示例都已更新

### 🎯 使用示例

#### 设置应用隔离
```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 设置应用唯一标识（建议在初始化前调用）
  await AnyWPEngine.setApplicationName('MyCompany_MyApp');
  
  // 查看存储路径（可选）
  final path = await AnyWPEngine.getStoragePath();
  print('数据存储在: $path');
  
  runApp(MyApp());
}
```

#### 卸载清理
```bat
REM 在卸载脚本中清理应用数据
rmdir /s /q "%LOCALAPPDATA%\AnyWPEngine\MyApp"
```

### 💡 升级优势

#### 存储隔离
✅ 多应用完全隔离，互不干扰  
✅ 卸载干净无残留  
✅ 易于备份和迁移配置  
✅ 向后兼容（默认使用 "Default" 应用名）

#### 测试体验
✅ 点击快捷按钮即可加载测试页面  
✅ 无需记忆完整文件路径  
✅ 测试效率提升 12 倍（60秒 → 5秒）  
✅ 视觉友好的浅蓝色主题

### 🔧 技术细节

**存储路径**：
```
%LOCALAPPDATA%\AnyWPEngine\
├── AppA\
│   └── state.json    # 应用 A 的数据
├── AppB\
│   └── state.json    # 应用 B 的数据
└── Default\
    └── state.json    # 未设置应用名的默认数据
```

**测试按钮数据结构**：
```dart
final List<Map<String, String>> _testPages = [
  {'name': 'Simple', 'file': 'test_simple.html', 'icon': '🎨'},
  {'name': 'Draggable', 'file': 'test_draggable.html', 'icon': '🖱️'},
  // ... 更多测试页面
];
```

### 📊 测试结果

**功能测试**: ✅ 100% 通过 (17/17)  
**编译测试**: ✅ 无错误无警告  
**运行测试**: ✅ 稳定运行，内存占用合理  
**隔离测试**: ✅ 多应用数据完全隔离

### 🎉 发布亮点

1. **彻底解决数据残留问题** - 应用级存储隔离
2. **大幅提升测试效率** - 快捷测试按钮
3. **完善的文档支持** - 新增存储隔离指南
4. **向后兼容** - 旧代码无需修改即可运行

---

## [4.3.0] - 2025-11-05 - 📦 预编译 DLL 支持与发布流程

### ✨ 新增功能

#### 预编译 DLL 支持
- **快速集成**：提供预编译的 DLL 包，Flutter 开发者无需安装 WebView2 SDK
- **自动化构建**：新增 `build_release.bat` 脚本，一键生成 Release 包
- **完整打包**：包含 DLL、头文件、Dart 源码、JavaScript SDK
- **GitHub Release**：支持作为 GitHub Release 发布

#### 文档完善
- **集成指南**：新增 `PRECOMPILED_DLL_INTEGRATION.md` 详细说明预编译 DLL 使用方法
- **发布指南**：新增 `RELEASE_GUIDE.md` 说明如何构建和发布版本
- **Release 模板**：新增 `RELEASE_TEMPLATE.md` 作为 GitHub Release 说明模板
- **更新现有文档**：在 `PACKAGE_USAGE_GUIDE_CN.md` 和 `FOR_FLUTTER_DEVELOPERS.md` 中添加预编译 DLL 方式

### 🔨 构建工具

**新增脚本**：
```bash
.\scripts\build_release.bat  # 构建并打包 Release 版本
```

**生成产物**：
```
release/
└── anywp_engine_v1.1.0/
    ├── bin/              # DLL 文件
    ├── lib/              # 库文件和 Dart 源码
    ├── include/          # C++ 头文件
    ├── sdk/              # JavaScript SDK
    └── pubspec.yaml      # Flutter 包配置
```

### 📚 集成方式

现在支持四种集成方式（按推荐顺序）：

1. **预编译 DLL** ⭐（新增）
   - 无需编译，快速集成
   - 适合生产环境
   
2. **本地路径引用**
   - 适合开发测试
   
3. **Git 仓库引用**
   - 适合团队协作
   
4. **pub.dev 发布**
   - 适合公开发布

### 🎯 使用预编译 DLL

**下载**：
```
https://github.com/zhaibin/AnyWallpaper-Engine/releases
```

**集成**：
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.1.0
```

**详细文档**：
- [预编译 DLL 集成指南](docs/PRECOMPILED_DLL_INTEGRATION.md)
- [发布指南](docs/RELEASE_GUIDE.md)

---

## [4.2.0] - 2025-11-05 - 🎨 拖拽支持与状态持久化

### ✨ 核心功能

#### 拖拽支持
- **元素拖拽**：JavaScript SDK 新增 `makeDraggable()` 方法，支持任意元素拖拽
- **拖拽回调**：支持 `onDragStart`, `onDrag`, `onDragEnd` 回调函数
- **边界限制**：支持设置拖拽边界，防止元素超出可视区域
- **性能优化**：拖拽操作流畅无卡顿

#### 状态持久化
- **自动保存**：拖拽后的元素位置自动保存到 Windows Registry
- **自动恢复**：重新打开壁纸后自动恢复到上次的位置
- **通用存储**：支持保存任意键值对数据
- **跨会话**：状态在应用重启后依然保留

### 🆕 API 

**SDK 加载（HTML）**:
```html
<script src="../windows/anywp_sdk.js"></script>
```

**拖拽 (JavaScript)**:
```javascript
AnyWP.makeDraggable('#element', { persistKey: 'element_pos' });
AnyWP.resetPosition('#element', { left: 100, top: 100 });  // 复位
```

**状态 (Dart)**:
```dart
await AnyWPEngine.saveState('key', 'value');
String value = await AnyWPEngine.loadState('key');
```

## [4.1.0] - 2025-11-05 - 🚀 省电优化与即时恢复

### ✨ 核心改进

#### 轻量级暂停策略
- **即时恢复**：从 500-1000ms 优化到 **<50ms** ⚡
- **状态保留**：DOM 完全保留，无需重新加载
- **WebView2 优化**：使用 `put_IsVisible(FALSE)` 而非隐藏窗口
- **用户体验**：解锁后壁纸立即显示，仿佛从未暂停

#### 省电效果
- ✅ WebView2 完全停止渲染（节省 90% CPU/GPU）
- ✅ 自动暂停所有视频和音频
- ✅ 轻量内存管理（仅增加 10-20MB）
- ✅ Page Visibility API 集成

### 🆕 新增 API

#### JavaScript SDK (v4.1.0)
```javascript
// 监听可见性变化
AnyWP.onVisibilityChange(function(visible) {
  if (visible) {
    console.log('恢复 - 继续动画');
    resumeAnimations();
  } else {
    console.log('暂停 - 省电模式');
    pauseAnimations();
  }
});
```

**自动行为**：
- SDK 自动暂停所有 `<video>` 元素
- SDK 自动暂停所有 `<audio>` 元素
- 恢复时自动播放之前的媒体

#### Dart API
```dart
// 配置 API
await AnyWPEngine.setIdleTimeout(600);      // 设置空闲超时（秒）
await AnyWPEngine.setMemoryThreshold(200);  // 设置内存阈值（MB）
await AnyWPEngine.setCleanupInterval(30);   // 设置清理间隔（分钟）

// 获取配置
Map<String, dynamic> config = await AnyWPEngine.getConfiguration();

// 回调 API
AnyWPEngine.setOnPowerStateChangeCallback((old, newState) {
  print('状态变化: $old -> $newState');
});
```

### 📚 文档更新

#### 新增文档
- **[DEVELOPER_API_REFERENCE.md](docs/DEVELOPER_API_REFERENCE.md)** - 完整 API 参考
- **[API_USAGE_EXAMPLES.md](docs/API_USAGE_EXAMPLES.md)** - 7个实用示例
- **[BEST_PRACTICES.md](docs/BEST_PRACTICES.md)** - 最佳实践指南
- **[docs/README.md](docs/README.md)** - 文档索引

#### 新增示例
- `examples/test_visibility.html` - 可见性 API 测试页面

### 🎯 性能对比

| 指标 | v4.0.0 | v4.1.0 | 改进 |
|-----|--------|--------|------|
| 恢复速度 | 500-1000ms | **<50ms** | **20倍提升** ⚡ |
| 暂停后内存 | -50MB | -10MB | 更少清理开销 |
| 用户体验 | 明显延迟 | 几乎瞬间 | ✅ 优秀 |
| 省电效果 | 90% | 90% | 相同 |
| 状态保留 | 部分 | 完全 | ✅ 更好 |

### 💡 技术亮点

#### 智能暂停机制
```cpp
// 暂停：停止渲染但保留状态
webview_controller->put_IsVisible(FALSE);
NotifyWebContentVisibility(false);
SetProcessWorkingSetSize(...);  // 轻量 trim

// 恢复：瞬间恢复渲染
webview_controller->put_IsVisible(TRUE);
NotifyWebContentVisibility(true);
// 完成！<50ms
```

#### Page Visibility API 集成
- 发送标准 `visibilitychange` 事件
- 发送自定义 `AnyWP:visibility` 事件
- 网页可优雅处理暂停/恢复

### 🎨 用户体验提升

**解锁场景**：
1. 用户按 Win+L 锁屏
2. 壁纸立即停止渲染（省电 90%）
3. 状态完全保留在内存中
4. 用户解锁
5. **壁纸瞬间恢复**（<50ms）⚡
6. 视频从暂停处继续播放
7. 动画流畅过渡

**对比 v4.0.0**：
- ❌ 旧版：解锁后黑屏 → 等待加载 → 壁纸重新出现
- ✅ 新版：解锁后壁纸立即显示，完全无感知

### 🔄 向后兼容
- 完全兼容 v4.0.0 API
- 旧代码无需修改
- 自动享受性能提升
- `onVisibilityChange` 为可选 API

---

## [4.0.0] - 2025-11-03 - 🎉 重大更新：React/Vue SPA 完整支持

### ✨ 新增功能
- **SPA 框架自动检测**：自动识别 React、Vue、Angular 应用
- **智能路由监听**：自动监听 pushState/replaceState/popstate 事件
- **DOM 变化监听**：使用 MutationObserver 自动检测动态内容变化
- **元素自动重新挂载**：SPA 路由切换后自动重新绑定点击区域
- **ResizeObserver 集成**：自动跟踪元素尺寸和位置变化
- **等待元素出现**：`waitFor` 选项支持异步加载的动态元素
- **手动边界刷新**：提供 `refreshBounds()` API 手动刷新所有点击区域
- **处理器清理**：提供 `clearHandlers()` API 清理所有注册的点击处理器
- **React/Vue 生命周期辅助**：`useReactEffect()` 和 `vueLifecycle()` 辅助函数

### 🔧 API 改进
- `onClick()` 新增选项：
  - `immediate`: 立即注册（不延迟）
  - `waitFor`: 等待元素出现（默认 true）
  - `maxWait`: 最大等待时间（默认 10000ms）
  - `autoRefresh`: 自动刷新边界（默认 true）
  - `delay`: 自定义延迟时间（默认 100ms）
- 移除硬编码的 2000ms 延迟
- 支持自动检测并启用 SPA 模式
- 支持手动启用/禁用 SPA 模式：`setSPAMode(enabled)`

### 📚 文档
- 新增 **Web 开发者集成指南**（中英文）
  - `docs/WEB_DEVELOPER_GUIDE_CN.md`
  - `docs/WEB_DEVELOPER_GUIDE.md`
- 详细的 React 集成最佳实践
- 详细的 Vue 2/3 集成最佳实践
- SPA 路由处理指南
- 性能优化建议
- 调试技巧和常见问题解答

### 📝 示例
- 新增 `examples/test_react.html`：完整的 React 集成示例
  - Counter 组件
  - 可点击的卡片
  - 事件日志
  - SPA 模式展示
- 新增 `examples/test_vue.html`：完整的 Vue 3 集成示例
  - 多标签页切换
  - Todo List 应用
  - Counter 组件
  - 动态内容处理

### 🐛 修复
- 修复 SPA 路由切换后点击区域失效的问题
- 修复动态内容加载后点击区域不准确的问题
- 修复元素重新挂载后点击检测失败的问题
- 改进调试边框的更新和清理逻辑

### ⚡ 性能优化
- 使用 ResizeObserver 替代定时轮询
- 优化 DOM 变化监听，只在必要时刷新
- 路由切换后智能延迟刷新（500ms）
- 减少不必要的边界计算

### 🔄 向后兼容
- 完全向后兼容 v3.x API
- 旧代码无需修改即可继续工作
- 新功能默认启用，可选退出

---

## [3.1.3] - 2025-11-01 - 文档增强：打包与集成指南

### 📦 新增文档

#### 新增：完整的打包和使用指南

**新增文件**:
1. **[QUICK_INTEGRATION.md](QUICK_INTEGRATION.md)** - 30秒快速集成指南
   - 三种集成方式（本地路径、Git、pub.dev）
   - 完整代码示例
   - 常用场景速查

2. **[docs/PACKAGE_USAGE_GUIDE_CN.md](docs/PACKAGE_USAGE_GUIDE_CN.md)** - 详细打包使用文档
   - 三种集成方式详细对比
   - 本地路径引用完整说明
   - Git 仓库引用完整流程
   - pub.dev 发布详细步骤
   - 完整功能示例代码
   - API 参考文档
   - 故障排除指南

**更新文件**:
- **[README.md](README.md)** - 添加集成指南链接

### ✨ 主要内容

#### 1. 三种集成方式

| 方式 | 适用场景 | 配置方法 |
|------|---------|---------|
| **本地路径** | 开发测试 | `path: ../AnyWP_Engine` |
| **Git 仓库** | 团队协作 | `git: url + ref` |
| **pub.dev** | 生产环境 | `anywp_engine: ^1.0.0` |

#### 2. 完整使用示例

提供了以下示例代码：
- 最小化示例（10行代码启动壁纸）
- 完整功能示例（带UI控制器）
- 常用场景示例（透明、交互、本地文件）

#### 3. API 完整参考

- `initializeWallpaper()` - 详细参数说明
- `stopWallpaper()` - 清理和停止
- `navigateToUrl()` - 动态导航

#### 4. 故障排除

- WebView2 Runtime 安装
- 路径格式问题
- 权限问题
- 调试技巧

---

## [3.1.2] - 2025-11-01 - 安全增强与调试优化

### 🔒 安全增强

#### URL Blacklist System
- 新增 URL 黑名单功能，防止恶意网页访问敏感系统路径
- 默认封禁：`file:///c:/windows`, `file:///c:/program files` 及子目录

#### Security Logger
- 新增完整的安全日志系统：
  - Navigation attempts
  - Permission requests (camera/microphone/location)
  - Content security policy violations
- 所有安全事件自动记录到日志

### 🐛 Bug Fixes

#### Mouse Hook 稳定性
- 修复在大型监视器（3840x2160）上的鼠标钩子崩溃
- 修复偶发性内存泄漏和访问违例
- 完善线程安全处理

#### Resource Management
- 新增 ResourceTracker 统一管理所有窗口句柄
- 自动清理未释放的资源
- 改进析构函数清理逻辑

### 🔧 改进

#### Navigation Safety
- 所有 URL 导航前进行黑名单验证
- 拒绝访问的 URL 会写入日志并通知用户
- 支持子路径匹配（如 `/windows/system32`）

#### Display Change Monitoring
- 新增显示器变化监听器
- 支持分辨率变化、显示器添加/移除等事件
- Flutter 端可注册回调响应显示器变化

### 📊 性能

- 优化鼠标钩子性能（避免频繁坐标转换）
- 减少不必要的日志输出（非调试模式）
- 改进窗口创建和销毁流程

---

## [3.1.1] - 2025-10-31 - 最终优化：无缝壁纸体验

### ✨ 重大改进

#### 完美的桌面集成
- **智能 Z-Order 排序**：壁纸窗口自动定位到桌面图标正下方
  - 定位到 `SHELLDLL_DefView` 和 `WorkerW` 之间
  - 使用 `SWP_NOSENDCHANGING | SWP_NOACTIVATE` 避免闪烁
  - 自动重试机制确保成功

#### 鼠标点击穿透优化
- **智能点击区域管理**：
  - 桌面图标区域完全可点击（启用穿透）
  - 网页互动区域精确响应（禁用穿透）
  - 自动根据点击坐标动态切换
- **性能优化**：
  - 使用低级鼠标钩子（`WH_MOUSE_LL`）
  - 最小化性能开销
  - 仅在必要时调用 `SetWindowLongPtr`

#### 窗口管理改进
- **独立子窗口架构**：
  - WebView2 托管在独立子窗口中
  - 使用 `WS_CHILD | WS_VISIBLE` 样式
  - 支持多显示器（每个显示器独立子窗口）
- **生命周期管理**：
  - 完整的创建/销毁流程
  - 资源自动清理
  - 内存泄漏防护

### 🔧 技术实现

#### Mouse Hook System
```cpp
// 全局鼠标钩子
static HHOOK mouseHook_;
// 动态窗口透明度管理
LRESULT CALLBACK LowLevelMouseProc(...)
```

#### Z-Order Management
```cpp
// 智能定位算法
FindDesktopWindow()
SetWindowPos(..., HWND_BOTTOM - 1)
```

### 📝 API 不变
- 保持 Flutter API 完全兼容
- 无需修改现有代码
- 无缝升级体验

### 🐛 已知问题修复
- ✅ 修复桌面图标无法点击（完全解决）
- ✅ 修复窗口 Z-order 错误（完全解决）
- ✅ 修复资源泄漏问题
- ✅ 修复多显示器支持

---

## [3.1.0] - 2025-10-30 - 多显示器支持 + 完整 API

### 🎨 新增功能

#### 🖥️ 完整的多显示器支持
- **显示器枚举**：
  ```dart
  List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();
  ```
- **独立壁纸控制**：每个显示器可以显示不同的网页壁纸
- **显示器信息**：
  - 索引、名称（如 `\\.\DISPLAY1`）
  - 分辨率（宽度 x 高度）
  - 位置（x, y 坐标）
  - 是否为主显示器

#### 🔄 灵活的壁纸管理
- **多显示器模式**：
  ```dart
  // 在显示器 0 上启动壁纸
  await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com',
    monitorIndex: 0,
  );
  
  // 在显示器 1 上启动另一个壁纸
  await AnyWPEngine.initializeWallpaper(
    url: 'https://another.com',
    monitorIndex: 1,
  );
  ```

- **单一模式（向后兼容）**：
  ```dart
  // 不指定 monitorIndex，使用主显示器
  await AnyWPEngine.initializeWallpaper(
    url: 'https://example.com',
  );
```

#### 🌐 独立 URL 导航
- 每个显示器的壁纸可独立导航到不同 URL：
  ```dart
  await AnyWPEngine.navigateToUrl(
    url: 'https://newpage.com',
    monitorIndex: 0,
  );
  ```

### 🔧 API 改进

#### 新增方法
- `getMonitors()` → `List<MonitorInfo>`
  - 返回所有显示器信息
  - 包含分辨率、位置、是否为主显示器

#### 参数增强
所有主要方法新增 `monitorIndex` 可选参数：
- `initializeWallpaper(..., monitorIndex: int?)`
- `stopWallpaper(monitorIndex: int?)`
- `navigateToUrl(..., monitorIndex: int?)`

#### 新增数据模型
```dart
class MonitorInfo {
  final int index;
  final String name;
  final int width;
  final int height;
  final int x;
  final int y;
  final bool isPrimary;
}
```

### 📱 示例应用升级

#### 新增功能
- 显示器列表 UI
- 每个显示器独立控制面板
- 每个显示器独立 URL 输入框
- "Start All" / "Stop All" 批量操作
- 实时显示器状态指示

### 🐛 Bug Fixes
- 修复多显示器环境下坐标计算问题
- 修复显示器索引不匹配的问题
- 改进窗口定位算法

### 📚 文档更新
- 所有文档更新为多显示器 API
- 新增多显示器使用示例
- API 参考完全更新

---

## [3.0.0] - 2025-10-30 - API 重构 + 多显示器支持

### 🚀 重大变更

#### API 完全重构
**旧版 API (弃用)**:
```dart
// ❌ 已弃用
await AnyWPEngine.startWallpaper(url);
await AnyWPEngine.setMouseTransparent(true);
```

**新版 API**:
```dart
// ✅ 推荐使用
await AnyWPEngine.initializeWallpaper(
  url: url,
  isMouseTransparent: true,  // 默认 true
);
```

#### 命名一致性
- `startWallpaper` → `initializeWallpaper`
- `setMouseTransparent` → 合并到 `initializeWallpaper` 参数中
- 参数全部使用命名参数

### ✨ 新增功能

#### 1. 多显示器支持（初步）
- C++ 层新增显示器枚举 API
- 支持获取所有显示器信息
- 为每个显示器独立创建壁纸窗口

#### 2. 简化的初始化流程
```dart
// 一行代码启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  isMouseTransparent: true,
);
```

#### 3. 更好的错误处理
- 所有方法返回明确的错误信息
- 参数验证更严格

### 🔧 内部改进

#### C++ 架构优化
- 重构 Plugin 类，更清晰的方法命名
- 改进显示器管理逻辑
- 优化资源管理和清理

#### Flutter 层优化
- 简化 MethodChannel 调用
- 统一错误处理逻辑
- 改进日志输出

### 📚 文档更新
- 所有示例代码更新为新 API
- 添加 API 迁移指南
- 完善注释文档

### ⚠️ Breaking Changes
- 必须使用新的 `initializeWallpaper` API
- `setMouseTransparent` 不再作为独立方法
- 所有参数改为命名参数

### 🔄 向后兼容
- 旧 API 标记为 `@Deprecated` 但仍可使用
- 提供详细的迁移指南
- 建议在 v4.0.0 前完成迁移

---

## [2.0.0] - 2025-10-29 - 完整功能实现

### ✨ 核心功能

#### WebView2 集成
- 完整的 WebView2 Runtime 支持
- 异步初始化流程
- 环境复用机制

#### 桌面壁纸模式
- 窗口定位到桌面图标后方
- 支持鼠标穿透（桌面图标可点击）
- 支持交互模式（网页可交互）

#### JavaScript 桥接
- `anywp_sdk.js` 自动注入
- 点击事件处理
- URL 打开功能
- 鼠标/键盘事件支持

### 🎯 API

#### Dart API
```dart
await AnyWPEngine.startWallpaper(url);
await AnyWPEngine.stopWallpaper();
await AnyWPEngine.setMouseTransparent(true);
await AnyWPEngine.navigateToUrl(url);
```

#### JavaScript API
```javascript
AnyWP.ready('My Wallpaper');
AnyWP.onClick(element, callback);
AnyWP.openURL(url);
AnyWP.onMouse(callback);
AnyWP.onKeyboard(callback);
```

### 📱 示例应用
- 完整的 UI 控制面板
- URL 输入和导航
- 鼠标模式切换
- 常用场景按钮

### 📝 测试 HTML
- `test_simple.html` - 基础功能
- `test_api.html` - 完整 API 演示
- `test_iframe_ads.html` - 复杂场景测试

---

## [1.0.0] - 2025-10-28 - 初始发布

### 🎉 初始功能
- Windows 平台支持
- 基础的 WebView2 集成
- 简单的壁纸显示功能
- Flutter MethodChannel 通信
