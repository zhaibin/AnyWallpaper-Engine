# 📦 使用预编译 DLL 快速集成 AnyWP Engine

本指南说明如何使用预编译的 DLL 文件集成 AnyWP Engine，**无需安装 WebView2 SDK 或编译 C++ 代码**。

---

## 📦 两种发布包说明

从 v2.1.0 开始，AnyWP Engine 提供两种发布包：

### 1️⃣ **预编译包** (`anywp_engine_v{版本号}_precompiled.zip`) - 推荐

**包含内容**：
- ✅ `bin/` - 预编译的 DLL 文件（包含 WebView2Loader.dll）
- ✅ `lib/` - LIB 文件 + Dart API
- ✅ `include/anywp_engine/` - **纯 C API 头文件**（无 C++ 依赖）
- ✅ `windows/CMakeLists.txt` - **预编译专用 CMakeLists.txt**（使用 IMPORTED 库，无需 WebView2 packages）
- ✅ `sdk/anywp_sdk.js` - **Web SDK**（用于开发 HTML 壁纸）🆕
- ✅ `examples/` - **示例 HTML 文件**（14 个示例）🆕
- ✅ `INTEGRATION_GUIDE.md` - Flutter 集成指南（本文档）
- ✅ `WEB_DEVELOPER_GUIDE_CN.md` / `WEB_DEVELOPER_GUIDE.md` - Web 开发者指南🆕
- ✅ 文档和许可证

**优势**：
- ✨ **最小化依赖** - 只需要 Flutter SDK，无需 WebView2 开发环境
- ✨ **简化集成** - 使用纯 C API，无需处理 C++ 类和依赖
- ✨ **快速构建** - 跳过 C++ 编译步骤
- ✨ **体积小** - 约 0.5MB（包含 Web SDK 和示例）
- ✨ **一站式解决方案** - 包含 Flutter 插件和 Web SDK，可直接开发 HTML 壁纸🆕

**适用场景**：
- ✅ 大多数 Flutter 开发者（直接使用插件功能）
- ✅ 生产环境部署
- ✅ CI/CD 自动化构建

### 2️⃣ **源码包** (`anywp_engine_v{版本号}_source.zip`)

**包含内容**：
- ✅ 预编译包的所有内容
- ✅ `windows/anywp_engine_plugin.cpp/h` - 完整 C++ 源码
- ✅ `windows/modules/` - 所有核心模块
- ✅ `windows/utils/` - 所有工具类
- ✅ `windows/sdk/` - TypeScript SDK 源码
- ✅ `windows/packages/` - WebView2 依赖包
- ✅ `windows/test/` - C++ 单元测试

**适用场景**：
- 🔧 需要修改 C++ 代码
- 🔧 需要调试插件内部
- 🔧 需要自定义功能
- 🔧 学习插件实现原理

**前置要求**：
- Visual Studio 2019+ (C++17)
- WebView2 开发环境

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

访问 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases) 页面，根据需求选择：

**推荐：预编译包**（最简单集成）
```
anywp_engine_v2.3.2_precompiled.zip
```

**可选：源码包**（需要自定义修改）
```
anywp_engine_v2.3.2_source.zip
```

### 2. 解压到项目目录

将 ZIP 文件解压到你的项目根目录的 `packages/` 子目录：

