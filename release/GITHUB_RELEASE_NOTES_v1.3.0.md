# AnyWP Engine v1.3.0 - 会话切换与多显示器稳定性大幅提升

## 📦 发布包说明

本次发布提供 **两个独立的包**，请根据您的需求下载：

### 1️⃣ anywp_engine_v1.3.0.zip（标准 Flutter 包）
**适用于**：Flutter/Dart 开发者，需要将 AnyWP Engine 集成到 Flutter 应用中
- ✅ 预编译 DLL + LIB（无需 Visual Studio）
- ✅ 完整的 Dart API + C++ 头文件
- ✅ CMake 集成文件 + 自动化安装脚本
- ✅ 最小示例项目 + 完整文档

**快速开始**：
```bash
# 解压到 Flutter 项目的 packages/ 目录
cd your_flutter_project/packages
unzip anywp_engine_v1.3.0.zip

# 运行自动安装脚本
cd anywp_engine_v1.3.0
setup_precompiled.bat
```

### 2️⃣ anywp_web_sdk_v1.3.0.zip（Web 开发者专用包）
**适用于**：Web 前端开发者，只需要开发 HTML/JS 壁纸页面
- ✅ JavaScript SDK（anywp_sdk.js）
- ✅ 8个完整示例（拖拽、点击、React、Vue 等）
- ✅ Web 开发者文档（中英双语）
- ✅ 轻量级（无 DLL，仅 SDK + 文档）

**快速开始**：
```html
<script src="anywp_sdk.js"></script>
<script>
    // 使元素可拖拽
    window.AnyWP.makeDraggable('element-id');
    
    // 点击检测
    window.AnyWP.onClick('element-id', function() {
        console.log('Clicked!');
    });
</script>
```

---

## 🎯 重大改进

### 🖥️ 跨会话稳定性（核心特性）

经过 **15个迭代修复**，AnyWP Engine 现在完全支持以下场景：

#### ✅ 自动处理的场景
- **远程桌面切换**：主机 ↔ 远程桌面切换时自动重建壁纸，保持持续可见
- **锁屏智能恢复**：锁屏后再解锁，壁纸自动恢复；跨会话锁屏也能正确重建
- **多显示器适配**：显示器数量不同时，自动跳过不可用的显示器
- **设备名称映射**：即使显示器枚举顺序改变，也能正确恢复到对应的物理显示器

#### 🔋 自动优化的行为
- **锁屏时**：壁纸自动暂停，停止动画、视频、音频，节省电量
- **解锁时**：同会话直接恢复动画，跨会话重建壁纸
- **会话切换时**：自动检测当前会话的显示器配置，重建壁纸到正确的物理显示器

#### ✅ 测试验证（全部通过）

**会话切换场景**：
- ✅ 主机锁屏 → 主机解锁（动画暂停再继续）
- ✅ 远程桌面锁屏 → 远程桌面解锁（动画暂停再继续）
- ✅ 主机锁屏 → 远程桌面解锁（壁纸重建）
- ✅ 远程桌面锁屏 → 主机解锁（壁纸重建）
- ✅ 主机不锁屏 → 远程桌面进入（壁纸重建）
- ✅ 远程桌面不锁屏 → 主机进入（壁纸重建）

**多显示器场景**：
- ✅ 主机2显示器 → 远程1显示器 → 主机（所有显示器恢复）
- ✅ 显示器枚举顺序改变（设备名称映射正确）
- ✅ 多次往返切换（无资源泄漏）

---

## 🔧 技术细节

### 会话管理系统重构

**核心机制**：
- 使用 atomic 标志 (`is_session_locked_`, `is_remote_session_`) 实时追踪系统状态
- 统一决策函数 `ShouldWallpaperBeActive()` 基于锁屏状态决定壁纸是否应该激活
- 解锁时自动检测 `wallpaper_instances_` 是否为空，决定是普通恢复还是强制重建

**事件处理**：
- `WTS_SESSION_LOCK` → 暂停壁纸
- `WTS_SESSION_UNLOCK` → 检测是否需要重建，然后恢复或重建
- `WTS_CONSOLE_CONNECT/DISCONNECT` → 强制重建（跨会话窗口不可见）
- `WTS_REMOTE_CONNECT/DISCONNECT` → 强制重建

### 多显示器持久化架构

**使用设备名称而非索引**：
```cpp
// 保存配置时使用稳定的设备名称
std::vector<std::string> original_monitor_devices_;  // ["\\.\DISPLAY1", "\\.\DISPLAY2"]
```

**恢复时动态映射**：
1. 重新枚举当前会话的显示器（`GetMonitors()`）
2. 将设备名称映射到当前索引
3. 只在存在的显示器上初始化

---

## 🐛 修复的关键问题

### 15个迭代修复历程

