# Migration to AnyWP Naming Convention

## ✅ 完成总结

项目已成功从 `anywallpaper` 迁移到 `anywp` 命名规范。

### 🎯 命名规范统一

| 项目 | 旧命名 | 新命名 | 状态 |
|------|--------|--------|------|
| **包名** | `anywallpaper_engine` | `anywp_engine` | ✅ |
| **Dart 类** | `AnyWallpaperEngine` | `AnyWPEngine` | ✅ |
| **C++ 插件类** | `AnyWallpaperEnginePlugin` | `AnyWPEnginePlugin` | ✅ |
| **主文件** | `anywallpaper_engine_plugin.cpp` | `anywp_engine_plugin.cpp` | ✅ |
| **SDK文件** | `anywallpaper_sdk.js` | `anywp_sdk.js` | ✅ |
| **Include目录** | `anywallpaper_engine/` | `anywp_engine/` | ✅ |
| **命名空间** | `anywallpaper_engine` | `anywp_engine` | ✅ |
| **日志前缀** | `[AnyWallpaper]` | `[AnyWP]` | ✅ |

### 📁 文件结构变化

#### 重命名的文件

```
lib/
  anywallpaper_engine.dart → anywp_engine.dart

windows/
  anywallpaper_engine_plugin.cpp → anywp_engine_plugin.cpp
  anywallpaper_engine_plugin.h → anywp_engine_plugin.h
  anywallpaper_sdk.js → anywp_sdk.js
  
  include/
    anywallpaper_engine/ → anywp_engine/
      anywallpaper_engine_plugin.h → anywp_engine_plugin.h
      anywallpaper_engine_plugin_c_api.h → anywp_engine_plugin_c_api.h
```

#### Flutter 兼容副本

为兼容 Flutter 自动生成代码，保留了副本：
```
windows/include/anywp_engine/
  ├── anywp_engine_plugin.h           (主头文件)
  ├── anywp_engine_plugin_c_api.h     (C API)
  ├── any_w_p_engine_plugin.h         (Flutter 兼容副本)
  └── any_w_p_engine_plugin_c_api.h   (Flutter 兼容副本)
```

### 🔧 代码更新

#### pubspec.yaml

```yaml
name: anywp_engine  # 从 anywallpaper_engine 更新
description: AnyWP - Flutter plugin for WebView2 desktop wallpaper engine

flutter:
  plugin:
    platforms:
      windows:
        pluginClass: AnyWPEnginePlugin  # 从 AnyWallpaperEnginePlugin 更新
        fileName: anywp_engine_plugin.cpp  # 从 anywallpaper_engine_plugin.cpp 更新
  
  assets:
    - windows/anywp_sdk.js  # 从 anywallpaper_sdk.js 更新
```

#### Dart API

```dart
import 'package:anywp_engine/anywp_engine.dart';  // 包名更新

class AnyWPEngine {  // 类名从 AnyWallpaperEngine 更新
  static const MethodChannel _channel = MethodChannel('anywp_engine');  // 通道名更新
  // ...
}
```

#### example/lib/main.dart

```dart
import 'package:anywp_engine/anywp_engine.dart';  // 导入更新

await AnyWPEngine.initializeWallpaper(...);  // 类名更新
```

### 📖 文档更新

所有文档已更新为使用 `AnyWP` 命名：
- ✅ README.md
- ✅ docs/*.md (所有文档)
- ✅ scripts/*.bat
- ✅ examples/*.html

### ⚠️ 项目文件夹命名

#### 当前状态

项目文件夹名称：`AnyWallpaper-Engine` (带连接符)

#### 建议

**推荐重命名为**: `AnyWP_Engine`

原因：
- 与代码命名一致
- 无空格和连接符
- 简短清晰

#### 重命名步骤

```powershell
# 1. 关闭所有编辑器
# 2. 重命名文件夹
cd E:\Projects\AnyWallpaper
Rename-Item "AnyWallpaper-Engine" "AnyWP_Engine"

# 3. 更新 example/lib/main.dart 中的测试文件路径
#    将 file:///E:/Projects/AnyWallpaper/AnyWallpaper-Engine
#    改为 file:///E:/Projects/AnyWallpaper/AnyWP_Engine

# 4. 重新打开项目
code AnyWP_Engine
```

### ✅ 构建验证

```bash
# 清理并重新构建
cd example
flutter clean
flutter pub get
flutter build windows --debug

# 结果: ✅ 成功
# Built build\windows\x64\runner\Debug\anywallpaper_engine_example.exe
```

### 🎯 使用新API

#### 初始化壁纸

```dart
import 'package:anywp_engine/anywp_engine.dart';

await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);
```

#### 停止壁纸

```dart
await AnyWPEngine.stopWallpaper();
```

#### 导航URL

```dart
await AnyWPEngine.navigateToUrl('https://new-url.com');
```

### 📋 注意事项

1. **旧代码兼容**: 由于包名和类名都改变了，旧代码需要更新导入和类名
2. **文件夹重命名**: 建议但非必须，不影响功能
3. **Flutter 副本**: `any_w_p_engine_plugin.h` 是自动生成兼容副本，不要手动编辑
4. **构建输出**: exe文件名暂时还是 `anywallpaper_engine_example.exe`，这是example项目的名称，可以保持不变

### 📚 相关文档

- [NAMING_CONVENTION.md](../NAMING_CONVENTION.md) - 完整命名规范
- [BUILD_NOTES.md](BUILD_NOTES.md) - 构建注意事项
- [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) - 项目结构

---

**迁移日期**: 2025-11-02  
**版本**: 1.0.0
**状态**: ✅ 完成并验证


