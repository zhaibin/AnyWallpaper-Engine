# 🔀 AnyWP Engine - 跨平台集成指南

本文档说明如何在已有的 Windows 项目中添加 macOS 支持，或如何创建同时支持 Windows 和 macOS 的新项目。

---

## 📋 目录

- [场景 1: 已有 Windows 项目，添加 macOS 支持](#场景-1-已有-windows-项目添加-macos-支持)
- [场景 2: 创建新的跨平台项目](#场景-2-创建新的跨平台项目)
- [场景 3: 已有 macOS 项目，添加 Windows 支持](#场景-3-已有-macos-项目添加-windows-支持)
- [统一代码库管理](#统一代码库管理)
- [平台差异处理](#平台差异处理)
- [常见问题](#常见问题)

---

## 场景 1: 已有 Windows 项目，添加 macOS 支持

假设你已经有一个使用 AnyWP Engine 的 Windows Flutter 项目，现在想要添加 macOS 支持。

### Step 1: 检查当前版本

确认你当前使用的 AnyWP Engine 版本：

```yaml
# pubspec.yaml
dependencies:
  anywp_engine:
    path: ./packages/anywp_engine_v2.1.x  # 旧版本（仅 Windows）
```

### Step 2: 升级到 v2.2.0+

**重要**: macOS 支持从 v2.2.0 开始提供。

#### 方式 A: 使用预编译包（推荐）

1. **下载最新的预编译包**

   从 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases) 下载：
   - Windows: `anywp_engine_v2.2.0_precompiled.zip`
   - macOS: `anywp_engine_macos_v2.2.0_precompiled.zip`

2. **更新项目结构**

   ```
   YourProject/
   ├── lib/
   ├── windows/
   ├── macos/           ← Flutter 会自动创建
   ├── pubspec.yaml
   └── packages/
       └── anywp_engine/  ← 统一的包名（不带版本号）
   ```

3. **Windows 端配置**

   ```bash
   # 在 Windows 机器上
   cd YourProject
   
   # 解压 Windows 预编译包到 packages/anywp_engine
   # 注意：解压后重命名为 anywp_engine（去掉版本号）
   
   # 或者使用安装脚本
   packages\anywp_engine\setup_precompiled.bat
   ```

4. **macOS 端配置**

   ```bash
   # 在 macOS 机器上（或通过 Git 同步后）
   cd YourProject
   
   # 如果 macOS 目录不存在，创建它
   flutter create --platforms=macos .
   
   # 解压 macOS 预编译包（覆盖或合并到 packages/anywp_engine）
   # 关键：Windows 和 macOS 的 Dart API 是相同的
   
   # 配置 Entitlements
   # 编辑 macos/Runner/DebugProfile.entitlements
   # 编辑 macos/Runner/Release.entitlements
   ```

5. **配置 macOS Entitlements**

   编辑 `macos/Runner/DebugProfile.entitlements` 和 `macos/Runner/Release.entitlements`：

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
       
       <!-- 允许 JIT（JavaScript 运行时编译） -->
       <key>com.apple.security.cs.allow-jit</key>
       <true/>
       
       <!-- 如果需要访问本地文件 -->
       <key>com.apple.security.files.user-selected.read-only</key>
       <true/>
   </dict>
   </plist>
   ```

6. **更新 pubspec.yaml**

   ```yaml
   dependencies:
     flutter:
       sdk: flutter
     anywp_engine:
       path: ./packages/anywp_engine  # 统一路径，不带版本号
   ```

7. **安装 CocoaPods 依赖（macOS）**

   ```bash
   # 在 macOS 上
   cd macos
   pod install
   cd ..
   ```

8. **测试两个平台**

   **Windows**:
   ```bash
   flutter pub get
   flutter run -d windows
   ```

   **macOS**:
   ```bash
   flutter pub get
   flutter run -d macos
   ```

#### 方式 B: 使用 Git 引用（推荐给团队）

如果你的团队使用 Git 管理项目：

```yaml
# pubspec.yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
      ref: v2.2.0  # 或 main 分支
```

这样做的好处：
- ✅ 自动获取最新版本
- ✅ 团队成员统一版本
- ✅ 无需手动下载预编译包
- ✅ 两个平台共享同一个包

⚠️ **注意**: 使用 Git 引用时，Flutter 会自动处理平台差异。

### Step 3: 更新代码（如果需要）

**好消息**: AnyWP Engine v2.2.0 的 Dart API 与之前版本 **100% 兼容**！

你的现有代码无需修改，例如：

```dart
// 这段代码在 Windows 和 macOS 上都能运行，无需改动
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);
```

**可选**: 如果你想针对不同平台进行优化：

```dart
import 'dart:io' show Platform;
import 'package:anywp_engine/anywp_engine.dart';

Future<void> initWallpaper() async {
  if (Platform.isWindows) {
    // Windows 特定配置
    await AnyWPEngine.initializeWallpaper(
      url: 'file:///C:/wallpapers/windows_wallpaper.html',
      enableMouseTransparent: true,
    );
  } else if (Platform.isMacOS) {
    // macOS 特定配置
    await AnyWPEngine.initializeWallpaper(
      url: 'https://example.com/macos_wallpaper.html',
      enableMouseTransparent: true,
    );
  }
}
```

### Step 4: 处理平台差异

某些功能在不同平台上有差异：

| 功能 | Windows | macOS | 说明 |
|------|---------|-------|------|
| **基础壁纸** | ✅ | ✅ | 完全支持 |
| **多显示器** | ✅ | ✅ | 完全支持 |
| **电源管理** | ✅ | ✅ | 完全支持 |
| **双向通信** | ✅ | ✅ | 完全支持 |
| **状态持久化** | ✅ | ✅ | 完全支持 |
| **交互模式** | ✅ | ⏳ | macOS 待实现 |
| **文件加密** | ✅ | ⏳ | macOS 待实现 |

**处理方式**:

```dart
// 检查功能支持
Future<void> setupAdvancedFeatures() async {
  if (Platform.isWindows) {
    // Windows 独有功能
    // await AnyWPEngine.setInteractiveMode(true);
  }
  
  // 通用功能（两平台都支持）
  await AnyWPEngine.setAutoPowerSaving(true);
}
```

### Step 5: 构建和测试

**Windows**:
```bash
flutter build windows --release
# 输出: build/windows/x64/runner/Release/
```

**macOS**:
```bash
flutter build macos --release
# 输出: build/macos/Build/Products/Release/
```

---

## 场景 2: 创建新的跨平台项目

从零开始创建一个同时支持 Windows 和 macOS 的项目。

### Step 1: 创建 Flutter 项目

```bash
# 创建项目，同时启用 Windows 和 macOS
flutter create --platforms=windows,macos my_wallpaper_app
cd my_wallpaper_app
```

### Step 2: 添加 AnyWP Engine 依赖

#### 方式 A: 预编译包（快速开始）

1. 创建 `packages/` 目录
2. 下载并解压预编译包到 `packages/anywp_engine`
3. 更新 `pubspec.yaml`:

```yaml
dependencies:
  flutter:
    sdk: flutter
  anywp_engine:
    path: ./packages/anywp_engine
```

#### 方式 B: Git 引用（推荐）

```yaml
dependencies:
  flutter:
    sdk: flutter
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
      ref: v2.2.0
```

### Step 3: 配置平台特定设置

**Windows**:
- ✅ 通常无需额外配置
- ✅ WebView2 Runtime 会自动处理

**macOS**:

编辑 `macos/Runner/DebugProfile.entitlements` 和 `macos/Runner/Release.entitlements`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.network.client</key>
    <true/>
    <key>com.apple.security.network.server</key>
    <true/>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
</dict>
</plist>
```

### Step 4: 编写跨平台代码

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'dart:io' show Platform;

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 设置应用名称（跨平台）
  await AnyWPEngine.setApplicationName('MyWallpaperApp');
  
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Cross-Platform Wallpaper',
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
  String _platformName = '';

  @override
  void initState() {
    super.initState();
    _platformName = Platform.isWindows ? 'Windows' : 'macOS';
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Wallpaper Engine - $_platformName'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              '当前平台: $_platformName',
              style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
            ),
            SizedBox(height: 40),
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
    try {
      bool success = await AnyWPEngine.initializeWallpaper(
        url: 'https://www.bing.com',
        enableMouseTransparent: true,
      );
      
      if (success) {
        setState(() => _isRunning = true);
        _showMessage('壁纸启动成功！');
      } else {
        _showMessage('壁纸启动失败');
      }
    } catch (e) {
      _showMessage('错误: $e');
    }
  }

  Future<void> _stopWallpaper() async {
    try {
      await AnyWPEngine.stopWallpaper();
      setState(() => _isRunning = false);
      _showMessage('壁纸已停止');
    } catch (e) {
      _showMessage('错误: $e');
    }
  }

  void _showMessage(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message)),
    );
  }
}
```

### Step 5: 安装依赖并运行

```bash
# 获取依赖
flutter pub get