1. ✅ **远程桌面壁纸消失**：允许远程会话运行壁纸，切换时强制重建
2. ✅ **锁屏后壁纸不恢复**：锁屏时暂停，解锁时检测是否需要重建
3. ✅ **WebView2 初始化失败**：避免连续两次 `StopWallpaper()` 导致 COM 对象冲突
4. ✅ **副显示器消失**：持久化保存所有显示器配置，重建时恢复所有显示器
5. ✅ **显示器索引混乱**：使用设备名称而非索引标识，解决枚举顺序不一致问题
6. ✅ **跨会话锁屏场景**：正确处理远程桌面锁屏 ↔ 主机解锁
7. ✅ **多显示器尺寸错误**：重建前重新枚举显示器，确保使用当前配置
8. ✅ **主显示器消失**：设备名称映射确保壁纸恢复到正确的物理显示器
9. ✅ **资源泄漏**：统一清理所有 `wallpaper_instances_`，防止窗口泄漏
10. ✅ **URL丢失**：保留 `default_wallpaper_url_` 供会话切换重建使用
11. ✅ **环境无效**：清空 `shared_environment_` 强制重新创建 WebView2 环境
12. ✅ **初始化模式不一致**：统一使用 `InitializeWallpaperOnMonitor` 多显示器模式
13. ✅ **双重停止错误**：避免连续 `StopWallpaper()` 导致 `E_ABORT`
14. ✅ **锁屏重建错误**：会话切换时检查 `ShouldWallpaperBeActive()` 避免在锁屏时重建
15. ✅ **窗口验证失败**：会话切换时强制重建，绕过跨会话窗口验证

---

## 🚀 快速开始

### 使用预编译 DLL（推荐）

**1. 下载并解压**
```bash
# 下载 anywp_engine_v1.3.0.zip 并解压到项目根目录
```

**2. 在 pubspec.yaml 中添加**
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.3.0
```

**3. 开始使用**
```dart
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // 推荐：设置应用名称以隔离存储
  await AnyWPEngine.setApplicationName('MyAwesomeApp');
  
  runApp(MyApp());
}

// 启动壁纸（自动适配会话切换和多显示器）
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// 多显示器支持（自动适配会话切换）
final monitors = await AnyWPEngine.getMonitors();
for (var monitor in monitors) {
  await AnyWPEngine.initializeWallpaperOnMonitor(
    url: 'file:///path/to/wallpaper.html',
    enableMouseTransparent: true,
    monitorIndex: monitor['index'],
  );
}
```

**完成！** 所有会话切换和多显示器场景都会自动处理，无需任何额外代码。

---

## 📥 下载

### 预编译 DLL 包（推荐）

**文件名**：`anywp_engine_v1.3.0.zip`  
**大小**：~2MB  

**包含内容**：
- ✅ 预编译 DLL（`anywp_engine_plugin.dll`, `WebView2Loader.dll`）
- ✅ 链接库（`anywp_engine_plugin.lib`）
- ✅ Dart 源代码（标准位置 `lib/anywp_engine.dart`）
- ✅ JavaScript SDK（`sdk/anywp_sdk.js`）
- ✅ 头文件（C++ API）
- ✅ CMake 配置
- ✅ 完整文档

---

## 🔄 从旧版本升级

### 从 v1.2.x 升级

✅ **完全向后兼容**：无需修改现有代码  
✅ **自动获益**：会话切换和多显示器功能自动生效  
✅ **零配置**：所有优化都在插件层自动处理

**升级步骤**：
```bash
# 1. 删除旧版本
rmdir /s /q anywp_engine_v1.2.1

# 2. 下载并解压 v1.3.0

# 3. 更新引用
# pubspec.yaml:
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.3.0  # 更新版本号

# 4. 重新构建
flutter clean
flutter pub get
flutter build windows
```

### 破坏性更改

- ✅ **无破坏性更改**：v1.3.0 完全兼容 v1.2.x

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
- [Flutter 开发者导航](docs/FOR_FLUTTER_DEVELOPERS.md) 🆕

### API 文档
- [开发者 API 参考](docs/DEVELOPER_API_REFERENCE.md)
- [API 使用示例](docs/API_USAGE_EXAMPLES.md)
- [Web 开发者指南](docs/WEB_DEVELOPER_GUIDE_CN.md) 🆕

### 进阶指南
- [最佳实践](docs/BEST_PRACTICES.md)
- [故障排除](docs/TROUBLESHOOTING.md)

---

## ⚠️ 重要说明

### 系统要求
- **Windows 10/11**：完整支持所有会话切换场景
- **WebView2 Runtime**：确保安装最新版本

### 已知限制
- **远程桌面显示器数量**：如果远程桌面显示器数量少于主机，部分显示器会被跳过（正常行为）
- **DPI 缩放**：不同会话的 DPI 设置可能影响壁纸尺寸，但会自动适配

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

特别感谢所有测试用户在远程桌面和多显示器场景下的详细反馈，帮助我们完成了15个迭代修复！

---

**发布日期**：2025-11-07  
**版本号**：v1.3.0  
**Changelog**：4.6.0

---

## 📞 获取帮助

- 📖 查看 [完整文档](https://github.com/zhaibin/AnyWallpaper-Engine/tree/main/docs)
- 🐛 提交 [Issue](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- 💬 参与 [讨论](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)
- 📧 联系作者：[GitHub @zhaibin](https://github.com/zhaibin)

