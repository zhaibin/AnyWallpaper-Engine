# AnyWP Engine v2.2.1 - Release Notes

**发布日期**: 2025-11-17
**版本**: 2.2.1

---


## 修复 (Fixed)

### 🔧 SDK 路径统一
- **修复 SDK 文件加载失败问题** - 统一 SDK 路径为 `sdk/dist/` 结构
- 解决其他项目集成时出现 "SDK file not found" 错误
- 更新所有 C++ 代码、CMake 配置和脚本以使用新路径
- 添加向后兼容路径 `sdk/anywp_sdk.js` 支持旧集成

### 📝 配置改进
- 更新 `.gitignore` 忽略 `node_modules/` 和 `package-lock.json`
- 优化构建脚本，明确 SDK 源码目录为 `sdk/src/`
- 改进发布脚本，从 `sdk/dist/` 复制 SDK 到发布包
- 更新验证脚本，检查新路径结构

## 技术细节 (Technical)

### C++ 代码修改
- `anywp_engine_plugin.cpp`: 更新 SDK 查找路径
- `sdk_bridge.cpp`: 添加向后兼容路径
- `webview_manager.cpp`: 更新 LoadSDKScriptFromFile 路径
- `startup_optimizer.cpp`: 更新 PreloadSDK 路径

### 构建配置修改
- `windows/CMakeLists.txt`: SDK 复制到 `sdk/dist/` 目录
- `windows/CMakeLists.precompiled.txt`: 预编译包使用新结构

### 脚本更新
- `scripts/build_sdk.bat`: 更新工作目录和输出说明
- `scripts/release.bat`: 从新路径复制 SDK 文件
- `scripts/test_full.bat`: 更新测试路径
- `scripts/verify_sdk.bat`: 验证新路径
- `scripts/verify_precompiled.bat`: 检查源码包结构

**影响**: 统一了开发、构建、发布流程中的 SDK 路径，解决了用户报告的 SDK 加载失败问题

