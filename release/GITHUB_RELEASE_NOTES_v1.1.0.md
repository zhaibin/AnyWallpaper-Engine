# AnyWP Engine v1.1.0 - 预编译版本发布 🎉

**重大更新！** 现在支持预编译 DLL 包，Flutter 开发者无需安装 WebView2 SDK 即可快速集成！

---

## 📦 下载

### ⭐ 推荐：预编译 DLL 包

**文件名**：`anywp_engine_v1.1.0.zip`  
**大小**：~220 KB  

**包含内容**：
- ✅ 预编译 DLL（`anywp_engine_plugin.dll`, `WebView2Loader.dll`）
- ✅ Dart 源代码
- ✅ JavaScript SDK
- ✅ 头文件（C++ API）
- ✅ 使用文档

**优势**：
- 🚀 无需编译，快速集成
- 🛠️ 无需安装 WebView2 SDK
- 💻 无需安装 Visual Studio
- ⚡ 减少构建时间

---

## 🚀 快速开始（5分钟）

### 1. 下载并解压

下载 `anywp_engine_v1.1.0.zip` 并解压到你的 Flutter 项目根目录

### 2. 添加依赖

在 `pubspec.yaml` 中添加：

```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.1.0
```

### 3. 获取依赖并构建

```bash
flutter pub get
flutter build windows
```

### 4. 开始使用

```dart
import 'package:anywp_engine/anywp_engine.dart';

// 启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// 停止壁纸
await AnyWPEngine.stopWallpaper();
```

**完成！** 就这么简单！

---

## ✨ 主要更新（v1.1.0）

### 🆕 新增功能

#### 预编译 DLL 支持
- **快速集成**：提供预编译的 DLL 包，无需安装 WebView2 SDK
- **自动化构建**：新增 `build_release.bat` 脚本，一键生成 Release 包
- **完整打包**：包含 DLL、头文件、Dart 源码、JavaScript SDK
- **GitHub Release**：支持作为 GitHub Release 发布

#### 文档完善
- **集成指南**：新增 `PRECOMPILED_DLL_INTEGRATION.md` 详细说明预编译 DLL 使用方法
- **发布指南**：新增 `RELEASE_GUIDE.md` 说明如何构建和发布版本
- **Release 模板**：新增 `RELEASE_TEMPLATE.md` 作为 GitHub Release 说明模板
- **更新现有文档**：在 `PACKAGE_USAGE_GUIDE_CN.md` 和 `FOR_FLUTTER_DEVELOPERS.md` 中添加预编译 DLL 方式

### 🎨 拖拽支持与状态持久化（v4.2.0 功能）

#### 元素拖拽
- **任意元素拖拽**：JavaScript SDK 新增 `makeDraggable()` 方法
- **拖拽回调**：支持 `onDragStart`, `onDrag`, `onDragEnd`
- **边界限制**：支持设置拖拽边界
- **性能优化**：拖拽操作流畅无卡顿

#### 状态持久化
- **自动保存**：拖拽后的元素位置自动保存到 Windows Registry
- **自动恢复**：重新打开壁纸后自动恢复到上次的位置
- **通用存储**：支持保存任意键值对数据
- **跨会话**：状态在应用重启后依然保留

### ⚡ 省电优化与即时恢复（v4.1.0 功能）

#### 轻量级暂停策略
- **即时恢复**：从 500-1000ms 优化到 **<50ms** ⚡
- **状态保留**：DOM 完全保留，无需重新加载
- **WebView2 优化**：使用 `put_IsVisible(FALSE)` 而非隐藏窗口
- **用户体验**：解锁后壁纸立即显示，仿佛从未暂停

---

## 📚 完整文档

### 快速开始
- [预编译 DLL 集成指南](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PRECOMPILED_DLL_INTEGRATION.md) ⭐
- [快速开始指南](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/QUICK_START.md)
- [打包使用指南](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/PACKAGE_USAGE_GUIDE_CN.md)

### API 文档
- [开发者 API 参考](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/DEVELOPER_API_REFERENCE.md)
- [API 使用示例](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/API_USAGE_EXAMPLES.md)

### 进阶指南
- [最佳实践](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/BEST_PRACTICES.md)
- [Web 开发者指南](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/WEB_DEVELOPER_GUIDE_CN.md)
- [故障排除](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/docs/TROUBLESHOOTING.md)

---

## 🎓 核心 API

### Dart API（Flutter）

```dart
// 启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// 多显示器支持
List<MonitorInfo> monitors = await AnyWPEngine.getMonitors();
await AnyWPEngine.initializeWallpaperOnMonitor(url: 'https://example.com', monitorIndex: 0);

// 省电控制
await AnyWPEngine.pauseWallpaper();
await AnyWPEngine.resumeWallpaper();
await AnyWPEngine.setAutoPowerSaving(true);

// 状态管理
await AnyWPEngine.saveState('key', 'value');
String value = await AnyWPEngine.loadState('key');
```

### JavaScript SDK（壁纸网页）

```html
<!-- 引入 SDK -->
<script src="../windows/anywp_sdk.js"></script>

<script>
// 元素拖拽
AnyWP.makeDraggable('#draggable-element', {
  persistKey: 'element_pos',
  boundary: { left: 0, top: 0, right: 800, bottom: 600 }
});

// 保存/加载状态
AnyWP.saveState('myKey', 'myValue');
AnyWP.loadState('myKey').then(value => {
  console.log('Loaded:', value);
});
</script>
```

---

## 📋 系统要求

- **操作系统**：Windows 10 或 Windows 11
- **Flutter**：3.0.0 或更高版本
- **Dart**：3.0.0 或更高版本
- **WebView2 Runtime**：Windows 11 自带，Windows 10 需要 [下载安装](https://developer.microsoft.com/microsoft-edge/webview2/)

---

## 🔄 从旧版本升级

### 从 v1.0.x 升级

1. 下载新版本 `anywp_engine_v1.1.0.zip`
2. 删除旧版本目录
3. 解压新版本并更新 `pubspec.yaml` 中的路径
4. 运行 `flutter clean && flutter pub get && flutter build windows`

**破坏性更改**：✅ 无破坏性更改，完全兼容 v1.0.x

---

## 🎯 集成方式对比

| 方式 | 优点 | 适用场景 |
|------|------|---------|
| **预编译 DLL** ⭐ | 无需编译、快速集成 | 生产环境、快速开发 |
| **Git 引用** | 版本追踪、便于共享 | 团队协作 |
| **本地路径** | 即时修改、简单快速 | 开发测试 |

---

## 🐛 已知问题

- 在某些低端设备上，WebView2 首次加载可能较慢（正常现象）
- Windows 10 用户需要手动安装 WebView2 Runtime

---

## 🙏 致谢

感谢所有贡献者和用户的支持！

---

## 📞 获取帮助

- 📖 查看 [完整文档](https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs)
- 🐛 提交 [Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

---

**发布日期**：2025-11-05  
**版本号**：v1.1.0  
**许可证**：MIT License