```
YourProject/
├── lib/
├── windows/
├── pubspec.yaml
└── packages/
    └── anywp_engine/  ← 解压到这里（重命名为 anywp_engine）
        ├── bin/
        │   ├── anywp_engine_plugin.dll
        │   └── WebView2Loader.dll
        ├── lib/
        │   ├── dart/
        │   │   └── anywp_engine.dart
        │   └── anywp_engine_plugin.lib
        ├── include/
        │   └── anywp_engine/
        │       └── anywp_engine_plugin_c_api.h  ← 纯 C API 头文件
        ├── windows/
        │   └── CMakeLists.txt  ← 预编译专用配置（无需 WebView2 packages）
        ├── pubspec.yaml
        ├── README.md
        ├── INTEGRATION_GUIDE.md  ← 完整集成指南（本文档）
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

### 4. 获取依赖并构建

```bash
flutter pub get
flutter build windows
```

**就这么简单！** Flutter 会自动处理插件的加载和 DLL 的复制。

**技术细节**：
- 预编译包中的 `windows/CMakeLists.txt` 使用 `IMPORTED` 库模式
- 直接链接预编译的 DLL，无需编译 C++ 源码
- 无需 WebView2 packages，因为 WebView2 已静态链接在 DLL 中
- Flutter 构建系统会自动复制 DLL 到输出目录
- **Web SDK 已包含**：`sdk/anywp_sdk.js` 可直接用于 HTML 壁纸开发
- **示例文件已包含**：`examples/` 目录包含 14 个示例 HTML 文件

---

## 📥 方式二：使用 pubspec.yaml 的 git 引用（团队协作）

如果你的团队使用 Git 管理预编译包，可以这样引用：

```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/your-org/anywp_engine_precompiled.git
      ref: v2.1.0  # 或使用 main 分支
```

---

## 📥 方式三：高级 - 自定义 CMake 配置

⚠️ **注意**：**v2.1.3+ 版本不再需要此方式**！预编译包已包含专用的 CMakeLists.txt，直接使用方式一即可。

**v2.1.3 之前的版本**：如果你需要自定义 CMake 配置，可以直接在项目的 `windows/CMakeLists.txt` 中引用预编译 DLL。

**v2.1.3+ 版本**：预编译包中的 `windows/CMakeLists.txt` 已经是预编译专用配置，无需修改。如果你确实需要自定义配置，可以参考以下示例：

### 1. 修改 windows/CMakeLists.txt

在 `windows/CMakeLists.txt` 文件末尾添加：

```cmake
# ==========================================
# 引用预编译的 AnyWP Engine 插件
# ==========================================
set(ANYWP_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../packages/anywp_engine")

if(EXISTS "${ANYWP_ENGINE_DIR}")
  # 添加插件库（IMPORTED 表示使用外部预编译库）
  add_library(anywp_engine_plugin SHARED IMPORTED)
  set_target_properties(anywp_engine_plugin PROPERTIES
    IMPORTED_LOCATION "${ANYWP_ENGINE_DIR}/bin/anywp_engine_plugin.dll"
    IMPORTED_IMPLIB "${ANYWP_ENGINE_DIR}/lib/anywp_engine_plugin.lib"
  )
  
  # 包含纯 C API 头文件（无 C++ 依赖）
  target_include_directories(${BINARY_NAME} PRIVATE
    "${ANYWP_ENGINE_DIR}/include"
  )
  
  # 链接插件
  target_link_libraries(${BINARY_NAME} PRIVATE anywp_engine_plugin)
  
  # 复制 DLL 到输出目录（确保运行时能找到）
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
  message(FATAL_ERROR "Precompiled AnyWP Engine not found at ${ANYWP_ENGINE_DIR}")
endif()
```

**注意**：v2.1.3+ 版本的预编译包已经包含了这样的配置，你无需手动添加。只有在需要完全自定义构建流程时才使用此方式。

### 2. 使用纯 C API（推荐）

如果你需要在 C++ 代码中调用插件功能，使用纯 C API 头文件：

```cpp
// 在你的 C++ 代码中
#include <anywp_engine/anywp_engine_plugin_c_api.h>

// 注册插件（通常由 Flutter 框架自动调用）
// 你不需要手动调用，除非有特殊需求
void MyPluginRegistration(FlutterDesktopPluginRegistrarRef registrar) {
  AnyWPEnginePluginRegisterWithRegistrar(registrar);
}
```

**纯 C API 的优势**：
- ✅ 无需引入 `<WebView2.h>` 和 C++ 类定义
- ✅ 无需 WebView2 SDK 和 Visual Studio
- ✅ 更简洁的接口，只暴露必要的注册函数
- ✅ 避免 ABI 兼容性问题

### 3. 构建项目

```bash
flutter pub get
flutter build windows
```

---

## 🔍 纯 C API vs 完整 C++ API

AnyWP Engine 提供两种 API 接口：

### 📘 纯 C API（推荐给预编译包用户）

**文件**: `include/anywp_engine/anywp_engine_plugin_c_api.h`

```cpp
// 纯 C API - 简单清晰
#include <anywp_engine/anywp_engine_plugin_c_api.h>

// 只暴露一个注册函数
void AnyWPEnginePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);
```

**优势**：
- ✅ **零依赖** - 不需要 `<WebView2.h>` 或其他 C++ 头文件
- ✅ **简洁** - 只有一个函数，接口清晰
- ✅ **稳定** - C ABI 稳定，避免 C++ 名称修饰问题
- ✅ **快速编译** - 无需解析大量的 WebView2 类型定义

**适用场景**：
- Flutter 插件集成（大多数情况）
- 不需要直接调用插件内部 C++ 类
- 追求最小依赖和最快编译速度

### 📕 完整 C++ API（仅源码包提供）

**文件**: `windows/anywp_engine_plugin.h`

```cpp
// 完整 C++ API - 包含内部实现细节
#include "anywp_engine_plugin.h"
#include <WebView2.h>
#include <wrl.h>

