# AnyWP Engine v1.1.0 - 预编译版本

## 📦 包含内容

- `bin/` - 预编译的 DLL 文件
- `lib/` - Dart 源代码
- `include/` - C++ 头文件
- `sdk/` - JavaScript SDK
- `windows/` - CMake 配置

## 🚀 快速集成

### 1. 在你的 Flutter 项目 `pubspec.yaml` 中添加：
```yaml
dependencies:
  anywp_engine:
    path: ./anywp_engine_v1.1.0
```

### 2. 获取依赖并构建
```bash
flutter pub get
flutter build windows
```

### 3. 开始使用：
```dart
import 'package:anywp_engine/anywp_engine.dart';

await AnyWPEngine.initializeWallpaper(url: 'https://example.com');
```

## 📚 完整文档

请参阅 README.md 和 CHANGELOG_CN.md

或访问：https://github.com/zhaibin/AnyWallpaper-Engine
