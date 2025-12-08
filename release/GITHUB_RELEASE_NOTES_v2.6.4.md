# AnyWP Engine v2.6.4 - macOS File Access Control

## 🎯 核心功能

### 扩展文件访问范围 (macOS)

**问题**: WKWebView 的 `loadFileURL:allowingReadAccessToURL:` 默认只授权文件所在目录的访问权限，导致 HTML 无法加载其他子目录的资源。

**解决方案**: 
- 默认将授权范围扩展到 `~/Library` 目录
- 支持自定义全局授权路径
- 支持单个壁纸级别的授权路径

## ✨ 新增 API (macOS)

```dart
// 设置全局文件授权路径
await AnyWPEngine.setAllowedAccessPath('/path/to/directory');

// 获取 Library 目录路径
final libraryPath = await AnyWPEngine.getDefaultLibraryPath();

// 获取 Application Support 路径
final appSupportPath = await AnyWPEngine.getApplicationSupportPath();

// initializeWallpaperOnMonitor 新增 allowedAccessPath 参数
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file:///path/to/wallpaper.html',
  monitorIndex: 0,
  allowedAccessPath: '/path/to/authorize', // 新增
);
```

## 📦 下载

| 包名 | 说明 | 适用场景 |
|------|------|----------|
| `anywp_engine_macos_v2.6.4_precompiled.zip` | macOS 预编译包 | Flutter 开发者（推荐） |
| `anywp_engine_macos_v2.6.4_source.zip` | macOS 源码包 | 需要自定义修改的开发者 |
| `anywp_web_sdk_v2.5.0.zip` | Web SDK 包 | HTML 壁纸开发者 |

## 🛠️ 技术改进

### WallpaperManager.h/m
- 新增 `globalAllowedAccessPath` 属性存储全局授权路径
- 新增 `setGlobalAllowedAccessPath:` 方法设置全局授权路径
- 新增 `determineAllowedAccessURL:forFileURL:` 方法智能确定授权路径
- 新增 `defaultLibraryPath` 和 `applicationSupportPath` 类方法

### AnyWPEnginePlugin.m
- 新增 `handleSetAllowedAccessPath:` 方法处理
- 新增 `handleGetDefaultLibraryPath:` 方法处理
- 新增 `handleGetApplicationSupportPath:` 方法处理

### anywp_engine.dart
- `initializeWallpaperOnMonitor()` 新增 `allowedAccessPath` 可选参数
- 新增 `setAllowedAccessPath()` 方法
- 新增 `getDefaultLibraryPath()` 方法
- 新增 `getApplicationSupportPath()` 方法

## 📖 使用示例

```dart
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // macOS: 设置文件访问授权路径
  if (Platform.isMacOS) {
    final libraryPath = await AnyWPEngine.getDefaultLibraryPath();
    await AnyWPEngine.setAllowedAccessPath(libraryPath);
  }
  
  runApp(MyApp());
}

// 或者针对单个壁纸指定授权路径
final appSupportPath = await AnyWPEngine.getApplicationSupportPath();
await AnyWPEngine.initializeWallpaperOnMonitor(
  url: 'file://$appSupportPath/MyApp/wallpaper.html',
  monitorIndex: 0,
  allowedAccessPath: '$appSupportPath/MyApp',
);
```

## 🧪 测试验证

- ✅ macOS Debug/Release 编译成功
- ✅ 应用正常启动和运行
- ✅ 壁纸正常初始化和显示
- ✅ 多显示器支持正常

## 📋 完整更新日志

详见 [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md)

---

**Full Changelog**: https://github.com/zhaibin/AnyWallpaper-Engine/compare/v2.6.3...v2.6.4

