# ⚡ AnyWP Engine 快速集成

> 30秒快速集成壁纸引擎到你的项目

---

## 🎯 三步集成

### 1️⃣ 添加依赖

在你的项目 `pubspec.yaml` 中添加：

```yaml
dependencies:
  # 方式A：本地路径（开发推荐）
  anywp_engine:
    path: ../AnyWP_Engine

  # 方式B：Git 仓库（团队推荐）
  anywp_engine:
    git:
      url: https://github.com/zhaibin/AnyWallpaper-Engine.git
      ref: v1.0.0

  # 方式C：pub.dev（发布后）
  anywp_engine: ^1.0.0
```

### 2️⃣ 获取依赖

```bash
flutter pub get
```

### 3️⃣ 使用代码

```dart
import 'package:anywp_engine/anywp_engine.dart';

// 启动壁纸
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);

// 停止壁纸
await AnyWPEngine.stopWallpaper();
```

---

## 📋 完整示例

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('壁纸控制器')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: () => AnyWPEngine.initializeWallpaper(
                  url: 'https://www.bing.com',
                  enableMouseTransparent: true,
                ),
                child: Text('🚀 启动壁纸'),
              ),
              SizedBox(height: 20),
              ElevatedButton(
                onPressed: () => AnyWPEngine.stopWallpaper(),
                child: Text('⏹️ 停止壁纸'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

---

## 🔥 常用场景

### 场景1：透明壁纸（点击穿透）
```dart
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,  // 鼠标穿透
);
```

### 场景2：交互式壁纸（可点击）
```dart
await AnyWPEngine.initializeWallpaper(
  url: 'https://game.example.com',
  enableMouseTransparent: false,  // 可交互
);
```

### 场景3：本地 HTML 文件
```dart
await AnyWPEngine.initializeWallpaper(
  url: 'file:///E:/wallpapers/index.html',
  enableMouseTransparent: true,
);
```

### 场景4：动态切换 URL
```dart
// 不需要重启，直接切换
await AnyWPEngine.navigateToUrl('https://new-site.com');
```

---

## 🛠️ 首次配置（仅需一次）

如果这是首次使用，需要安装 WebView2 SDK：

```bash
# 在插件目录运行
cd AnyWP_Engine/scripts
setup_webview2.bat
```

---

## 📖 详细文档

- **完整指南**: [docs/PACKAGE_USAGE_GUIDE_CN.md](docs/PACKAGE_USAGE_GUIDE_CN.md)
- **API 文档**: [docs/API_BRIDGE.md](docs/API_BRIDGE.md)
- **故障排除**: [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)

---

## 💡 提示

✅ **推荐做法**
- 开发阶段使用 `path:` 引用
- 团队协作使用 `git:` 引用
- 生产环境使用 `pub.dev` 版本

❌ **常见错误**
- 忘记运行 `flutter pub get`
- WebView2 SDK 未安装
- 使用了错误的路径格式

---

**更新日期**: 2025-11-01

