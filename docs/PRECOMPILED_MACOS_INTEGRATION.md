# 📦 使用预编译包快速集成 AnyWP Engine (macOS)

本指南说明如何在 macOS 上使用预编译包集成 AnyWP Engine，**无需 Xcode 编译 Objective-C 代码**。

---

## 📦 两种发布包说明

从 v2.2.0 开始，AnyWP Engine 提供 macOS 预编译包：

### 1️⃣ **预编译包** (`anywp_engine_macos_v{版本号}_precompiled.zip`) - 推荐

**包含内容**：
- ✅ `Frameworks/` - 预编译的插件框架（或通过 CocoaPods 自动获取）
- ✅ `lib/` - Dart API
- ✅ `include/anywp_engine/` - C API 头文件
- ✅ `macos/` - CocoaPods 配置 (podspec + CMakeLists.txt)
- ✅ `sdk/anywp_sdk.js` - **Web SDK**（用于开发 HTML 壁纸）
- ✅ `examples/` - **示例 HTML 文件**
- ✅ `INTEGRATION_GUIDE_MACOS.md` - macOS 集成指南（本文档）
- ✅ 文档和许可证

**优势**：
- ✨ **最小化依赖** - 只需要 Flutter SDK 和 Xcode Command Line Tools
- ✨ **简化集成** - 通过 CocoaPods 自动管理依赖
- ✨ **快速构建** - 跳过 Objective-C 编译步骤
- ✨ **体积小** - 约 0.5MB（包含 Web SDK 和示例）
- ✨ **一站式解决方案** - 包含 Flutter 插件和 Web SDK

**适用场景**：
- ✅ 大多数 Flutter 开发者（直接使用插件功能）
- ✅ 生产环境部署
- ✅ CI/CD 自动化构建

### 2️⃣ **源码包** (`anywp_engine_macos_v{版本号}_source.zip`)

**包含内容**：
- ✅ 预编译包的所有内容
- ✅ `macos/Classes/` - 完整 Objective-C 源码
  - `Modules/` - 核心功能模块
  - `Utils/` - 工具类
- ✅ `sdk/` - TypeScript SDK 源码

**适用场景**：
- 🔧 需要修改 Objective-C 代码
- 🔧 需要调试插件内部
- 🔧 需要自定义功能
- 🔧 学习插件实现原理

**前置要求**：
- Xcode 12+
- CocoaPods 1.10+
- macOS 10.14+

---

## 🎯 适用场景

### ✅ 适合使用预编译包：

- ✅ **快速集成**：不想从源码编译 Objective-C 代码
- ✅ **简化构建**：减少项目构建时间和复杂度
- ✅ **生产环境**：使用稳定的发布版本
- ✅ **团队协作**：统一团队使用的插件版本
- ✅ **CI/CD 流程**：在自动化构建环境中使用

### ⚠️ 不适合使用预编译包：

- ❌ **需要修改 Objective-C 代码**：建议从源码构建
- ❌ **需要调试插件**：需要完整的开发环境
- ❌ **需要自定义功能**：建议 fork 源码

---

## 📥 方式一：从 GitHub Release 下载（推荐）

### 1. 下载预编译包

访问 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases) 页面：

**推荐：预编译包**（最简单集成）
```
anywp_engine_macos_v2.2.0_precompiled.zip
```

**可选：源码包**（需要自定义修改）
```
anywp_engine_macos_v2.2.0_source.zip
```

### 2. 解压到项目目录

将 ZIP 文件解压到你的项目根目录的 `packages/` 子目录：

```
YourProject/
├── lib/
├── macos/
├── pubspec.yaml
└── packages/
    └── anywp_engine/  ← 解压到这里（重命名为 anywp_engine）
        ├── Frameworks/   (可选，通过 CocoaPods 自动获取)
        ├── lib/
        │   ├── dart/
        │   │   └── anywp_engine.dart
        │   └── anywp_engine.dart
        ├── macos/
        │   ├── anywp_engine.podspec
        │   └── CMakeLists.txt
        ├── sdk/
        │   ├── anywp_sdk.js
        │   └── anywp_sdk.min.js
        ├── examples/
        ├── pubspec.yaml
        ├── README.md
        ├── INTEGRATION_GUIDE_MACOS.md  ← 完整集成指南（本文档）
        ├── CHANGELOG_CN.md
        └── LICENSE
```

