# 📦 使用预编译 DLL 快速集成 AnyWP Engine

本指南说明如何使用预编译的 DLL 文件集成 AnyWP Engine，**无需安装 WebView2 SDK 或编译 C++ 代码**。

---

## 🎯 适用场景

### ✅ 适合使用预编译 DLL：

- ✅ **快速集成**：不想安装 Visual Studio 和 WebView2 SDK
- ✅ **简化构建**：减少项目构建时间和复杂度
- ✅ **生产环境**：使用稳定的发布版本
- ✅ **团队协作**：统一团队使用的插件版本
- ✅ **CI/CD 流程**：在自动化构建环境中使用

### ⚠️ 不适合使用预编译 DLL：

- ❌ **需要修改 C++ 代码**：建议从源码构建
- ❌ **需要调试插件**：需要完整的开发环境
- ❌ **需要自定义功能**：建议 fork 源码

---

## 📥 方式一：从 GitHub Release 下载（推荐）

### 1. 下载预编译包

访问 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases) 页面，下载最新版本：

```
anywp_engine_v2.0.0.zip
```

### 2. 解压到项目目录

将 ZIP 文件解压到你的项目根目录：

```
YourProject/
├── lib/
├── windows/
├── pubspec.yaml
└── packages/
    └── anywp_engine_v1.2.1/  ← 解压到这里（建议放在 packages/）
    ├── bin/
        │   ├── anywp_engine_plugin.dll
        │   └── WebView2Loader.dll
        ├── lib/
        │   ├── anywp_engine.dart
        │   └── anywp_engine_plugin.lib
        ├── include/
        ├── windows/
        │   ├── anywp_sdk.js
        │   ├── CMakeLists.txt
        │   └── src/
        │       └── anywp_engine_plugin.cpp
        ├── setup_precompiled.bat
        ├── verify_precompiled.bat
        ├── generate_pubspec_snippet.bat
        ├── example_minimal/
        └── pubspec.yaml
```

### 3. 一键安装预编译包（推荐）

在 Flutter 项目根目录执行：

```powershell
packages\anywp_engine_v1.2.1\setup_precompiled.bat
```

脚本会自动：

- 验证关键文件是否齐全
- 将预编译包复制到 `packages/anywp_engine_v1.2.1`
- 执行 `flutter pub get`

### 4. 手动在 pubspec.yaml 中引用（可选）

```yaml
dependencies:
  flutter:
    sdk: flutter
  anywp_engine:
    path: ./packages/anywp_engine_v1.2.1
```

### 5. 获取依赖并构建

```bash
flutter pub get
flutter build windows
```

---

## 📥 方式二：直接引用 DLL（简化版）

如果你不想复制文件，可以直接在构建配置中引用 DLL。

### 1. 修改 windows/CMakeLists.txt

在 `windows/CMakeLists.txt` 文件末尾添加：

```cmake
# 引用预编译的 AnyWP Engine 插件
set(ANYWP_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../packages/anywp_engine_v1.2.1")

if(EXISTS "${ANYWP_ENGINE_DIR}")
  # 添加插件库
  add_library(anywp_engine_plugin SHARED IMPORTED)
  set_target_properties(anywp_engine_plugin PROPERTIES
    IMPORTED_LOCATION "${ANYWP_ENGINE_DIR}/bin/anywp_engine_plugin.dll"
    IMPORTED_IMPLIB "${ANYWP_ENGINE_DIR}/lib/anywp_engine_plugin.lib"
  )
  
  # 包含头文件
  target_include_directories(${BINARY_NAME} PRIVATE
    "${ANYWP_ENGINE_DIR}/include"
  )
  
  # 链接插件
  target_link_libraries(${BINARY_NAME} PRIVATE anywp_engine_plugin)
  
  # 复制 DLL 到输出目录
  add_custom_command(TARGET ${BINARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${ANYWP_ENGINE_DIR}/bin/anywp_engine_plugin.dll"
      "$<TARGET_FILE_DIR:${BINARY_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${ANYWP_ENGINE_DIR}/bin/WebView2Loader.dll"
      "$<TARGET_FILE_DIR:${BINARY_NAME}>"
  )
  
  message(STATUS "Using precompiled AnyWP Engine from ${ANYWP_ENGINE_DIR}")
else()
  message(WARNING "Precompiled AnyWP Engine not found at ${ANYWP_ENGINE_DIR}")
endif()
```

### 2. 构建项目

```bash
flutter pub get
flutter build windows
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
      appBar: AppBar(title: Text('AnyWP Engine')),
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

### bin/ - 运行时 DLL

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_engine_plugin.dll` | 插件核心 DLL | ✅ 必需 |
| `WebView2Loader.dll` | WebView2 加载器 | ✅ 必需 |

