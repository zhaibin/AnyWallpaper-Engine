# AnyWP Engine 日志标准 v2.3.2

## 概述

从 v2.3.2 开始，项目统一使用 Logger 系统进行所有日志输出，**禁止使用 std::cout**。

## 日志格式

### 文件日志格式
```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [COMPONENT] message
```

**示例**:
```
[2025-11-18 16:45:23.456] [INFO] [Plugin] Plugin initialized
[2025-11-18 16:45:23.789] [ERROR] [WebViewManager] Failed to create WebView: Invalid parameter
```

### 控制台日志格式
```
[AnyWP] [COMPONENT] message
```

**示例**:
```
[AnyWP] [Plugin] Plugin initialized
[AnyWP] [WebViewManager] Failed to create WebView: Invalid parameter
```

## 输出模式

Logger 支持三种输出模式：

| 模式 | 说明 | 默认使用场景 |
|------|------|-------------|
| `FILE_ONLY` | 仅输出到文件 | Release 构建 |
| `CONSOLE_ONLY` | 仅输出到控制台 | 测试场景 |
| `BOTH` | 同时输出到文件和控制台 | Debug 构建 |

**配置方式**:
```cpp
// 设置为仅控制台输出（测试时使用）
Logger::Instance().SetOutputMode(Logger::OutputMode::CONSOLE_ONLY);

// 设置为两者都输出（开发时使用）
Logger::Instance().SetOutputMode(Logger::OutputMode::BOTH);
```

## 日志级别

### DEBUG
**用途**: 详细的诊断信息，帮助开发者追踪程序执行流程

**使用场景**:
- 函数入口/出口
- 循环迭代信息
- 详细的状态变量值
- 性能测量点

**示例**:
```cpp
Logger::Instance().Debug("WindowManager", "CreateWindow called with size: 1920x1080");
Logger::Instance().Debug("EventLoop", "Processing event #" + std::to_string(event_id));
```

**注意**: Debug 级别日志应该只在 Debug 构建中输出，Release 构建中自动过滤。

### INFO
**用途**: 正常的操作信息，记录系统的重要状态和操作

**使用场景**:
- 组件初始化/关闭
- 重要配置加载
- 状态转换
- 用户操作记录

**示例**:
```cpp
Logger::Instance().Info("Plugin", "Plugin initialized successfully");
Logger::Instance().Info("WebViewManager", "WebView created on monitor 0");
Logger::Instance().Info("PowerManager", "Power state changed: ACTIVE -> IDLE");
```

### WARNING
**用途**: 警告信息，表示潜在问题但程序可以继续运行

**使用场景**:
- 配置缺失使用默认值
- 资源接近限制
- 可恢复的错误
- 弃用功能使用

**示例**:
```cpp
Logger::Instance().Warning("ConfigManager", "Config file not found, using defaults");
Logger::Instance().Warning("MemoryOptimizer", "Memory usage reached 80% threshold");
Logger::Instance().Warning("Plugin", "is_initialized_ flag is deprecated, use GetActiveInstanceCount()");
```

### ERROR
**用途**: 错误信息，表示操作失败或异常情况

**使用场景**:
- API 调用失败
- 资源分配失败
- 异常捕获
- 关键操作失败

**示例**:
```cpp
Logger::Instance().Error("WebViewManager", "Failed to create WebView: " + error_message);
Logger::Instance().Error("MouseHookManager", "Hook installation failed: Access denied");
Logger::Instance().Error("Plugin", "WorkerW window not found after 10 retries");
```

## 组件命名规范

### PascalCase 命名
所有组件名称使用 PascalCase（每个单词首字母大写）。

**正确示例**:
```cpp
Logger::Instance().Info("Plugin", "Message");
Logger::Instance().Info("WebViewManager", "Message");
Logger::Instance().Info("MouseHookManager", "Message");
Logger::Instance().Info("InitCoordinator", "Message");
Logger::Instance().Info("FlutterBridge", "Message");
```

**错误示例**:
```cpp
Logger::Instance().Info("plugin", "Message");           // ❌ 全小写
Logger::Instance().Info("webview_manager", "Message");  // ❌ snake_case
Logger::Instance().Info("PLUGIN", "Message");           // ❌ 全大写
```

### 常用组件名称