# macOS 需要额外安装 CocoaPods 依赖
cd macos && pod install && cd ..

# 在 Windows 上运行
flutter run -d windows

# 在 macOS 上运行
flutter run -d macos
```

---

## 场景 3: 已有 macOS 项目，添加 Windows 支持

如果你先在 macOS 上开发，现在想添加 Windows 支持：

### Step 1: 添加 Windows 平台支持

```bash
flutter create --platforms=windows .
```

### Step 2: 配置 Windows 预编译包

参考 [场景 1](#场景-1-已有-windows-项目添加-macos-支持) 的 Windows 配置步骤。

### Step 3: 测试

```bash
# 在 Windows 机器上
flutter pub get
flutter run -d windows
```

---

## 统一代码库管理

### 推荐的项目结构

```
YourProject/
├── lib/
│   ├── main.dart                 # 跨平台入口
│   ├── core/
│   │   └── wallpaper_service.dart  # 统一的业务逻辑
│   └── platform/
│       ├── windows_config.dart   # Windows 特定配置
│       └── macos_config.dart     # macOS 特定配置
├── windows/
│   └── (Windows 配置)
├── macos/
│   └── (macOS 配置)
├── packages/
│   └── anywp_engine/             # 统一的插件包
├── assets/
│   ├── wallpapers/
│   │   ├── common/               # 跨平台壁纸
│   │   ├── windows/              # Windows 专用
│   │   └── macos/                # macOS 专用
└── pubspec.yaml
```

### 平台抽象层示例

```dart
// lib/core/wallpaper_service.dart
import 'package:anywp_engine/anywp_engine.dart';
import 'dart:io' show Platform;
import '../platform/windows_config.dart' if (dart.library.io) '../platform/windows_config.dart';
import '../platform/macos_config.dart' if (dart.library.io) '../platform/macos_config.dart';