**重要提示**：建议将解压后的文件夹重命名为 `anywp_engine`（去掉版本号），这样升级时只需替换内容，无需修改 `pubspec.yaml`。

### 3. 在 pubspec.yaml 中引用

在你的 Flutter 项目的 `pubspec.yaml` 中添加依赖：

```yaml
dependencies:
  flutter:
    sdk: flutter
  anywp_engine:
    path: ./packages/anywp_engine
```

### 4. 配置 Entitlements

macOS 应用需要正确的 entitlements 才能访问网络和其他资源。

编辑 `macos/Runner/DebugProfile.entitlements` 和 `macos/Runner/Release.entitlements`，添加：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <!-- 网络访问权限 -->
    <key>com.apple.security.network.client</key>
    <true/>
    <key>com.apple.security.network.server</key>
    <true/>
    
    <!-- 允许 JIT（JavaScript 运行时编译，WKWebView 需要） -->
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    
    <!-- 如果需要访问本地文件 -->
    <key>com.apple.security.files.user-selected.read-only</key>
    <true/>
    
    <!-- 如果需要在 Release 模式下禁用沙箱（谨慎使用） -->
    <!-- <key>com.apple.security.app-sandbox</key> -->
    <!-- <false/> -->
</dict>
</plist>
```

### 5. 获取依赖并构建

```bash
# 安装 Flutter 依赖
flutter pub get

# 安装 CocoaPods 依赖
cd macos
pod install
cd ..

# 构建和运行
flutter run -d macos

# 或者构建 Release 版本
flutter build macos --release
```

**就这么简单！** Flutter 会自动处理插件的加载。

**技术细节**：
- 预编译包使用 CocoaPods 管理依赖
- 插件通过 podspec 自动集成到项目中
- 无需手动编译 Objective-C 源码
- Flutter 构建系统会自动链接插件
- **Web SDK 已包含**：`sdk/anywp_sdk.js` 可直接用于 HTML 壁纸开发
- **示例文件已包含**：`examples/` 目录包含多个示例 HTML 文件

---

## 📥 方式二：使用 pubspec.yaml 的 git 引用（团队协作）

如果你的团队使用 Git 管理预编译包，可以这样引用：

```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/your-org/anywp_engine_precompiled_macos.git
      ref: v2.2.0  # 或使用 main 分支
```

---

## 📚 使用示例

### 基础使用

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: WallpaperController(),
    );
  }
}

class WallpaperController extends StatefulWidget {
  @override
  _WallpaperControllerState createState() => _WallpaperControllerState();
}

class _WallpaperControllerState extends State<WallpaperController> {
  bool _isRunning = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('AnyWP Engine - macOS')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ElevatedButton(
              onPressed: _isRunning ? null : _startWallpaper,
              child: Text('启动壁纸'),
            ),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: _isRunning ? _stopWallpaper : null,
              child: Text('停止壁纸'),
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _startWallpaper() async {
    bool success = await AnyWPEngine.initializeWallpaper(
      url: 'https://www.bing.com',
      enableMouseTransparent: true,
    );
    
    if (success) {
      setState(() => _isRunning = true);
    }
  }

  Future<void> _stopWallpaper() async {
    await AnyWPEngine.stopWallpaper();
    setState(() => _isRunning = false);
  }
}
```

---

## 📋 包含的文件说明

### Frameworks/ - 预编译框架（可选）

可以通过 CocoaPods 自动获取，也可以手动包含预编译的插件框架。

### lib/ - Dart API

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_engine.dart` | Dart API（标准位置） | ✅ 必需 |
| `dart/anywp_engine.dart` | Dart API（向后兼容） | ✅ 必需 |

### macos/ - 平台配置

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_engine.podspec` | CocoaPods 配置 | ✅ 必需 |
| `CMakeLists.txt` | CMake 配置 | ✅ 必需 |