### lib/ - 库文件

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_engine_plugin.lib` | 静态链接库（仅编译时） | ⚠️ 高级用户 |
| `dart/` | Dart 源代码 | ✅ 必需 |

### include/ - 头文件

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_engine_plugin.h` | C++ API 头文件 | ⚠️ 高级用户 |
| `anywp_engine_plugin_c_api.h` | C API 头文件 | ⚠️ 高级用户 |

### sdk/ - JavaScript SDK

| 文件 | 说明 | 必需 |
|------|------|------|
| `anywp_sdk.js` | 壁纸 JavaScript SDK | ⚠️ Web 开发时需要 |

---

## 🔄 版本更新

### 更新到新版本

1. **下载新版本**：
   ```bash
   # 下载新版本的 ZIP
   # https://github.com/zhaibin/AnyWallpaper-Engine/releases
   ```

2. **删除旧版本**：
   ```bash
   rmdir /s /q packages\anywp_engine_v1.9.9
   ```

3. **解压新版本**：
   ```bash
   # 解压 anywp_engine_v2.0.0.zip 到 packages\anywp_engine_v1.2.1
   ```

4. **更新 pubspec.yaml**：
   ```yaml
   dependencies:
     anywp_engine:
       path: ./packages/anywp_engine_v1.2.1  # 更新版本号
   ```

5. **复制新的 DLL（如需手动复制）**：
   ```bash
   copy packages\anywp_engine_v1.2.1\bin\*.dll windows\plugins\anywp_engine\ /Y
   ```

6. **重新构建**：
   ```bash
   flutter clean
   flutter pub get
   flutter build windows
   ```

---

## 🔍 验证安装

### 检查 DLL 是否正确加载

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
flutter run -d windows
```

---

## 🚫 不需要的操作

使用预编译 DLL 时，**你不需要**：

- ❌ 安装 WebView2 SDK (`setup_webview2.bat`)
- ❌ 安装 Visual Studio
- ❌ 配置 NuGet
- ❌ 编译 C++ 代码
- ❌ 修改 CMakeLists.txt（除非使用方式二）

---

## 🐛 常见问题

### Q: 构建时提示找不到 DLL？

**解决方案**：
```bash
# 确认 DLL 文件存在
dir packages\anywp_engine_v1.2.1\bin\*.dll

# 手动复制到构建输出目录
copy packages\anywp_engine_v1.2.1\bin\*.dll build\windows\runner\Release\ /Y
```

### Q: 运行时提示 DLL 缺失？

**解决方案**：
```bash
# 检查 WebView2 Runtime 是否安装
# Windows 11 自带，Windows 10 需要单独安装
# 下载地址：https://developer.microsoft.com/microsoft-edge/webview2/
```

### Q: 如何验证使用的是预编译版本？

在 Flutter 控制台查看日志：
```
[AnyWP] Using precompiled plugin version 1.1.0
```

### Q: 可以混合使用源码和预编译 DLL 吗？

**不建议**。要么完全使用源码，要么完全使用预编译 DLL，避免版本冲突。

---

## 📊 集成方式对比

| 方式 | 构建时间 | 环境要求 | 灵活性 | 推荐场景 |
|------|---------|---------|--------|---------|
| **预编译 DLL** | ⚡ 快 | ✅ 简单 | ⚠️ 受限 | 生产环境、快速集成 |
| **源码编译** | 🐌 慢 | ❌ 复杂 | ✅ 完全 | 开发调试、自定义功能 |
| **Git 引用** | 🐌 慢 | ❌ 复杂 | ✅ 完全 | 团队协作、版本追踪 |

---

## 📝 完整集成清单

### 首次集成

- [ ] 下载预编译包 (`anywp_engine_v1.1.0.zip`)
- [ ] 解压到项目根目录
- [ ] 更新 `pubspec.yaml` 引用路径
- [ ] 复制 DLL 到插件目录
- [ ] 运行 `flutter pub get`
- [ ] 运行 `flutter build windows`
- [ ] 测试插件是否正常工作

### 后续开发

- [ ] 引入 `package:anywp_engine/anywp_engine.dart`
- [ ] 调用 API 实现壁纸功能
- [ ] 测试所有功能
- [ ] 部署到目标设备

---

## 📚 更多资源

- **完整文档**：[README.md](../README.md)
- **API 参考**：[DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md)
- **使用示例**：[API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md)
- **故障排除**：[TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **发布页面**：[GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)

---

## 🤝 获取帮助

遇到问题？

- 📖 查看 [故障排除文档](TROUBLESHOOTING.md)
- 🐛 提交 [GitHub Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论区](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

---

**版本**: 1.1.0  
**更新日期**: 2025-11-05

