# AnyWP Engine v2.4.0 - Release Notes

**发布日期**: 2025-11-18
**版本**: 2.4.0

---


## ⚡ 简化集成：自动恢复 API（Auto Recovery）

### 问题背景

在 `v2.3.1` 中引入了 WorkerW 自动恢复功能，但需要开发者手动实现大约 20 行的监听和恢复代码：
- ❌ 必须监听 `WALLPAPER_RECREATE_REQUIRED` 消息
- ❌ 必须手动保存壁纸配置（URL、显示器索引）
- ❌ 必须手动停止和重建壁纸
- ❌ 必须处理延迟和清理逻辑
- ❌ 集成复杂，学习成本高

### 解决方案：`enableAutoRecovery` API

新增 **2 个 API**，将 20 行代码简化为 **2 行**：

```dart
// 1️⃣ 在 main() 中启用自动恢复（一次性设置）
await AnyWPEngine.enableAutoRecovery(true);

// 2️⃣ 正常初始化壁纸
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com',
  monitorIndex: 0,
);

// ✅ 完成！Explorer 重启时，插件会自动恢复壁纸
// ✅ 无需任何额外代码
```

### 新增 API

##### 1. `enableAutoRecovery(bool enabled)` ⭐
- **功能**: 启用或禁用自动恢复模式
- **默认值**: `false`（保持向后兼容）
- **自动保存**: 壁纸 URL、显示器索引、鼠标模式
- **自动恢复**: Explorer 重启、WorkerW 销毁、显示配置变化
- **智能延迟**: 插件自动处理系统稳定等待（1-2 秒）
- **多显示器**: 自动恢复所有显示器的壁纸
- **持久化**: 配置保存在内存中，应用重启后需重新启用

##### 2. `isAutoRecoveryEnabled()` 
- **功能**: 检查自动恢复是否启用
- **返回值**: `bool`

### 集成对比

**方案 A：自动恢复模式（推荐 - 99% 用户）**
```dart
// main() 中一次性设置
await AnyWPEngine.enableAutoRecovery(true);
```
- ✅ **极简集成** - 仅 1 行代码
- ✅ **零维护** - 插件自动保存和恢复
- ✅ **零学习成本** - 不需要理解底层机制

**方案 B：手动控制模式（高级用户）**
```dart
// 自定义恢复逻辑（例如：根据时间选择不同壁纸）
AnyWPEngine.setOnMessageCallback((message) {
  if (message['type'] == 'WALLPAPER_RECREATE_REQUIRED') {
    // 约 20 行自定义恢复代码
  }
});
```
- ⚠️ **仅适合** 需要自定义恢复逻辑的高级用户
- ⚠️ **代码量大** - 约 20 行代码
- ⚠️ **需手动管理** - 保存配置、处理延迟

### 技术实现

**C++ 层**：
- 新增 `auto_recovery_enabled_` 标志位
- 新增 `saved_wallpaper_configs_` 保存每个显示器的配置
- 在 `InitializeWallpaperOnMonitor` 成功时自动保存配置
- 在 `StopWallpaperOnMonitor` 成功时自动移除配置
- 在检测到 WorkerW 恢复时调用 `HandleAutoRecovery()`
- 自动延迟 1 秒等待系统稳定，然后依次恢复所有壁纸

**Dart 层**：
- 新增 `enableAutoRecovery(bool enabled)` API
- 新增 `isAutoRecoveryEnabled()` API
- 完整的文档注释和使用示例

### 文档更新

- ✅ `docs/FOR_FLUTTER_DEVELOPERS.md` - 新增自动恢复 vs 手动恢复对比
- ✅ `lib/anywp_engine.dart` - 新增 API 文档注释
- ✅ `example/lib/main.dart` - 更新示例代码使用自动恢复

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