| 组件名 | 说明 |
|--------|------|
| `Plugin` | 主插件类 |
| `WebViewManager` | WebView 管理器 |
| `MouseHookManager` | 鼠标钩子管理器 |
| `MonitorManager` | 显示器管理器 |
| `PowerManager` | 电源管理器 |
| `FlutterBridge` | Flutter 桥接 |
| `SDKBridge` | SDK 桥接 |
| `WindowManager` | 窗口管理器 |
| `InstanceManager` | 实例管理器 |
| `InitCoordinator` | 初始化协调器 |
| `EventDispatcher` | 事件分发器 |
| `MemoryOptimizer` | 内存优化器 |
| `WorkerWHealthMonitor` | WorkerW 健康监控 |
| `IframeDetector` | iframe 检测器 |
| `CustomSchemeHandler` | 自定义协议处理器 |
| `Lifecycle` | 生命周期管理 |
| `Refactor` | 重构相关 |

## 消息格式规范

### 基本原则
1. **英文消息**: 所有日志消息使用英文
2. **无表情符号**: 不使用 emoji 或特殊符号
3. **清晰简洁**: 消息要清晰表达意图，避免冗长
4. **包含上下文**: 提供足够的上下文信息（参数值、状态等）

### 消息模板

#### 操作开始
```cpp
Logger::Instance().Info("Module", "Operation starting...");
Logger::Instance().Info("Module", "Initializing component with URL: " + url);
```

#### 操作成功
```cpp
Logger::Instance().Info("Module", "Operation completed successfully");
Logger::Instance().Info("Module", "Component initialized successfully");
```

#### 操作失败
```cpp
Logger::Instance().Error("Module", "Operation failed: " + error_reason);
Logger::Instance().Error("Module", "Failed to initialize: Invalid parameter");
```

#### 状态变更
```cpp
Logger::Instance().Info("Module", "State changed: " + old_state + " -> " + new_state);
Logger::Instance().Info("PowerManager", "Power state changed: ACTIVE -> IDLE");
```

#### 资源操作
```cpp
Logger::Instance().Info("Module", "Resource allocated: " + resource_name);
Logger::Instance().Info("Module", "Resource released: " + resource_name);
Logger::Instance().Warning("Module", "Resource limit reached: " + std::to_string(count));
```

## 特殊格式方法

### Banner - 重要标题
用于标记重要的操作阶段或分隔不同部分。

```cpp
Logger::Instance().Banner("InitCoordinator", "Initialization Start");
```

**输出** (控制台):
```
[AnyWP] [InitCoordinator] ============================================================
[AnyWP] [InitCoordinator] Initialization Start
[AnyWP] [InitCoordinator] ============================================================
```

### Section - 小节标题
用于标记相关操作的小节。

```cpp
Logger::Instance().Section("Plugin", "WebView Setup");
```

**输出** (控制台):
```
[AnyWP] [Plugin] ---------------------------------------- WebView Setup ----------------------------------------
```

## 使用示例

### 基本使用
```cpp
// 替换 std::cout
// ❌ 旧代码
std::cout << "[AnyWP] Plugin initialized" << std::endl;

// ✅ 新代码
Logger::Instance().Info("Plugin", "Plugin initialized");
```

### 带变量的消息
```cpp
// ❌ 旧代码
std::cout << "[AnyWP] [InitCoordinator] URL: " << url << std::endl;
std::cout << "[AnyWP] Monitor: " << monitor_index << std::endl;

// ✅ 新代码
Logger::Instance().Info("InitCoordinator", "URL: " + url);
Logger::Instance().Info("InitCoordinator", "Monitor: " + std::to_string(monitor_index));
```

### 多行信息
```cpp
// ❌ 旧代码
std::cout << "[AnyWP] ========== Initialization Start ==========" << std::endl;
std::cout << "[AnyWP] URL: " << url << std::endl;
std::cout << "[AnyWP] Transparent: " << (transparent ? "true" : "false") << std::endl;

// ✅ 新代码
Logger::Instance().Banner("InitCoordinator", "Initialization Start");
Logger::Instance().Info("InitCoordinator", "URL: " + url);
Logger::Instance().Info("InitCoordinator", "Transparent: " + std::string(transparent ? "true" : "false"));
```

### 条件日志
```cpp
// ✅ Debug 级别（仅 Debug 构建输出）
#ifdef _DEBUG
  Logger::Instance().Debug("EventLoop", "Processing event #" + std::to_string(event_id));
#endif

// 或使用日志级别过滤
Logger::Instance().SetMinLevel(Logger::Level::INFO);  // Release 时设置为 INFO
Logger::Instance().Debug("Module", "This will not appear in Release");  // 自动过滤
```

