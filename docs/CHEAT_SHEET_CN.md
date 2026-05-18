# 🎯 AnyWP Engine 速查表

> 一页纸快速参考 - 打印或保存以便随时查阅

---

## 📦 依赖配置

| 方式 | pubspec.yaml 配置 | 使用场景 |
|------|------------------|---------|
| **本地路径** | `anywp_engine:`<br>`  path: ../AnyWP_Engine` | 🔧 开发调试 |
| **Git 仓库** | `anywp_engine:`<br>`  git:`<br>`    url: github.com/user/repo.git`<br>`    ref: v1.0.0` | 👥 团队协作 |
| **pub.dev** | `anywp_engine: ^1.0.0` | 🚀 生产环境 |

**安装依赖**: `flutter pub get`

---

## 🎮 API 速查

### 初始化壁纸
```dart
// 透明壁纸（点击穿透）
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);

// 交互壁纸（可点击）
await AnyWPEngine.initializeWallpaper(
  url: 'https://game.com',
  enableMouseTransparent: false,
);
```

### 停止壁纸
```dart
await AnyWPEngine.stopWallpaper();
```

### 导航到新 URL
```dart
await AnyWPEngine.navigateToUrl('https://new-url.com');
```

---

## 🔥 常用代码模板

### 最小化示例
```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() => runApp(MaterialApp(
  home: Scaffold(
    body: Center(
      child: ElevatedButton(
        onPressed: () => AnyWPEngine.initializeWallpaper(
          url: 'https://www.bing.com',
          enableMouseTransparent: true,
        ),
        child: Text('启动壁纸'),
      ),
    ),
  ),
));
```

### 带状态管理
```dart
class WallpaperController extends StatefulWidget {
  @override
  State createState() => _WallpaperControllerState();
}

class _WallpaperControllerState extends State {
  bool _isRunning = false;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        ElevatedButton(
          onPressed: _isRunning ? null : () async {
            var ok = await AnyWPEngine.initializeWallpaper(
              url: 'https://www.bing.com',
              enableMouseTransparent: true,
            );
            if (ok) setState(() => _isRunning = true);
          },
          child: Text('启动'),
        ),
        ElevatedButton(
          onPressed: !_isRunning ? null : () async {
            var ok = await AnyWPEngine.stopWallpaper();
            if (ok) setState(() => _isRunning = false);
          },
          child: Text('停止'),
        ),
      ],
    );
  }
}
```

---

## 🛠️ 命令速查

| 命令 | 说明 |
|------|------|
| `flutter pub get` | 安装依赖 |
| `flutter pub upgrade anywp_engine` | 更新插件 |
| `flutter clean` | 清理构建缓存 |
| `flutter run -d windows` | 运行（Debug） |
| `flutter build windows --release` | 构建（Release） |
| `.\scripts\setup_webview2.bat` | 安装 WebView2 SDK |

---

## 🎯 常用 URL 类型

| 类型 | 示例 | 说明 |
|------|------|------|
| **HTTPS** | `https://www.bing.com` | 在线网页 |
| **HTTP** | `http://localhost:3000` | 本地服务器 |
| **本地文件** | `file:///E:/wallpapers/index.html` | 本地 HTML |
| **视频** | `https://youtube.com/watch?v=...` | 视频网站 |

---

## 🔧 配置选项

### enableMouseTransparent

| 值 | 效果 | 适用场景 |
|----|------|---------|
| `true` | ✅ 鼠标穿透<br>✅ 桌面图标可点击<br>❌ 壁纸不可交互 | 纯壁纸展示 |
| `false` | ❌ 鼠标不穿透<br>❌ 桌面图标被遮挡<br>✅ 壁纸可交互 | 游戏、仪表盘 |

---

## 🐛 故障排除

| 问题 | 解决方案 |
|------|---------|
| 找不到包 | `flutter pub get` |
| WebView2 错误 | 运行 `.\scripts\setup_webview2.bat` |
| 壁纸不显示 | 检查 Windows 版本（需要 Win10+） |
| Git 更新失败 | `flutter pub upgrade anywp_engine --major-versions` |
| 构建失败 | `flutter clean && flutter pub get` |
| URL 无效 | 检查 URL 格式（https://...） |

---

## 📁 项目结构速查

```
你的项目/
├── lib/
│   └── main.dart          ← 引入: import 'package:anywp_engine/...'
├── windows/
│   ├── flutter/
│   │   └── generated_plugins.cmake  ← 自动生成
│   └── CMakeLists.txt
└── pubspec.yaml           ← 配置依赖
```

---

## 🎨 完整示例（复制即用）

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'AnyWP 控制器',
      home: WallpaperScreen(),
    );
  }
}

class WallpaperScreen extends StatefulWidget {
  @override
  _WallpaperScreenState createState() => _WallpaperScreenState();
}

class _WallpaperScreenState extends State<WallpaperScreen> {
  final _urlController = TextEditingController(
    text: 'https://www.bing.com'
  );
  bool _isRunning = false;
  bool _transparent = true;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('壁纸引擎控制器')),
      body: Padding(
        padding: EdgeInsets.all(20),
        child: Column(
          children: [
            // URL 输入
            TextField(
              controller: _urlController,
              decoration: InputDecoration(
                labelText: 'URL',
                border: OutlineInputBorder(),
              ),
            ),
            SizedBox(height: 20),
            
            // 透明选项
            SwitchListTile(
              title: Text('鼠标透明'),
              value: _transparent,
              onChanged: (v) => setState(() => _transparent = v),
            ),
            SizedBox(height: 30),
            
            // 控制按钮
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton.icon(
                  onPressed: _isRunning ? null : _start,
                  icon: Icon(Icons.play_arrow),
                  label: Text('启动'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green,
                  ),
                ),
                ElevatedButton.icon(
                  onPressed: _isRunning ? _stop : null,
                  icon: Icon(Icons.stop),
                  label: Text('停止'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.red,
                  ),
                ),
              ],
            ),
            SizedBox(height: 20),
            
            // 状态
            Text(
              _isRunning ? '🟢 运行中' : '⚪ 未运行',
              style: TextStyle(fontSize: 18),
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _start() async {
    final success = await AnyWPEngine.initializeWallpaper(
      url: _urlController.text,
      enableMouseTransparent: _transparent,
    );
    if (success) setState(() => _isRunning = true);
  }

  Future<void> _stop() async {
    final success = await AnyWPEngine.stopWallpaper();
    if (success) setState(() => _isRunning = false);
  }

  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }
}
```

---

## 📞 获取帮助

| 资源 | 链接 |
|------|------|
| **快速集成** | [QUICK_INTEGRATION.md](../QUICK_INTEGRATION.md) |
| **完整指南** | [PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md) |
| **架构文档** | [ARCHITECTURE_DESIGN.md](ARCHITECTURE_DESIGN.md) |
| **API 文档** | [API_BRIDGE.md](API_BRIDGE.md) |
| **GitHub Issues** | [报告问题](https://github.com/zhaibin/AnyWallpaper-Engine/issues) |

---

## 🎓 学习路径

1. ⚡ **新手**: 先看 [QUICK_INTEGRATION.md](../QUICK_INTEGRATION.md)
2. 📚 **进阶**: 阅读 [PACKAGE_USAGE_GUIDE_CN.md](PACKAGE_USAGE_GUIDE_CN.md)
3. 🏗️ **深入**: 研究 [ARCHITECTURE_DESIGN.md](ARCHITECTURE_DESIGN.md)
4. 🔧 **调试**: 参考 [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

**版本**: 1.0.0  
**更新**: 2025-11-01  
**打印提示**: 建议双面打印，方便携带 📄