// 暴露完整的 C++ 类定义
class AnyWPEnginePlugin {
  // ... 内部实现细节 ...
};
```

**适用场景**：
- 需要修改插件源码
- 需要调用插件内部的 C++ 类
- 需要深度集成或扩展功能

**前置要求**：
- WebView2 SDK
- Visual Studio 2019+
- C++17 支持

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
| `anywp_engine_plugin_c_api.h` | **纯 C API 头文件**（推荐） | ⚠️ 自定义 CMake 时需要 |

**注意**：预编译包只包含纯 C API 头文件，不包含完整的 C++ API 头文件（`anywp_engine_plugin.h`）。完整的 C++ API 只在源码包中提供。

### windows/ - CMake 配置

| 文件 | 说明 | 特点 |
|------|------|------|
| `CMakeLists.txt` | **预编译专用 CMakeLists.txt** | ✅ 使用 IMPORTED 库<br>✅ 无需 WebView2 packages<br>✅ 无需编译 C++ 源码 |

**重要说明**：预编译包中的 `windows/CMakeLists.txt` 是专门为预编译包设计的，与源码包中的 CMakeLists.txt 不同：

**预编译包 CMakeLists.txt 特点**：
- ✅ 使用 `add_library(anywp_engine_plugin SHARED IMPORTED)` 直接链接预编译 DLL
- ✅ 不查找 WebView2 packages 目录（WebView2 已静态链接在 DLL 中）
- ✅ 不编译任何 C++ 源码文件
- ✅ 自动复制 DLL 到输出目录
- ✅ 零依赖，只需要 Flutter SDK

**源码包 CMakeLists.txt 特点**：
- 需要 WebView2 packages 目录
- 需要编译所有 C++ 源码文件
- 需要 Visual Studio 和 WebView2 SDK

这就是为什么预编译包可以**真正实现零依赖集成**的原因！

### sdk/ - Web SDK

| 文件 | 说明 | 用途 |
|------|------|------|
| `anywp_sdk.js` | **Web SDK JavaScript 文件** | ✅ 用于 HTML 壁纸开发<br>✅ 提供完整的 API（点击、拖拽、双向通信等）<br>✅ 支持 TypeScript 类型定义 |

### examples/ - 示例文件

| 目录 | 说明 | 内容 |
|------|------|------|
| `examples/` | **示例 HTML 文件** | ✅ 14 个示例文件<br>✅ 涵盖基本用法、React、Vue、双向通信等<br>✅ 可直接参考和修改 |

---

## 🔄 版本更新

### 更新到新版本

由于建议将文件夹重命名为 `anywp_engine`（无版本号），更新非常简单：

1. **备份当前版本**（可选）：
   ```bash
   move packages\anywp_engine packages\anywp_engine_backup
   ```

2. **下载并解压新版本**：
   - 访问 [GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)
   - 下载最新的 `anywp_engine_v2.1.9_precompiled.zip`
   - 解压到 `packages\anywp_engine`

3. **重新构建**：
   ```bash
   flutter clean
   flutter pub get
   flutter build windows
   ```

**无需修改 `pubspec.yaml`**，因为路径保持不变！

### 如果使用了版本号文件夹

如果你使用了带版本号的文件夹名（如 `anywp_engine_v2.1.5_precompiled`），更新步骤：

1. **删除旧版本**：
   ```bash
   rmdir /s /q packages\anywp_engine_v2.1.2_precompiled
   ```

2. **解压新版本**：
   ```bash
   # 解压 anywp_engine_v2.1.8_precompiled.zip 到 packages\
   ```

3. **更新 pubspec.yaml**：
   ```yaml
   dependencies:
     anywp_engine:
       path: ./packages/anywp_engine_v2.1.5_precompiled  # 更新版本号
   ```

4. **重新构建**：
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

## 🚫 使用预编译包时不需要的操作

使用预编译 DLL 时，**你不需要**：

- ❌ **安装 WebView2 SDK** - DLL 已包含所有必要功能
- ❌ **安装 Visual Studio** - 无需编译 C++ 代码
- ❌ **配置 NuGet 或 CMake** - Flutter 会自动处理（使用方式一）
- ❌ **手动复制 DLL** - Flutter 构建系统会自动复制（使用方式一）
- ❌ **修改 windows/CMakeLists.txt** - 除非你使用方式三（高级用户）

**你只需要**：
1. 解压预编译包到 `packages/`
2. 在 `pubspec.yaml` 中添加依赖
3. 运行 `flutter pub get` 和 `flutter build windows`

---

## 🐛 常见问题

### Q: 构建时提示找不到 DLL？

**可能原因**：
- 预编译包路径不正确
- `pubspec.yaml` 中的路径配置错误

**解决方案**：
```bash
# 1. 确认 DLL 文件存在
dir packages\anywp_engine\bin\*.dll