### 错误处理
```cpp
try {
  // 操作
  Logger::Instance().Info("Module", "Operation started");
  
  // ... 执行操作 ...
  
  Logger::Instance().Info("Module", "Operation completed");
} catch (const std::exception& e) {
  Logger::Instance().Error("Module", "Operation failed: " + std::string(e.what()));
}
```

## 便利宏（可选）

为了简化代码，可以使用预定义的宏：

```cpp
// 使用完整方法（推荐）
Logger::Instance().Info("Plugin", "Message");

// 使用便利宏（可选）
ANYWP_LOG_INFO("Plugin", "Message");
ANYWP_LOG_DEBUG("Plugin", "Debug message");
ANYWP_LOG_WARNING("Plugin", "Warning message");
ANYWP_LOG_ERROR("Plugin", "Error message");
```

## 性能考虑

### 字符串拼接优化
对于复杂的字符串拼接，考虑使用 `std::ostringstream`：

```cpp
// ❌ 低效（多次字符串拼接）
std::string msg = "Window created: " + std::to_string(width) + "x" + std::to_string(height) + 
                  " at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
Logger::Instance().Info("WindowManager", msg);

// ✅ 高效（使用 ostringstream）
std::ostringstream oss;
oss << "Window created: " << width << "x" << height << " at (" << x << ", " << y << ")";
Logger::Instance().Info("WindowManager", oss.str());
```

### 条件日志
对于开销较大的日志构建，使用条件检查：

```cpp
// ✅ 仅在需要时构建日志消息
if (Logger::Instance().GetMinLevel() <= Logger::Level::DEBUG) {
  std::ostringstream oss;
  // 复杂的日志构建逻辑
  oss << "Detailed state: " << /* 大量数据 */;
  Logger::Instance().Debug("Module", oss.str());
}
```

## 迁移清单

### 迁移步骤

1. **识别 std::cout**
   ```bash
   grep -r "std::cout" windows/
   ```

2. **替换为 Logger**
   - 简单消息：直接替换
   - 带变量：使用字符串拼接
   - 多行：使用多个 Logger 调用
   - Banner：使用 `Banner()` 方法

3. **选择合适的日志级别**
   - 调试信息 → Debug
   - 状态变更 → Info
   - 警告信息 → Warning
   - 错误信息 → Error

4. **验证输出**
   - Debug 模式：控制台应有输出
   - Release 模式：控制台应无输出（仅文件）

### 常见模式替换

| 旧代码 | 新代码 |
|--------|--------|
| `std::cout << "[AnyWP] Message" << std::endl;` | `Logger::Instance().Info("Module", "Message");` |
| `std::cout << "[AnyWP] [Module] Message" << std::endl;` | `Logger::Instance().Info("Module", "Message");` |
| `std::cout << "[AnyWP] ====== Title ======" << std::endl;` | `Logger::Instance().Banner("Module", "Title");` |
| `std::cout << "[AnyWP] Value: " << value << std::endl;` | `Logger::Instance().Info("Module", "Value: " + std::to_string(value));` |

## 配置建议

### Debug 构建
```cpp
Logger::Instance().SetOutputMode(Logger::OutputMode::BOTH);
Logger::Instance().SetMinLevel(Logger::Level::DEBUG);
Logger::Instance().EnableConsoleLogging(true);
Logger::Instance().EnableFileLogging("debug.log");
```

### Release 构建
```cpp
Logger::Instance().SetOutputMode(Logger::OutputMode::FILE_ONLY);
Logger::Instance().SetMinLevel(Logger::Level::INFO);
Logger::Instance().EnableConsoleLogging(false);  // 禁用控制台
Logger::Instance().EnableFileLogging("anywp_engine.log");
```

## 注意事项

### ⚠️ 禁止事项
1. ❌ 禁止使用 `std::cout` 进行日志输出
2. ❌ 禁止在日志消息中使用 emoji
3. ❌ 禁止记录敏感信息（密码、密钥等）
4. ❌ 禁止在循环中大量输出 Debug 日志

### ✅ 最佳实践
1. ✅ 统一使用 Logger 系统
2. ✅ 选择合适的日志级别
3. ✅ 提供有意义的上下文信息
4. ✅ 保持消息简洁明了
5. ✅ 使用 PascalCase 组件名
6. ✅ 关键操作要有开始/完成日志

## 相关文档

- `windows/utils/logger.h` - Logger 类定义
- `windows/utils/logger.cpp` - Logger 实现
- `docs/LOGGING_ANALYSIS.md` - 日志系统分析报告
- `.cursorrules` - 编码规范

---

**版本**: v2.3.2  
**最后更新**: 2025-11-18  
**状态**: 正式规范