class WallpaperService {
  static Future<void> initialize() async {
    // 平台检测
    if (Platform.isWindows) {
      await WindowsConfig.setup();
    } else if (Platform.isMacOS) {
      await MacOSConfig.setup();
    }
  }

  static Future<bool> startWallpaper(String url) async {
    return await AnyWPEngine.initializeWallpaper(
      url: url,
      enableMouseTransparent: true,
    );
  }

  static Future<void> stopWallpaper() async {
    await AnyWPEngine.stopWallpaper();
  }
}

// lib/platform/windows_config.dart
class WindowsConfig {
  static Future<void> setup() async {
    // Windows 特定配置
    print('Setting up Windows configuration...');
  }
}

// lib/platform/macos_config.dart
class MacOSConfig {
  static Future<void> setup() async {
    // macOS 特定配置
    print('Setting up macOS configuration...');
  }
}
```

---

## 平台差异处理

### 1. URL 路径差异

**问题**: Windows 和 macOS 的文件路径格式不同。

**解决方案**:

```dart
import 'dart:io' show Platform;
import 'package:path/path.dart' as path;

String getWallpaperPath(String filename) {
  if (Platform.isWindows) {
    return 'file:///C:/wallpapers/$filename';
  } else if (Platform.isMacOS) {
    // macOS 使用 https 或相对路径更安全
    return 'https://example.com/wallpapers/$filename';
  }
  return '';
}
```

### 2. 功能兼容性检查

```dart
Future<bool> isFeatureSupported(String feature) async {
  switch (feature) {
    case 'interactive_mode':
      return Platform.isWindows;  // 仅 Windows 支持
    case 'file_encryption':
      return Platform.isWindows;  // 仅 Windows 支持
    case 'basic_wallpaper':
      return true;  // 两平台都支持
    default:
      return false;
  }
}

