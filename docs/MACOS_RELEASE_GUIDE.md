# 📦 AnyWP Engine - macOS 发布指南

本文档说明如何为 macOS 平台构建和发布 AnyWP Engine 预编译包。

---

## 🎯 前提条件

- macOS 10.14 或更高版本
- Flutter 3.0+
- Xcode 12+ (Command Line Tools)
- Node.js 和 npm（用于构建 SDK）

---

## 📋 发布流程

### Step 1: 更新版本号

确保以下文件中的版本号一致：

- [ ] `pubspec.yaml`: `version: 2.2.0`
- [ ] `macos/anywp_engine.podspec`: `s.version = '2.2.0'`
- [ ] `CHANGELOG_CN.md`: 添加新版本条目
- [ ] `.cursorrules`: 更新底部版本号

### Step 2: 更新文档

**新增 API 必须更新**:
- [ ] `lib/anywp_engine.dart` - 文档注释
- [ ] `README.md` - 示例代码
- [ ] `docs/DEVELOPER_API_REFERENCE.md` - API 章节
- [ ] `docs/FOR_FLUTTER_DEVELOPERS.md` - API 列表

**新增功能必须更新**:
- [ ] `README.md` - Features 章节
- [ ] `CHANGELOG_CN.md` - 更新日志条目
- [ ] `docs/MACOS_DEVELOPER_GUIDE.md` - macOS 特定功能

### Step 3: 构建和测试

```bash
cd example
flutter clean
flutter pub get
flutter build macos --debug
flutter build macos --release
```

**测试清单**:
- [ ] Debug/Release 构建成功
- [ ] 无 Linter 警告：`flutter analyze`
- [ ] 功能测试通过（至少 2 个测试页面）
- [ ] 验证 plugin framework 已生成
- [ ] 多显示器支持测试
- [ ] 电源管理功能测试

### Step 4: 构建发布包

```bash
# 在项目根目录执行
./scripts/release_macos.sh
```

**构建产物**（在 `release/` 目录）:
1. `anywp_engine_macos_v2.2.0_precompiled.zip` - 预编译包
2. `anywp_engine_macos_v2.2.0_source.zip` - 源码包
3. `anywp_web_sdk_v2.2.0.zip` - Web SDK 包（可选，如果未构建）

### Step 5: 验证发布包

```bash
./scripts/verify_precompiled_macos.sh 2.2.0
```

**验证清单**:
- [ ] 预编译包包含所有必要文件
- [ ] SDK 文件完整
- [ ] 示例文件齐全
- [ ] 文档完整
- [ ] Podspec 文件正确

### Step 6: 创建 GitHub Release

1. 访问: https://github.com/zhaibin/AnyWallpaper-Engine/releases/new
2. Tag: `v2.2.0-macos` 或 `v2.2.0` (如果同时发布 Windows)
3. Title: `AnyWP Engine v2.2.0 - macOS Support`
4. Description: 参考 Release Notes 模板（见下文）
5. 上传 ZIP 文件：
   - `anywp_engine_macos_v2.2.0_precompiled.zip` - **推荐给 Flutter 开发者**
   - `anywp_engine_macos_v2.2.0_source.zip` - 完整源码（高级用户）
   - `anywp_web_sdk_v2.2.0.zip` - Web 壁纸开发者（可选）
6. 勾选 "Set as pre-release" (如果是 Beta 版本)

---

## 📝 Release Notes 模板

```markdown
# AnyWP Engine v2.2.0 - macOS Support

**发布日期**: 2025-11-17  
**版本**: 2.2.0  
**平台**: macOS 10.14+

## 🎉 重大更新

### 🍎 macOS 平台支持

这是 AnyWP Engine 首次正式支持 macOS 平台！

**核心功能**:
- ✅ 桌面壁纸嵌入（使用 WKWebView）
- ✅ 多显示器支持
- ✅ 电源管理和自动优化
- ✅ 双向通信（JavaScript ↔ Native）
- ✅ 状态持久化
- ✅ 应用级存储隔离

**技术实现**:
- 使用 WKWebView 替代 WebView2
- 完整的 Objective-C 实现
- CocoaPods 集成
- 跨平台 Web SDK

## 📦 下载

**macOS 预编译包**（推荐）:
- `anywp_engine_macos_v2.2.0_precompiled.zip` (约 0.5MB)
- ✅ 无需 Xcode 编译
- ✅ CocoaPods 自动管理依赖
- ✅ 快速集成

**macOS 源码包**（高级用户）:
- `anywp_engine_macos_v2.2.0_source.zip` (约 2MB)
- ✅ 完整 Objective-C 源码
- ✅ 所有模块和工具类
- ✅ 可自定义修改

**Web SDK**（跨平台）:
- `anywp_web_sdk_v2.2.0.zip`
- ✅ 支持 Windows 和 macOS
- ✅ TypeScript 类型定义
- ✅ 完整示例

## 📚 文档

- [macOS 预编译包集成指南](docs/PRECOMPILED_MACOS_INTEGRATION.md)
- [macOS 开发者指南](docs/MACOS_DEVELOPER_GUIDE.md)
- [多平台架构设计](docs/MULTIPLATFORM_ARCHITECTURE.md)
- [API 参考文档](docs/DEVELOPER_API_REFERENCE.md)

## 🔧 系统要求

- macOS 10.14 或更高版本
- Flutter 3.0+
- Xcode 12+ (仅需 Command Line Tools)

## ⚠️ 已知限制

- ❌ 交互模式（Interactive Mode）未实现
- ❌ 文件加密/解密功能待实现
- ⚠️ WKWebView 内存使用略高于 Windows WebView2

计划在 v2.2.1 实现上述功能。

## 🐛 Bug 修复

（从 CHANGELOG_CN.md 复制相关内容）

## 🙏 致谢

感谢社区贡献者的反馈和测试！

---

**完整更新日志**: [CHANGELOG_CN.md](CHANGELOG_CN.md)
```

