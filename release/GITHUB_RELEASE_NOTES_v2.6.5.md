# AnyWP Engine v2.6.5 - 紧急修复

## 🐛 Bug 修复

### 修复 setGlobalAllowedAccessPath 无限递归崩溃

**问题**: `setGlobalAllowedAccessPath:` 方法中使用 `self.globalAllowedAccessPath = path` 导致无限递归调用 setter，最终栈溢出崩溃（递归 104478 次）。

**修复**: 使用实例变量 `_globalAllowedAccessPath` 直接赋值，避免触发 setter 递归。

```objc
// 修复前 (无限递归)
self.globalAllowedAccessPath = path;

// 修复后 (直接赋值)
_globalAllowedAccessPath = path;
```

### 同步版本号

- 更新 `anywp_engine.podspec` 版本号到 2.6.5
- 更新 `macos/CMakeLists.txt` 版本号到 2.6.5
- Framework Info.plist 现在正确显示 2.6.5

## 📦 下载

| 包名 | 说明 | 适用场景 |
|------|------|----------|
| `anywp_engine_macos_v2.6.5_precompiled.zip` | macOS 预编译包 | Flutter 开发者（推荐） |
| `anywp_engine_macos_v2.6.5_source.zip` | macOS 源码包 | 需要自定义修改的开发者 |
| `anywp_web_sdk_v2.5.0.zip` | Web SDK 包 | HTML 壁纸开发者 |

## ⚠️ 升级建议

**强烈建议从 v2.6.4 升级到 v2.6.5**，v2.6.4 存在严重的崩溃 bug。

## 📋 完整更新日志

详见 [CHANGELOG_CN.md](https://github.com/zhaibin/AnyWallpaper-Engine/blob/main/CHANGELOG_CN.md)

---

**Full Changelog**: https://github.com/zhaibin/AnyWallpaper-Engine/compare/v2.6.4...v2.6.5

