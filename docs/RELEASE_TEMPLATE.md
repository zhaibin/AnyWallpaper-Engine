# AnyWP Engine v1.1.0 - Release 说明

## 📦 预编译版本 - 快速集成，无需编译

**适用于**：希望快速集成 AnyWP Engine 的 Flutter 开发者，无需安装 WebView2 SDK 或 Visual Studio。

---

## 🎯 主要更新

### ✨ 新功能

- 🖱️ **元素拖拽支持**：JavaScript SDK 支持任意元素拖拽，带边界限制
- 💾 **状态持久化**：拖拽位置自动保存到注册表，重启后恢复
- ⚡ **即时恢复优化**：解锁恢复时间 <50ms，状态完全保留
- 🔋 **智能省电**：支持自动暂停/恢复，延长电池寿命
- 📺 **多显示器支持**：完整支持多显示器独立壁纸

### 🐛 修复

- 修复鼠标钩子在某些情况下的性能问题
- 修复 WebView2 内存泄漏
- 改进窗口层级管理

### 📚 文档

- 新增预编译 DLL 集成指南
- 完善 API 文档和示例
- 新增故障排除指南

---

## 📥 下载

### 预编译 DLL 包（推荐）

**文件名**：`anywp_engine_v1.1.0.zip`  
**大小**：~2MB  
**SHA256**：`[构建后自动生成]`

**包含内容**：
- ✅ 预编译 DLL（`anywp_engine_plugin.dll`, `WebView2Loader.dll`）
- ✅ Dart 源代码
- ✅ JavaScript SDK
- ✅ 头文件（C++ API）
- ✅ 使用文档

---

## 🚀 快速开始

### 方式一：使用预编译 DLL（推荐）

**1. 下载并解压**
```bash
# 下载 anywp_engine_v1.1.0.zip 并解压到项目根目录
```

**2. 在 pubspec.yaml 中添加**
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.1.0
```

**3. 获取依赖并构建**
```bash
flutter pub get
flutter build windows
```

**4. 开始使用**
```dart
import 'package:anywp_engine/anywp_engine.dart';

await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);
```

**完成！** 无需安装 WebView2 SDK 或 Visual Studio。

---

### 方式二：从 Git 引用（适合开发者）

```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
      ref: v1.1.0
```

> ⚠️ 需要安装 WebView2 SDK，运行 `scripts\setup_webview2.bat`

---

## 📋 系统要求

- **操作系统**：Windows 10 或 Windows 11
- **Flutter**：3.0.0 或更高版本
- **Dart**：3.0.0 或更高版本
- **WebView2 Runtime**：Windows 11 自带，Windows 10 需要 [下载安装](https://developer.microsoft.com/microsoft-edge/webview2/)

---

## 📚 完整文档

### 快速开始
- [预编译 DLL 集成指南](docs/PRECOMPILED_DLL_INTEGRATION.md) ⭐
- [快速开始指南](docs/QUICK_START.md)
- [打包使用指南](docs/PACKAGE_USAGE_GUIDE_CN.md)

### API 文档
- [开发者 API 参考](docs/DEVELOPER_API_REFERENCE.md)
- [API 使用示例](docs/API_USAGE_EXAMPLES.md)

### 进阶指南
- [最佳实践](docs/BEST_PRACTICES.md)
- [Web 开发者指南](docs/WEB_DEVELOPER_GUIDE_CN.md)
- [故障排除](docs/TROUBLESHOOTING.md)

---

## 🎓 核心 API

### Dart API（Flutter）

```dart
// 启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// 停止壁纸
await AnyWPEngine.stopWallpaper();

// 导航到新 URL
await AnyWPEngine.navigateToUrl('https://new-url.com');

// 获取显示器列表
List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();

// 在指定显示器启动壁纸
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'https://example.com',
  monitorIndex: 0,
);

// 省电控制
await AnyWPEngine.pauseWallpaper();        // 暂停
await AnyWPEngine.resumeWallpaper();       // 恢复
await AnyWPEngine.setAutoPowerSaving(true); // 自动省电

// 状态管理
await AnyWPEngine.saveState('key', 'value');
String value = await AnyWPEngine.loadState('key');
```

### JavaScript SDK（壁纸网页）

```html
<!-- 引入 SDK -->
<script src="../windows/anywp_sdk.js"></script>

<script>
// 设置交互模式
AnyWP.setInteractionMode('interactive');  // 或 'transparent'

// 元素拖拽
AnyWP.makeDraggable('#draggable-element', {
  persistKey: 'element_pos',
  boundary: { left: 0, top: 0, right: 800, bottom: 600 }
});

// 复位位置
AnyWP.resetPosition('#element', { left: 100, top: 100 });

// 保存状态
AnyWP.saveState('myKey', 'myValue');

// 加载状态
AnyWP.loadState('myKey').then(value => {
  console.log('Loaded:', value);
});
</script>
```

---

## 🔄 从旧版本升级

### 从 v1.0.x 升级

**1. 下载新版本**
```bash
# 下载 anywp_engine_v1.1.0.zip
```

**2. 删除旧版本**
```bash
rmdir /s /q anywp_engine_v1.0.0
```

**3. 解压新版本并更新引用**
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.1.0  # 更新版本号
```

**4. 重新构建**
```bash
flutter clean
flutter pub get
flutter build windows
```

### 破坏性更改

- ✅ **无破坏性更改**：v1.1.0 完全兼容 v1.0.x

---

## 🎯 示例代码

### 完整示例项目

查看 `example/` 目录获取完整的示例应用。

### 测试 HTML

项目包含多个测试 HTML 文件：
- `examples/test_simple.html` - 简单壁纸
- `examples/test_draggable.html` - 拖拽功能测试
- `examples/test_api.html` - 完整 API 测试
- `examples/test_react.html` - React 集成示例

---

## 🐛 已知问题

- 在某些低端设备上，WebView2 首次加载可能较慢（正常现象）
- Windows 10 用户需要手动安装 WebView2 Runtime

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

- **报告问题**：[GitHub Issues](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- **功能建议**：[GitHub Discussions](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- **参与开发**：Fork 项目并提交 PR

---

## 📜 许可证

本项目基于 [MIT License](LICENSE) 开源。

---

## 🙏 致谢

感谢所有贡献者和用户的支持！

---

**发布日期**：2025-11-05  
**版本号**：v1.1.0  
**下载次数**：[自动统计]

---

## 📞 获取帮助

- 📖 查看 [完整文档](https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs)
- 🐛 提交 [Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- 📧 联系作者：[GitHub @zhaibin](https://github.com/zhaibin)