# 2. 检查 pubspec.yaml 中的路径
#    应该是: path: ./packages/anywp_engine

# 3. 重新获取依赖
flutter clean
flutter pub get
flutter build windows
```

### Q: 运行时日志提示 "SDK file not found" 或 "没有找到anywp_sdk.js"？

**原因**：C++ 插件在运行时尝试加载 `anywp_sdk.js` 文件，但在常见路径中找不到。

**解决方案**：

1. **v2.1.9+ 自动修复**：
   - C++ 代码已增强，会自动从多个路径查找 SDK 文件
   - 包括从 DLL 所在目录查找预编译包中的 SDK

2. **手动复制 SDK 文件（v2.1.9 之前的版本）**：
   ```bash
   # 找到构建输出目录（例如：build\windows\x64\runner\Debug 或 Release）
   # 复制 SDK 文件到以下位置之一：
   
   # 方式1：复制到可执行文件目录
   copy packages\anywp_engine\sdk\anywp_sdk.js build\windows\x64\runner\Debug\
   
   # 方式2：复制到 windows 子目录
   mkdir build\windows\x64\runner\Debug\windows
   copy packages\anywp_engine\sdk\anywp_sdk.js build\windows\x64\runner\Debug\windows\
   ```

3. **使用 CMakeLists.txt 自动复制（推荐）**：
   
   在你的项目 `windows/CMakeLists.txt` 中添加：
   ```cmake
   # 在文件末尾添加（假设你已经引用了预编译包）
   if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../packages/anywp_engine/sdk/anywp_sdk.js")
     add_custom_command(TARGET ${BINARY_NAME} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E make_directory 
         "$<TARGET_FILE_DIR:${BINARY_NAME}>/windows"
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "${CMAKE_CURRENT_SOURCE_DIR}/../packages/anywp_engine/sdk/anywp_sdk.js"
         "$<TARGET_FILE_DIR:${BINARY_NAME}>/windows/anywp_sdk.js"
       COMMENT "Copying AnyWP SDK to runtime directory"
       VERBATIM
     )
   endif()
   ```

4. **验证 SDK 已加载**：
   
   查看应用日志，应该看到类似输出：
   ```
   [AnyWP] [SDKBridge] SDK loaded from: ... (size: XXXXX bytes)
   ```

**注意**：v2.1.9+ 版本已增强 SDK 查找逻辑，会自动从 DLL 所在目录的多个相对路径查找，优先使用 minified 版本，大多数情况下无需手动复制。

### Q: 构建时提示 "WebView2 package not found"？

**原因**：你使用的可能是旧版本的预编译包（v2.1.2 及之前），或者使用了源码包的 CMakeLists.txt。

**解决方案**：
1. **确保使用最新版本**（v2.1.3+）：
   - 下载最新的 `anywp_engine_v2.1.9_precompiled.zip`
   - v2.1.3 已修复此问题，使用预编译专用 CMakeLists.txt

2. **验证 CMakeLists.txt**：
   ```bash
   # 检查预编译包中的 CMakeLists.txt 是否包含以下内容
   findstr "IMPORTED" packages\anywp_engine\windows\CMakeLists.txt
   # 应该找到 "SHARED IMPORTED" 字样
   ```

3. **如果仍然报错**：
   - 确认使用的是预编译包，不是源码包
   - 检查 `packages/anywp_engine/windows/CMakeLists.txt` 是否存在
   - 重新解压预编译包，确保文件完整

### Q: 运行时提示 "无法加载 anywp_engine_plugin.dll"？

**可能原因**：
- WebView2 Runtime 未安装（Windows 10）
- 缺少 Visual C++ 运行时库

**解决方案**：
```bash
# Windows 11: 自带 WebView2 Runtime，无需安装
# Windows 10: 需要安装 WebView2 Runtime
# 下载地址: https://developer.microsoft.com/microsoft-edge/webview2/
```

### Q: 如何验证使用的是预编译版本？

**方法 1 - 检查文件**：
```bash
# 预编译包只包含 DLL 和纯 C API 头文件，没有源码
dir packages\anywp_engine\windows\*.cpp
# 应该提示找不到文件
```

**方法 2 - 检查 CMakeLists.txt**：
```bash
# 预编译包的 CMakeLists.txt 应该包含 "IMPORTED"
findstr "IMPORTED" packages\anywp_engine\windows\CMakeLists.txt
# 应该找到 "SHARED IMPORTED" 字样
# 不应该包含 "add_library.*\.cpp" 这样的源码编译语句
```

**方法 3 - 构建日志**：
```bash
flutter build windows --verbose
# 查找类似输出：
# "Using precompiled AnyWP Engine DLL: ..."
# "Using Flutter plugin anywp_engine from path"
# 不应该看到 C++ 编译日志（如 "Compiling anywp_engine_plugin.cpp"）
# 不应该看到 "WebView2 package not found" 错误
```

### Q: 可以同时使用预编译包和源码包吗？

**不可以**。必须选择其中一种：
- **预编译包**：快速、简单、无需编译
- **源码包**：灵活、可定制、需要开发环境

混合使用会导致版本冲突和编译错误。

### Q: 预编译包支持哪些平台？

- ✅ Windows x64（主要支持）
- ❌ Windows x86（需要从源码编译）
- ❌ Windows ARM64（需要从源码编译）

如需其他平台，请使用源码包自行编译。

### Q: 预编译包的 DLL 是 Debug 还是 Release 版本？

**Release 版本** - 经过完整优化，适合生产环境：
- 启用了编译器优化
- 移除了调试符号
- 体积更小，性能更好
- 包含完整的错误处理和日志

---

## 🔧 常见问题快速修复

### 问题1：找不到 anywp_engine.dart

**错误信息：** `Error when reading 'lib/anywp_engine.dart': 系统找不到指定的文件`

**解决方案：**

预编译包 v2.1.3+ 已自动修复此问题，Dart 文件同时存在于：
- `lib/anywp_engine.dart`（标准位置）
- `lib/dart/anywp_engine.dart`（向后兼容）

如果遇到此问题，请确保使用最新版本的预编译包。

**手动修复（旧版本）：**
```bash
# Windows
copy packages\anywp_engine_v2.1.5\lib\dart\anywp_engine.dart packages\anywp_engine_v2.1.5\lib\
```

### 问题2：CMake 错误 "No target 'anywp_engine_plugin'"

**错误信息：** `CMake Error: No target 'anywp_engine_plugin'`

**解决方案：**

预编译包 v2.1.3+ 已自动修复此问题，CMakeLists.txt 已包含 `GLOBAL` 关键字。

**手动修复（旧版本）：**

在 `windows/CMakeLists.txt` 第28行添加 `GLOBAL` 关键字：
```cmake
# 修改前
add_library(${PLUGIN_NAME} SHARED IMPORTED)

