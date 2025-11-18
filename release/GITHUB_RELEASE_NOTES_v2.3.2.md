# AnyWP Engine v2.3.2 - Release Notes

**发布日期**: 2025-11-18
**版本**: 2.3.2

---


## 🎯 日志系统现代化改造

### 问题背景
- **双重日志系统** - 混用 `std::cout` 和 `Logger::Instance()`，管理混乱
- **Release 构建噪音** - 控制台输出无法控制，Release 版本也有大量日志
- **日志过多** - 大量冗余日志（轮询、SDK 注入、初始化），影响可读性
- **国际化问题** - 代码中包含中文和 emoji，不符合国际化标准

### 解决方案

##### 📊 P3 级别日志优化（日志量减少 33.3%）

**1. 降低轮询日志级别为 DEBUG**
- `flutter_bridge.cpp`: 将 `getPendingPowerStateChanges`, `getPendingMessages`, `getMonitors` 等高频轮询方法的日志从 INFO 降为 DEBUG
- **效果**: 轮询日志从 22 行降为 0 行（Release 模式不显示）

**2. 合并重复的 SDK 注入日志**
- `sdk_bridge.cpp`: 全部迁移 `std::cout` 到 `Logger::Instance()`
- 简化 SDK 加载、注入、验证日志，避免重复输出
- 移除冗余的成功确认日志，仅保留关键错误日志
- **效果**: SDK 注入日志从 34 行降为 5 行（-85.3%）

**3. 精简 WebConsole 初始化日志**
- TypeScript SDK 源码优化（`windows/sdk/`）
- 移除 "Setting up WebMessage listener", "setup complete", "Events setup completed" 等冗余日志
- 保留 SDK 版本信息和重要警告
- **效果**: WebConsole 日志从 54 行降为 27 行（-50.0%）

**4. 模块初始化日志整合**
- 将每个模块独立的初始化日志合并为单行汇总
- 降级非关键的 "Module created", "Configuration updated" 日志为 DEBUG 级别
- **效果**: 模块初始化日志减少约 60%

**5. 移除 Emoji 和中文字符**
- TypeScript SDK: 移除所有 emoji（✅❌✨）和中文字符
- C++ 代码: 移除 emoji 和特殊符号（↔ → <->）
- Dart 应用: 移除 emoji
- **效果**: 100% 纯英文日志，符合国际化标准

##### 📈 日志系统统一（100% Logger 覆盖）

**完成日志迁移**:
- ✅ `anywp_engine_plugin.cpp`: 210/210 (100%)
- ✅ `initialization_coordinator.cpp`: 20/20 (100%)
- ✅ `window_manager.cpp`: 113/113 (100%)
- ✅ `sdk_bridge.cpp`: 全部迁移到 `Logger::Instance()`
- ✅ **总计**: 343 处 `std::cout` 全部迁移完成

**日志级别优化**:
- **DEBUG**: 高频操作、详细调试信息（仅 Debug 构建可见）
- **INFO**: 关键流程步骤、重要状态变化
- **WARN**: 非致命问题警告
- **ERROR**: 错误和异常

##### 🔧 技术改进

**智能日志分级**:
```cpp
// 轮询方法使用 DEBUG 级别
static const std::set<std::string> polling_methods = {
  "getPendingPowerStateChanges", "getPendingMessages", "getMonitors"
};
if (polling_methods.count(method_name) > 0) {
  Logger::Instance().Debug("FlutterBridge", "Method called: " + method_name);
}
```

**日志格式统一**:
- **控制台**: `[AnyWP] [COMPONENT] message`
- **文件**: `[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [COMPONENT] message`
- **输出模式**: Debug 构建 BOTH，Release 构建 FILE_ONLY

### 性能提升

| 指标 | 优化前 | 优化后 | 改善 |
|------|--------|--------|------|
| 总日志行数 | 327 | 211 | **-35.5%** |
| 轮询调用 | 22 | 0 | **-100%** |
| SDK 注入 | 34 | 5 | **-85.3%** |
| WebConsole | 54 | 27 | **-50.0%** |
| 中文/Emoji | 有 | 无 | **100% 纯英文** |

### 新增文档

- ✅ `docs/LOGGING_STANDARDS.md` - 日志规范和使用指南
- ✅ `docs/LOGGING_ANALYSIS.md` - 系统分析和问题诊断
- ✅ `docs/MIGRATION_PROGRESS.md` - 迁移进度跟踪
- ✅ `docs/SCRIPT_COMPARISON.md` - 迁移脚本对比
- ✅ `.cursorrules` - 更新编码规范，强制使用 Logger

### 质量保证

- ✅ 编译测试通过（Debug/Release）
- ✅ 功能正常（壁纸初始化、导航、交互）
- ✅ 日志量减少 35.5%
- ✅ 100% 纯英文输出
- ✅ 无编译错误或警告

---

