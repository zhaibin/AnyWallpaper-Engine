# AnyWP Engine v2.1.9 - Release Notes

**发布日期**: 2025-11-15
**版本**: 2.1.9

---

## 🔧 SDK 路径修复 + Minified 版本支持


## ✨ 功能增强

### SDK 加载逻辑优化
- **改进内容**:
  - C++ 代码现在优先查找并使用 `anywp_sdk.min.js`（minified 版本）
  - 自动从多个路径查找 SDK 文件，包括预编译包结构
  - 增强了对预编译包集成的支持
- **技术细节**:
  - `windows/modules/sdk_bridge.cpp`: 优先加载 minified 版本
  - `windows/anywp_engine_plugin.cpp`: 同步优化 SDK 加载路径
  - 支持从 DLL 所在目录的多个相对路径查找 SDK

### Minified SDK 构建支持
- **构建脚本**:
  - `scripts/build_sdk.bat`: 新增 `production` 模式，生成 minified 和 unminified 两个版本
  - `scripts/release.bat`: 发版时自动生成 minified SDK 并复制到所有发布包
- **发版流程**:
  - 预编译包：包含 `anywp_sdk.min.js`（优先）和 `anywp_sdk.js`（备用）
  - 源码包：包含两个版本的 SDK 文件
  - Web SDK 包：同时包含 minified 和 unminified 版本

## 🐛 问题修复

### 修复预编译包集成时的 SDK 找不到问题
- **问题**: 开发者使用预编译包集成后，日志出现 "没有找到anywp_sdk.js"
- **原因**: 预编译包的 SDK 文件路径与 C++ 代码的查找路径不匹配
- **修复**:
  - C++ 代码增强，支持从 DLL 所在目录的多个相对路径查找
  - 支持预编译包结构：`../sdk/anywp_sdk.min.js`、`../../sdk/anywp_sdk.min.js` 等
  - 优先使用 minified 版本，提供更好的性能

### 版本号同步
- **改进**: JS SDK 版本号与 Flutter 插件版本号保持同步（均为 2.1.9）
- **发版脚本**: `release.bat` 现在区分并显示两个版本号
  - Flutter 插件版本：从 `pubspec.yaml` 读取
  - JS SDK 版本：从 `windows/sdk/package.json` 读取
- **Web SDK 包**: 使用 JS SDK 版本号命名：`anywp_web_sdk_v{SDK_VERSION}.zip`

## 📝 文档更新

- `docs/PRECOMPILED_DLL_INTEGRATION.md`: 添加 "SDK file not found" 常见问题解答
- 提供多种解决方案：自动查找、手动复制、CMakeLists.txt 自动复制

## 🔄 技术改进

- **性能优化**: 优先使用 minified SDK，减少运行时脚本大小
- **兼容性**: 保留 unminified 版本作为备用，确保所有场景都能正常工作
- **开发体验**: 开发环境使用 unminified 版本，便于调试；生产环境自动使用 minified 版本

---

