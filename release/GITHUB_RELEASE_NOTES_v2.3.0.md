# AnyWP Engine v2.3.0 - Release Notes

**发布日期**: 2025-11-17
**版本**: 2.3.0

---


## 🎯 重大改进 (Breaking Changes)

### 嵌入式 SDK - 彻底解决路径问题
- **SDK 嵌入 DLL** - 使用 Windows RC 资源将 SDK 编译到 `anywp_engine_plugin.dll` 中
- **零外部依赖** - 运行时无需任何外部 SDK 文件
- **自动加载** - SDK 从 DLL 资源自动提取并注入到 WebView
- **向后兼容** - 保留文件系统 fallback，支持旧版本集成

### 版本号统一管理 - 避免版本不一致
- **单一数据源** - 版本号仅在 `pubspec.yaml` 中定义
- **自动生成** - CMake 自动从 `pubspec.yaml` 生成 `windows/version.h`
- **零手动维护** - C++ 代码中的版本常量自动同步
- **一致性保证** - Plugin 版本、SDK 版本、Package 版本完全一致

## 新增功能 (Added)

### 🔨 新增模块
- `windows/sdk_loader.h/cpp` - 统一 SDK 加载器（支持嵌入资源和文件 fallback）
- `windows/sdk_resource.h` - Windows RC 资源定义
- `windows/sdk_resource.rc` - RC 资源脚本（嵌入 SDK）
- `windows/version.h.in` - 版本号模板（CMake 自动生成 `version.h`）

## 改进 (Improved)

### 📦 部署简化
- **减少部署文件** - 无需复制 `sdk/` 目录到部署包
- **DLL 自包含** - SDK 包含在 DLL 中，约增加 80KB
- **构建简化** - 移除所有 SDK 复制逻辑，CMake 自动处理

### ⚡ 性能优化
- **加载速度提升** - 从内存加载 SDK 比文件 I/O 更快
- **缓存机制** - SDK 仅加载一次，后续使用缓存
- **启动优化** - 减少文件系统访问

### 🔒 安全性增强
- **防篡改** - 嵌入的 SDK 无法被用户修改
- **完整性保证** - SDK 与 DLL 一起分发，确保版本一致

## 技术细节 (Technical)

### C++ 代码修改
- `sdk_loader.cpp`: 新增，实现从 DLL 资源加载 SDK
  - `LoadSDKFromResource()` - 从嵌入资源加载（优先）
  - `LoadSDKFromFile()` - 从文件加载（fallback）
  - `LoadSDKScript()` - 智能加载（自动选择最佳方式）
- `sdk_bridge.cpp`: 使用新 SDK 加载器
- `webview_manager.cpp`: 移除旧的 SDK 文件查找逻辑
- `anywp_engine_plugin.cpp`: 简化 SDK 加载，调用统一接口

### 构建配置修改
- `windows/CMakeLists.txt`:
  - 添加 `RC` 语言支持（Windows Resource Compiler）
  - 添加 `sdk_loader.cpp` 和 `sdk_resource.rc` 到源文件列表
  - 移除所有 SDK 文件复制命令（不再需要）
  - 添加 SDK 文件存在性验证（构建时检查）
  - **新增版本号自动生成** - 从 `pubspec.yaml` 读取版本并生成 `version.h`
- `windows/CMakeLists.precompiled.txt`:
  - 更新预编译模式说明（SDK 已嵌入）
  - 移除 `anywp_copy_sdk` 函数（不再需要）
  - 添加向后兼容的占位函数（提示用户）

### 发布脚本更新
- 预编译包不再包含 `sdk/` 目录（SDK 已在 DLL 中）
- 源码包仍包含 `sdk/` 目录（供编译使用）

## 迁移指南 (Migration)

### 对于开发者
- **无需修改代码** - SDK 加载逻辑完全向后兼容
- **可移除 SDK 复制代码** - 如果手动复制 SDK，现在可以删除
- **简化部署** - 只需部署 DLL，无需额外 SDK 文件

### 对于预编译包用户
- **无需修改** - SDK 已嵌入 DLL，直接使用即可
- **可删除旧 SDK 文件** - 如果有手动复制的 SDK 文件，可以删除

### 从 v2.2.x 升级
1. 清理旧版本：`flutter clean`
2. 获取新版本：`flutter pub get`
3. 重新构建：`flutter build windows`
4. 删除手动复制的 SDK 文件（如果有）
5. **版本号管理变更**（对开发者）：
   - 如果您修改了 `windows/anywp_engine_plugin.cpp` 中的版本号，请删除这些修改
   - 版本号现在由 CMake 从 `pubspec.yaml` 自动生成到 `windows/version.h`
   - `windows/version.h` 会被自动生成，已添加到 `.gitignore`

**影响**: 彻底解决了 SDK 文件路径依赖问题和版本号不一致问题，大幅简化部署和发版流程，提升性能和安全性

---