### sdk/ - Web SDK

| 文件 | 说明 | 用途 |
|------|------|------|
| `anywp_sdk.js` | **Web SDK JavaScript 文件** | ✅ 用于 HTML 壁纸开发 |
| `anywp_sdk.min.js` | **Web SDK 压缩版** | ✅ 生产环境推荐 |

### examples/ - 示例文件

包含多个 HTML 示例文件，涵盖基本用法、React、Vue、双向通信等。

---

## 🔄 版本更新

### 更新到新版本

由于建议将文件夹重命名为 `anywp_engine`（无版本号），更新非常简单：

1. **备份当前版本**（可选）：
   ```bash
   mv packages/anywp_engine packages/anywp_engine_backup
   ```

2. **下载并解压新版本**：
   - 访问 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)
   - 下载最新的 `anywp_engine_macos_v2.2.0_precompiled.zip`
   - 解压到 `packages/anywp_engine`

3. **重新构建**：
   ```bash
   flutter clean
   flutter pub get
   cd macos && pod install && cd ..
   flutter build macos
   ```

**无需修改 `pubspec.yaml`**，因为路径保持不变！

---

## 🔍 验证安装

### 检查插件是否正确加载

创建一个测试脚本 `test_plugin.dart`：

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  try {
    // 测试插件是否可用
    final monitors = await AnyWPEngine.getMonitors();
    print('✅ AnyWP Engine 加载成功');
    print('📺 检测到 ${monitors.length} 个显示器');
  } catch (e) {
    print('❌ AnyWP Engine 加载失败: $e');
  }
}
```

运行测试：
```bash
flutter run -d macos
```

---

## 🚫 使用预编译包时不需要的操作

使用预编译包时，**你不需要**：

- ❌ **手动编译 Objective-C 代码** - 框架已经预编译好
- ❌ **打开 Xcode 编辑源码** - 无需修改插件代码
- ❌ **手动配置 CocoaPods** - podspec 已包含所有配置
- ❌ **手动复制框架文件** - CocoaPods 会自动处理

**你只需要**：
1. 解压预编译包到 `packages/`
2. 在 `pubspec.yaml` 中添加依赖
3. 配置正确的 entitlements
4. 运行 `flutter pub get && cd macos && pod install`
5. 运行 `flutter run -d macos`

---

## 🐛 常见问题

### Q: 构建时提示找不到插件？

**可能原因**：
- 预编译包路径不正确
- `pubspec.yaml` 中的路径配置错误
- CocoaPods 没有安装依赖

**解决方案**：
```bash
# 1. 确认包文件存在
ls -la packages/anywp_engine

# 2. 检查 pubspec.yaml 中的路径
#    应该是: path: ./packages/anywp_engine

