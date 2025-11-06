# AnyWP Engine v1.2.1 - 预编译包体验全面升级

**发布日期**: 2025-11-06  
**版本**: 1.2.1 (Changelog: 4.5.0)

---

## 🎉 发布亮点

### 1. 📦 预编译包结构标准化
- ✅ `lib/anywp_engine.dart` 与 `lib/anywp_engine_plugin.lib` 位于标准路径
- ✅ `windows/anywp_sdk.js` 与 `windows/src/`（C++ 源码）同包发布
- ✅ `CMakeLists.txt` 自动检测预编译 / 源码两种模式

```text
anywp_engine_v1.2.1/
├── bin/anywp_engine_plugin.dll
├── lib/anywp_engine_plugin.lib
├── lib/anywp_engine.dart
├── windows/anywp_sdk.js
└── windows/src/anywp_engine_plugin.cpp
```

### 2. ⚙️ 一键集成脚本
- `setup_precompiled.bat`：自动验证 + 安装 + flutter pub get
- `verify_precompiled.bat`：检查 8 个关键文件是否齐全
- `generate_pubspec_snippet.bat`：输出 `pubspec.yaml` 依赖片段
- `example_minimal/`：最小可运行示例，5 分钟内完成验证

### 3. 🧭 版本检测 API
- 新增 `AnyWPEngine.getPluginVersion()` 与 `AnyWPEngine.isCompatible()`
- 在应用启动时快速判断依赖版本是否满足要求

```dart
final version = await AnyWPEngine.getPluginVersion();
final compatible = await AnyWPEngine.isCompatible(expectedPrefix: '1.2.');
if (!compatible) {
  throw Exception('AnyWP Engine version mismatch: $version');
}
```

### 4. 🛠️ 更友好的错误提示
- WebView2 初始化失败时，主动提示安装运行时与官方下载地址

---

## 🔧 技术细节

- `scripts/build_release_v2.bat`：
  - 复制 C++ 源码与 NuGet 依赖，确保源码构建路径完整
  - 引入模板渲染系统，自动生成脚本、示例与文档
  - 引入关键文件校验（.dll/.lib/.dart/.js/CMake/headers）
- `windows/CMakeLists.txt`：自动检测预编译 DLL，缺失时回退源码编译
- `lib/anywp_engine.dart`：新增版本 API，并提供兼容性检查
- `windows/anywp_engine_plugin.cpp`：新增 `getVersion` 方法与错误提示

---

## 🚀 升级指南

1. 从 GitHub Releases 下载 `anywp_engine_v1.2.1.zip`
2. 解压到 `packages/anywp_engine_v1.2.1/`
3. 在 Flutter 项目根目录执行：

```powershell
packages\anywp_engine_v1.2.1\setup_precompiled.bat
```

4. 运行最小示例验证：

```bash
cd packages/anywp_engine_v1.2.1/example_minimal
flutter pub get
flutter run -d windows
```

---

## ✅ 验证清单

### 预编译包完整性

```powershell
packages\anywp_engine_v1.2.1\verify_precompiled.bat
```

### 手动检查（全部返回 True）

```powershell
Test-Path "packages/anywp_engine_v1.2.1/bin/anywp_engine_plugin.dll"
Test-Path "packages/anywp_engine_v1.2.1/bin/WebView2Loader.dll"
Test-Path "packages/anywp_engine_v1.2.1/lib/anywp_engine_plugin.lib"
Test-Path "packages/anywp_engine_v1.2.1/lib/anywp_engine.dart"
Test-Path "packages/anywp_engine_v1.2.1/lib/dart/anywp_engine.dart"
Test-Path "packages/anywp_engine_v1.2.1/windows/anywp_sdk.js"
Test-Path "packages/anywp_engine_v1.2.1/windows/CMakeLists.txt"
```

### 构建验证

```bash
flutter pub get
flutter build windows --release
```

---

## 🧾 相关文档

- [CHANGELOG_CN.md](../CHANGELOG_CN.md)
- [PRECOMPILED_README.md](../release/anywp_engine_v1.2.1/PRECOMPILED_README.md)（解压后）
- [docs/PRECOMPILED_DLL_INTEGRATION.md](../docs/PRECOMPILED_DLL_INTEGRATION.md)

---

感谢所有集成反馈，本次版本专注于“解压即用”的体验，让预编译包安装时间从 2~3 小时降至 5~10 分钟。欢迎继续提交建议，帮助我们把 AnyWP Engine 打磨得更好！