# 修改后
add_library(${PLUGIN_NAME} SHARED IMPORTED GLOBAL)
```

### 问题3：找不到 any_w_p_engine_plugin.h

**错误信息：** `fatal error: anywp_engine/any_w_p_engine_plugin.h: No such file or directory`

**解决方案：**

预编译包 v2.1.3+ 已自动包含此包装头文件。

**手动修复（旧版本）：**

创建文件 `include/anywp_engine/any_w_p_engine_plugin.h`：
```c
// Wrapper header for Flutter's generated plugin registrant
#ifndef ANY_W_P_ENGINE_PLUGIN_H_
#define ANY_W_P_ENGINE_PLUGIN_H_
#include "anywp_engine_plugin_c_api.h"
#endif  // ANY_W_P_ENGINE_PLUGIN_H_
```

### 问题4：pubspec.yaml asset 错误

**错误信息：** `No file or variants found for asset: packages/anywp_engine/windows/anywp_sdk.js`

**解决方案：**

预编译包 v2.1.3+ 的 `pubspec.yaml` 已移除不存在的 asset 引用（SDK 由 C++ 插件自动注入，无需打包为 asset）。

**手动修复（旧版本）：**

从 `pubspec.yaml` 中移除或注释掉以下行：
```yaml
flutter:
  # assets:
  #   - windows/anywp_sdk.js  # SDK 由 C++ 插件自动注入，无需打包