# 3. 重新获取依赖
flutter clean
flutter pub get
cd macos
pod install
pod update  # 如果 pod install 不够
cd ..
flutter build macos
```

### Q: 运行时提示权限错误？

**原因**：macOS 沙箱限制或缺少 entitlements。

**解决方案**：

1. **检查 Entitlements**：
   确保 `macos/Runner/*.entitlements` 包含必要的权限（见上文）。

2. **检查 Info.plist**：
   如果需要访问网络，确保 `macos/Runner/Info.plist` 正确配置。

3. **禁用沙箱（仅调试）**：
   ```xml
   <key>com.apple.security.app-sandbox</key>
   <false/>
   ```
   **注意**：生产环境不推荐禁用沙箱。

### Q: WKWebView 无法加载 file:// URL？

**原因**：WKWebView 有沙箱限制，无法直接访问本地文件。

**解决方案**：
1. **使用 https:// URL**（推荐）
2. **使用 data: URI**
3. **使用自定义 URL Scheme**（需要源码修改）
4. **配置 entitlements** 允许文件访问

### Q: 如何在 Xcode 中调试？

**步骤**：
```bash
# 1. 打开 Xcode workspace
open macos/Runner.xcworkspace

# 2. 在 Xcode 中选择 Run scheme
# 3. 设置断点并运行
```

### Q: 预编译包支持哪些平台？

- ✅ macOS 10.14+ (x86_64)
- ✅ macOS 11+ (Apple Silicon - arm64)
- ❌ iOS（不支持，API 设计不适用）

如需其他平台，请使用源码包自行编译。

### Q: 可以同时使用预编译包和源码包吗？

**不可以**。必须选择其中一种：
- **预编译包**：快速、简单、无需编译
- **源码包**：灵活、可定制、需要开发环境

混合使用会导致版本冲突和编译错误。

---

## 🔧 常见问题快速修复

### 问题1：CocoaPods 找不到插件

**错误信息：** `[!] Unable to find a specification for 'anywp_engine'`

**解决方案：**

确保 podspec 文件在正确位置：
```bash
ls -la packages/anywp_engine/macos/anywp_engine.podspec
```

如果不存在，重新解压预编译包。

### 问题2：构建时提示 "Library not loaded"

**错误信息：** `Library not loaded: @rpath/...`

**解决方案：**

重新安装 CocoaPods 依赖：
```bash
cd macos
rm -rf Pods
rm Podfile.lock
pod install
cd ..
flutter clean
flutter build macos
```

### 问题3：运行时崩溃

**可能原因：** Entitlements 配置错误或权限不足。

**解决方案：**

1. 检查 Console.app 中的崩溃日志
2. 确认所有必需的 entitlements 已配置
3. 尝试在非沙箱模式下运行（仅调试）

---

## 📊 集成方式对比

| 方式 | 构建时间 | 环境要求 | 灵活性 | 推荐场景 |
|------|---------|---------|--------|---------|
| **预编译包** | ⚡ 快 | ✅ 简单 | ⚠️ 受限 | 生产环境、快速集成 |
| **源码编译** | 🐌 慢 | ❌ 复杂 | ✅ 完全 | 开发调试、自定义功能 |
| **Git 引用** | 🐌 慢 | ❌ 复杂 | ✅ 完全 | 团队协作、版本追踪 |

---

## 📝 完整集成清单

### 首次集成

- [ ] 下载预编译包 (`anywp_engine_macos_v2.2.0_precompiled.zip`)
- [ ] 解压到项目的 `packages/anywp_engine` 目录
- [ ] 在 `pubspec.yaml` 中添加依赖
- [ ] 配置 entitlements
- [ ] 运行 `flutter pub get`
- [ ] 运行 `cd macos && pod install`
- [ ] 运行 `flutter build macos` 测试构建
- [ ] 验证插件加载成功

### 后续开发

- [ ] 在代码中引入 `package:anywp_engine/anywp_engine.dart`
- [ ] 调用 API 实现壁纸功能
- [ ] 测试所有功能
- [ ] 构建 Release 版本并测试
- [ ] 部署到目标设备

---

## 📚 更多资源

- **完整文档**：[README.md](../README.md)
- **macOS 开发指南**：[docs/MACOS_DEVELOPER_GUIDE.md](docs/MACOS_DEVELOPER_GUIDE.md)
- **API 参考**：[docs/DEVELOPER_API_REFERENCE.md](docs/DEVELOPER_API_REFERENCE.md)
- **Web 开发指南**：[docs/WEB_DEVELOPER_GUIDE_CN.md](docs/WEB_DEVELOPER_GUIDE_CN.md)
- **发布页面**：[GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)

---

## 🤝 获取帮助

遇到问题？

- 📖 首先查看本文档的 [常见问题](#-常见问题) 部分
- 🐛 提交 [GitHub Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论区](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

---

**Version**: 2.2.0  
**Updated**: 2025-11-17  
**主要变更**:
- ✅ **首次发布 macOS 预编译包支持**
- ✅ **CocoaPods 集成** - 简化依赖管理
- ✅ **完整的 entitlements 配置指南**
- ✅ **跨平台 Web SDK 支持**
- ✅ **详细的故障排除指南**

