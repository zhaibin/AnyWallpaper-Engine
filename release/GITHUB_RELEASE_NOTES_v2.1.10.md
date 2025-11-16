# AnyWP Engine v2.1.10 - Release Notes

**发布日期**: 2025-11-16
**版本**: 2.1.10

---


## 新增功能 (Added)

### 🔢 版本号 API
- 新增 `AnyWPEngine.getSDKVersion()` API，用于获取内置 Web SDK 版本号
- 优化 `AnyWPEngine.getPluginVersion()` 文档说明
- 支持在 Dart 和 JavaScript 中查询版本信息

### 🔐 自定义协议与文件加密
- 新增自定义 URL 协议 `anywp://file?path=<path>` 支持
- 新增 `AnyWPEngine.encryptFile(sourcePath, destPath)` API（Dart + JavaScript）
- 新增 `AnyWPEngine.decryptFile(encryptedPath, destPath)` API（Dart + JavaScript）
- 实现零拷贝内容交付：协议自动解密文件
- 新增 MIME 类型自动检测（支持图片/视频格式）
- 开发者可自定义缓存路径，引擎不管理固定目录

### 📖 文档完善
- 新增 `docs/VERSION_MANAGEMENT.md` - 版本号统一管理指南
- 更新 `docs/DEVELOPER_API_REFERENCE.md` - 添加版本号和加密 API 文档
- 更新 `docs/WEB_DEVELOPER_GUIDE_CN.md` - 添加自定义协议完整指南
- 更新 `docs/FOR_FLUTTER_DEVELOPERS.md` - 更新 API 列表

## 修复 (Fixed)

### 🖱️ 交互式壁纸点击支持
- 修复测试页面在交互式壁纸中无法点击的问题
- 添加 `AnyWP.onClick()` 点击处理器自动注册
- 优化交互式壁纸测试页面用户体验

### 📬 消息处理链路
- 修复 `encryptFile`/`decryptFile` 消息未被正确处理的问题
- 完善 Dart 层消息处理器（`main.dart` 中添加 encryptFile/decryptFile 处理）
- 优化 JavaScript SDK 使用 `sendToFlutter` 发送加密请求

### 🔧 版本号统一性
- 修复 `windows/sdk/package.json` 版本号不一致（2.1.9 → 2.1.10）
- 统一 5 个关键位置的版本号定义
- 添加版本号检查脚本和文档

## 技术改进 (Technical)

### 🏗️ 架构优化
- 新增 `CustomSchemeHandler` 模块处理自定义协议
- 新增 `MimeTypeDetector` 工具类支持文件类型检测
- 完善 `FlutterBridge` 添加 encryptFile/decryptFile 处理器
- TypeScript SDK 新增 `File` 模块（`windows/sdk/modules/file.ts`）

### 🔒 安全性增强
- 路径验证：强制绝对路径，禁止 `..` 遍历
- 输入验证：检查文件路径有效性和安全性
- 错误响应：返回标准 HTTP 状态码（404/403/500）

### 📊 测试改进
- 新增交互式壁纸测试页面（`examples/test_anywp_protocol_interactive.html`）
- 新增自定义协议测试脚本（`scripts/test_anywp_protocol.bat`）
- 优化测试流程和日志输出

---