```

### 问题5：ZIP 解压后文件夹嵌套

**问题描述：** 解压后需要手动调整文件夹结构

**解决方案：**

预编译包 v2.1.3+ 已修复打包方式，解压后直接就是正确的文件夹结构，无需手动调整。

**验证：**

运行预编译包根目录的 `verify.bat` 脚本，检查所有必需文件是否存在：
```bash
cd packages\anywp_engine_v2.1.5
verify.bat
```

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

- [ ] 下载预编译包 (`anywp_engine_v2.1.10_precompiled.zip`)
- [ ] 解压到项目的 `packages/anywp_engine` 目录
- [ ] 在 `pubspec.yaml` 中添加依赖
- [ ] 运行 `flutter pub get`
- [ ] 运行 `flutter build windows` 测试构建
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
- **API 参考**：[DEVELOPER_API_REFERENCE.md](DEVELOPER_API_REFERENCE.md)
- **Web 开发指南**：[WEB_DEVELOPER_GUIDE_CN.md](WEB_DEVELOPER_GUIDE_CN.md)
- **使用示例**：[API_USAGE_EXAMPLES.md](API_USAGE_EXAMPLES.md)
- **发布页面**：[GitHub Releases](https://github.com/zhaibin/AnyWallpaper-Engine/releases)

---

## 🤝 获取帮助

遇到问题？

- 📖 首先查看本文档的 [常见问题](#-常见问题) 部分
- 🐛 提交 [GitHub Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论区](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

---

**Version**: 2.3.2  
**Updated**: 2025-11-18  
**主要变更**:
- ✅ **WorkerW 自动恢复** - 支持 Explorer 重启后自动重建壁纸
- ✅ **Lively 风格优化** - 优化 WorkerW 创建和查找策略
- ✅ **Windows 11 兼容** - 支持 Raised Desktop 模式
- ✅ **修复 verify.bat 脚本** - 解决预编译包验证脚本执行错误
- ✅ **集成文档包含在 ZIP 包中** - `INTEGRATION_GUIDE.md` 随包提供
- ✅ **完善集成文档** - 详细说明预编译专用 CMakeLists.txt 的特点
- ✅ **新增纯 C API 头文件支持** - 无 C++ 依赖
- ✅ **创建独立的预编译包和源码包** - 简化集成流程
- ✅ **Web SDK 包含在预编译包中** - `sdk/anywp_sdk.js` 和 `examples/` 目录🆕
- ✅ **Web 开发者指南包含在预编译包中** - 中英文文档🆕