---

## 🔍 发布包内容验证

### 预编译包

必须包含以下文件：
```
anywp_engine_macos_v2.2.0_precompiled/
├── Frameworks/          (可选，通过 CocoaPods 获取)
├── lib/
│   ├── anywp_engine.dart
│   └── dart/
│       └── anywp_engine.dart
├── include/
│   └── anywp_engine/
│       └── anywp_engine_plugin.h
├── macos/
│   ├── anywp_engine.podspec
│   └── CMakeLists.txt
├── sdk/
│   ├── anywp_sdk.js
│   └── anywp_sdk.min.js
├── examples/
│   └── *.html
├── README.md
├── CHANGELOG_CN.md
├── LICENSE
├── pubspec.yaml
└── INTEGRATION_GUIDE_MACOS.md
```

### 源码包

额外包含：
```
anywp_engine_macos_v2.2.0_source/
├── (预编译包的所有内容)
├── macos/
│   └── Classes/
│       ├── AnyWPEnginePlugin.h/m
│       ├── Modules/
│       │   ├── MonitorManager.h/m
│       │   ├── WallpaperManager.h/m
│       │   ├── PowerManager.h/m
│       │   └── MessageBridge.h/m
│       └── Utils/
│           ├── Logger.h/m
│           ├── StatePersistence.h/m
│           └── AWPCustomSchemeHandler.h/m
└── sdk/
    └── src/     (TypeScript 源码)
```

---

## 🚀 自动化脚本

### 构建脚本

```bash
# 构建预编译包和源码包
./scripts/release_macos.sh

# 只构建 SDK
./scripts/build_sdk.sh production

# 验证发布包
./scripts/verify_precompiled_macos.sh 2.2.0
```

### 手动步骤

如果自动化脚本失败，可以手动执行以下步骤：

1. **构建 SDK**:
   ```bash
   cd sdk
   npm install
   npm run build:prod
   cd ..
   ```

2. **构建 macOS Release**:
   ```bash
   cd example
   flutter clean
   flutter pub get
   flutter build macos --release
   cd ..
   ```

3. **打包预编译包**:
   ```bash
   # 手动复制文件到 release/ 目录
   # 参考 release_macos.sh 脚本
   ```

---

## 📊 发布检查清单

完整发布前检查：

### 代码
- [ ] 所有代码通过 `flutter analyze`
- [ ] 所有测试通过
- [ ] 版本号一致

### 文档
- [ ] README.md 更新
- [ ] CHANGELOG_CN.md 更新
- [ ] API 文档更新
- [ ] 集成指南完整

### 构建
- [ ] Debug 构建成功
- [ ] Release 构建成功
- [ ] 预编译包验证通过
- [ ] 源码包验证通过

### 测试
- [ ] 基本功能测试
- [ ] 多显示器测试
- [ ] 电源管理测试
- [ ] 双向通信测试
- [ ] 状态持久化测试

### 发布
- [ ] GitHub Release 创建
- [ ] Tag 推送
- [ ] 发布笔记完整
- [ ] 下载链接可用

---

## 🆘 常见问题

### Q: 构建失败，提示找不到 SDK

**解决方案**:
```bash
# 确保 SDK 已构建
./scripts/build_sdk.sh production

# 检查 SDK 文件
ls -la sdk/dist/
```

### Q: 预编译包过大

**解决方案**:
- 使用压缩的 SDK（`anywp_sdk.min.js`）
- 不包含调试符号
- 移除不必要的示例文件

### Q: 如何同时发布 Windows 和 macOS

**解决方案**:
1. 在 Windows 机器上运行 `.\scripts\release.bat`
2. 在 macOS 机器上运行 `./scripts/release_macos.sh`
3. 合并 `release/` 目录
4. 创建统一的 GitHub Release

---

**版本**: 2.2.0  
**更新日期**: 2025-11-17