// 使用
if (await isFeatureSupported('interactive_mode')) {
  // 启用交互模式
}
```

### 3. UI 适配

```dart
Widget buildPlatformButton(BuildContext context) {
  if (Platform.isWindows) {
    return ElevatedButton(
      onPressed: () {},
      child: Text('Windows Style Button'),
    );
  } else if (Platform.isMacOS) {
    return CupertinoButton(
      onPressed: () {},
      child: Text('macOS Style Button'),
    );
  }
  return SizedBox.shrink();
}
```

---

## 常见问题

### Q1: 两个平台的预编译包可以共存吗？

**答**: 可以，而且推荐共存！

```
packages/anywp_engine/
├── lib/
│   └── anywp_engine.dart        # Dart API（共享）
├── windows/                      # Windows 特定文件
│   ├── CMakeLists.txt
│   └── anywp_engine_plugin.dll
├── macos/                        # macOS 特定文件
│   ├── anywp_engine.podspec
│   └── Classes/
└── sdk/
    └── anywp_sdk.js             # Web SDK（共享）
```

Flutter 会自动选择对应平台的文件。

### Q2: 如何在 Git 中管理跨平台项目？

**答**: 使用统一的 `.gitignore`：

```gitignore
# Flutter
.dart_tool/
.packages
build/

# Windows
windows/flutter/
*.dll
*.lib

# macOS
macos/Flutter/
macos/Pods/
*.xcworkspace

# 保留预编译包（可选）
# packages/anywp_engine/
```

**提示**: 如果团队成员在不同平台，考虑使用 Git 引用而非预编译包。

### Q3: 如何处理平台特定的资源文件？

**答**: 使用条件编译或运行时检测：

```dart
String getAssetPath(String filename) {
  final platform = Platform.isWindows ? 'windows' : 'macos';
  return 'assets/wallpapers/$platform/$filename';
}

// 在 pubspec.yaml 中
flutter:
  assets:
    - assets/wallpapers/windows/
    - assets/wallpapers/macos/
    - assets/wallpapers/common/
```

### Q4: 团队协作时如何同步配置？

**答**: 使用版本控制 + 文档：

1. **提交配置模板**:
   ```bash
   git add windows/CMakeLists.txt
   git add macos/Runner/*.entitlements
   git add pubspec.yaml
   git commit -m "Add cross-platform configuration"
   ```

2. **创建设置脚本**:
   ```bash
   # setup.sh (macOS)
   #!/bin/bash
   flutter pub get
   cd macos && pod install && cd ..
   
   # setup.bat (Windows)
   flutter pub get
   ```

3. **团队文档**:
   在 README.md 中说明不同平台的设置步骤。

### Q5: 性能差异如何处理？

**答**: 根据平台调整配置：

```dart
Future<void> optimizeForPlatform() async {
  if (Platform.isWindows) {
    // Windows 性能较好，可以用更高的设置
    await AnyWPEngine.setAutoPowerSaving(false);
  } else if (Platform.isMacOS) {
    // macOS WKWebView 内存较高，启用自动优化
    await AnyWPEngine.setAutoPowerSaving(true);
  }
}
```

### Q6: 如何测试跨平台兼容性？

**答**: 使用自动化测试 + 手动测试：

```dart
// test/cross_platform_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'dart:io';

void main() {
  test('AnyWP Engine should work on current platform', () async {
    // 测试基础功能
    final monitors = await AnyWPEngine.getMonitors();
    expect(monitors.length, greaterThan(0));
    
    // 测试平台特定功能
    if (Platform.isWindows) {
      // Windows 特定测试
    } else if (Platform.isMacOS) {
      // macOS 特定测试
    }
  });
}
```

---

## 📚 相关文档

- [Windows 预编译包集成指南](PRECOMPILED_DLL_INTEGRATION.md)
- [macOS 预编译包集成指南](PRECOMPILED_MACOS_INTEGRATION.md)
- [多平台架构设计](MULTIPLATFORM_ARCHITECTURE.md)
- [API 参考文档](DEVELOPER_API_REFERENCE.md)

---

## 🤝 获取帮助

- 📖 查看完整文档
- 🐛 提交 [GitHub Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论区](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

---

**版本**: 2.2.0  
**更新日期**: 2025-11-17  
**重要性**: ⭐⭐⭐⭐⭐ (必读文档)

