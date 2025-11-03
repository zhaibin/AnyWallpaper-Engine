# 📦 AnyWP Engine 打包与使用指南

本指南详细说明如何将 AnyWP Engine 打包给其他项目使用，提供三种不同的集成方式。

---

## 🎯 三种集成方式对比

| 方式 | 适用场景 | 优点 | 缺点 |
|------|---------|------|------|
| **本地路径引用** | 开发测试、本地项目 | 简单快速、即时修改 | 不便于分发 |
| **Git 仓库引用** | 团队协作、版本管理 | 便于共享、版本追踪 | 需要 Git 仓库 |
| **pub.dev 发布** | 公开发布、生产环境 | 最规范、易于管理 | 需要发布审核 |

---

## 📋 方式一：本地路径引用（推荐用于开发）

### 1. 准备插件包

确保你的插件目录结构完整：

```
AnyWP_Engine/
├── lib/
│   └── anywp_engine.dart
├── windows/
│   ├── anywp_engine_plugin.cpp
│   ├── anywp_engine_plugin.h
│   ├── anywp_sdk.js
│   └── CMakeLists.txt
├── pubspec.yaml
└── README.md
```

### 2. 在其他项目中引用

**场景A：相邻目录结构**
```
Projects/
├── AnyWP_Engine/          # 插件目录
└── MyWallpaperApp/        # 你的项目
    └── pubspec.yaml
```

在 `MyWallpaperApp/pubspec.yaml` 中添加：
```yaml
dependencies:
  flutter:
    sdk: flutter
  anywp_engine:
    path: ../AnyWP_Engine
```

**场景B：自定义路径**
```yaml
dependencies:
  anywp_engine:
    path: E:/Plugins/AnyWP_Engine
```

### 3. 使用示例

```dart
import 'package:anywp_engine/anywp_engine.dart';

class WallpaperController {
  // 启动壁纸（透明模式）
  Future<void> startTransparentWallpaper() async {
    bool success = await AnyWPEngine.initializeWallpaper(
      url: 'https://www.bing.com',
      enableMouseTransparent: true,
    );
    
    if (success) {
      print('✅ 壁纸启动成功');
    } else {
      print('❌ 壁纸启动失败');
    }
  }

  // 启动交互式壁纸
  Future<void> startInteractiveWallpaper() async {
    await AnyWPEngine.initializeWallpaper(
      url: 'https://game.example.com',
      enableMouseTransparent: false,
    );
  }

  // 切换网址
  Future<void> changeUrl(String newUrl) async {
    await AnyWPEngine.navigateToUrl(newUrl);
  }

  // 停止壁纸
  Future<void> stop() async {
    await AnyWPEngine.stopWallpaper();
  }
}
```

### 4. 获取依赖

```bash
cd MyWallpaperApp
flutter pub get
```

---

## 🌐 方式二：Git 仓库引用（推荐用于团队）

### 1. 发布到 Git 仓库

**步骤 A：创建 GitHub 仓库**

1. 在 GitHub 创建新仓库，例如：`https://github.com/yourusername/anywp_engine`
2. 使用项目中的脚本推送：

```bash
# 方法1：使用自动脚本
cd AnyWP_Engine
scripts\PUSH_TO_GITHUB.bat

# 方法2：手动推送
git init
git add .
git commit -m "Initial release v1.0.0"
git branch -M main
git remote add origin https://github.com/yourusername/anywp_engine.git
git push -u origin main
```

**步骤 B：创建版本标签（可选但推荐）**

```bash
git tag v1.0.0
git push origin v1.0.0
```

### 2. 在其他项目中引用

**方式 A：引用主分支**
```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/yourusername/anywp_engine.git
```

**方式 B：引用特定分支**
```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/yourusername/anywp_engine.git
      ref: develop
```

**方式 C：引用特定版本（推荐）**
```yaml
dependencies:
  anywp_engine:
    git:
      url: https://github.com/yourusername/anywp_engine.git
      ref: v1.0.0
```

**方式 D：引用私有仓库**
```yaml
dependencies:
  anywp_engine:
    git:
      url: git@github.com:yourusername/anywp_engine.git
      ref: v1.0.0
```

### 3. 版本管理最佳实践

```bash
# 发布新版本
git tag v1.0.1
git push origin v1.0.1

# 更新依赖
flutter pub upgrade anywp_engine
```

---

## 📢 方式三：发布到 pub.dev（推荐用于公开发布）

### 1. 准备发布

**检查清单：**

- [ ] 更新 `pubspec.yaml` 中的版本号
- [ ] 更新 `CHANGELOG.md` 记录变更
- [ ] 完善 `README.md` 文档
- [ ] 添加 `LICENSE` 文件
- [ ] 确保代码通过 `flutter analyze`
- [ ] 确保示例代码可运行

**示例 pubspec.yaml 配置：**
```yaml
name: anywp_engine
description: A Flutter Windows plugin that embeds WebView2 as an interactive desktop wallpaper, displaying web content behind desktop icons.
version: 1.0.0
homepage: https://github.com/yourusername/anywp_engine
repository: https://github.com/yourusername/anywp_engine
issue_tracker: https://github.com/yourusername/anywp_engine/issues
documentation: https://github.com/yourusername/anywp_engine/wiki

environment:
  sdk: '>=3.0.0 <4.0.0'
  flutter: ">=3.0.0"

dependencies:
  flutter:
    sdk: flutter
  plugin_platform_interface: ^2.0.0

flutter:
  plugin:
    platforms:
      windows:
        pluginClass: AnyWPEnginePlugin
        fileName: anywp_engine_plugin.cpp
```

### 2. 验证包

```bash
# 检查包是否符合发布标准
flutter pub publish --dry-run
```

修复所有警告和错误。

### 3. 发布包

```bash
# 正式发布（需要 Google 账号）
flutter pub publish
```

### 4. 在其他项目中使用

```yaml
dependencies:
  anywp_engine: ^1.0.0
```

---

## 🚀 完整使用示例

### 最小化示例

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: WallpaperScreen(),
    );
  }
}

class WallpaperScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('壁纸控制器')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            ElevatedButton(
              onPressed: () async {
                await AnyWPEngine.initializeWallpaper(
                  url: 'https://www.bing.com',
                  enableMouseTransparent: true,
                );
              },
              child: Text('启动壁纸'),
            ),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: () async {
                await AnyWPEngine.stopWallpaper();
              },
              child: Text('停止壁纸'),
            ),
          ],
        ),
      ),
    );
  }
}
```

### 完整功能示例

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

class AdvancedWallpaperController extends StatefulWidget {
  @override
  _AdvancedWallpaperControllerState createState() => 
      _AdvancedWallpaperControllerState();
}

class _AdvancedWallpaperControllerState 
    extends State<AdvancedWallpaperController> {
  
  final TextEditingController _urlController = TextEditingController(
    text: 'https://www.bing.com'
  );
  bool _isRunning = false;
  bool _enableTransparent = true;

  // 预设网址
  final List<Map<String, String>> _presets = [
    {'name': 'Bing', 'url': 'https://www.bing.com'},
    {'name': 'YouTube', 'url': 'https://www.youtube.com'},
    {'name': '本地文件', 'url': 'file:///E:/wallpapers/index.html'},
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('AnyWP 壁纸引擎'),
        backgroundColor: Colors.blue,
      ),
      body: Padding(
        padding: EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // URL 输入
            TextField(
              controller: _urlController,
              decoration: InputDecoration(
                labelText: '壁纸 URL',
                border: OutlineInputBorder(),
                prefixIcon: Icon(Icons.link),
              ),
            ),
            
            SizedBox(height: 20),
            
            // 预设按钮
            Wrap(
              spacing: 10,
              children: _presets.map((preset) {
                return ElevatedButton(
                  onPressed: () {
                    setState(() {
                      _urlController.text = preset['url']!;
                    });
                  },
                  child: Text(preset['name']!),
                );
              }).toList(),
            ),
            
            SizedBox(height: 20),
            
            // 鼠标透明选项
            SwitchListTile(
              title: Text('鼠标透明（点击穿透）'),
              subtitle: Text(
                _enableTransparent 
                  ? '启用：点击会穿透到桌面' 
                  : '禁用：可以与壁纸交互'
              ),
              value: _enableTransparent,
              onChanged: (value) {
                setState(() {
                  _enableTransparent = value;
                });
              },
            ),
            
            SizedBox(height: 30),
            
            // 控制按钮
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton.icon(
                  onPressed: _isRunning ? null : _startWallpaper,
                  icon: Icon(Icons.play_arrow),
                  label: Text('启动壁纸'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green,
                    padding: EdgeInsets.symmetric(
                      horizontal: 30, 
                      vertical: 15
                    ),
                  ),
                ),
                ElevatedButton.icon(
                  onPressed: _isRunning ? _stopWallpaper : null,
                  icon: Icon(Icons.stop),
                  label: Text('停止壁纸'),
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.red,
                    padding: EdgeInsets.symmetric(
                      horizontal: 30, 
                      vertical: 15
                    ),
                  ),
                ),
              ],
            ),
            
            SizedBox(height: 20),
            
            // 导航按钮（仅在运行时可用）
            if (_isRunning)
              ElevatedButton.icon(
                onPressed: _navigateToUrl,
                icon: Icon(Icons.navigation),
                label: Text('跳转到当前 URL'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.orange,
                ),
              ),
            
            SizedBox(height: 30),
            
            // 状态指示
            Container(
              padding: EdgeInsets.all(15),
              decoration: BoxDecoration(
                color: _isRunning ? Colors.green[100] : Colors.grey[200],
                borderRadius: BorderRadius.circular(10),
                border: Border.all(
                  color: _isRunning ? Colors.green : Colors.grey,
                ),
              ),
              child: Row(
                children: [
                  Icon(
                    _isRunning ? Icons.check_circle : Icons.radio_button_unchecked,
                    color: _isRunning ? Colors.green : Colors.grey,
                  ),
                  SizedBox(width: 10),
                  Text(
                    _isRunning ? '壁纸运行中' : '壁纸未运行',
                    style: TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.bold,
                      color: _isRunning ? Colors.green[800] : Colors.grey[800],
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Future<void> _startWallpaper() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showSnackBar('请输入 URL', isError: true);
      return;
    }

    final success = await AnyWPEngine.initializeWallpaper(
      url: url,
      enableMouseTransparent: _enableTransparent,
    );

    if (success) {
      setState(() => _isRunning = true);
      _showSnackBar('✅ 壁纸启动成功');
    } else {
      _showSnackBar('❌ 壁纸启动失败', isError: true);
    }
  }

  Future<void> _stopWallpaper() async {
    final success = await AnyWPEngine.stopWallpaper();
    
    if (success) {
      setState(() => _isRunning = false);
      _showSnackBar('✅ 壁纸已停止');
    } else {
      _showSnackBar('❌ 停止壁纸失败', isError: true);
    }
  }

  Future<void> _navigateToUrl() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) return;

    final success = await AnyWPEngine.navigateToUrl(url);
    
    if (success) {
      _showSnackBar('✅ 已跳转到: $url');
    } else {
      _showSnackBar('❌ 跳转失败', isError: true);
    }
  }

  void _showSnackBar(String message, {bool isError = false}) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: isError ? Colors.red : Colors.green,
        duration: Duration(seconds: 2),
      ),
    );
  }

  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }
}
```

---

## 🔧 配置和依赖

### Windows 系统要求

- Windows 10 或 Windows 11
- WebView2 Runtime（Windows 11 自带）

### 首次使用配置

如果你的项目首次使用 AnyWP Engine，需要确保 WebView2 SDK 已安装：

```bash
# 在插件目录运行（仅需一次）
cd AnyWP_Engine/scripts
setup_webview2.bat
```

### CMakeLists.txt 配置

你的项目 `windows/CMakeLists.txt` 应该会自动包含插件，但如果遇到问题，可以检查：

```cmake
# 应该自动生成在 windows/flutter/generated_plugins.cmake
include(flutter/generated_plugins.cmake)
```

---

## 📝 API 参考

### AnyWPEngine 类

#### `initializeWallpaper()`
初始化并启动桌面壁纸。

**参数：**
- `url` (String, 必需) - 要显示的网址或本地 HTML 路径
- `enableMouseTransparent` (bool, 可选) - 是否启用鼠标穿透，默认 `true`

**返回：** `Future<bool>` - 成功返回 `true`

**示例：**
```dart
await AnyWPEngine.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);
```

#### `stopWallpaper()`
停止并清理壁纸。

**返回：** `Future<bool>` - 成功返回 `true`

**示例：**
```dart
await AnyWPEngine.stopWallpaper();
```

#### `navigateToUrl()`
在不重启壁纸的情况下导航到新 URL。

**参数：**
- `url` (String, 必需) - 新的网址

**返回：** `Future<bool>` - 成功返回 `true`

**示例：**
```dart
await AnyWPEngine.navigateToUrl('https://new-site.com');
```

---

## 🐛 故障排除

### 常见问题

**Q: 引用后提示找不到包？**
```bash
# 清理缓存后重新获取
flutter clean
flutter pub get
```

**Q: Windows 构建失败？**
```bash
# 确保 WebView2 SDK 已安装
cd AnyWP_Engine/scripts
setup_webview2.bat
```

**Q: 壁纸不显示？**
- 检查 Windows 版本（需要 Win10+）
- 确认 WebView2 Runtime 已安装
- 查看调试日志

**Q: Git 引用更新不生效？**
```bash
# 强制更新到最新版本
flutter pub upgrade anywp_engine --major-versions
```

---

## 📚 更多资源

- [完整示例代码](../example/lib/main.dart)
- [API 文档](API_BRIDGE.md)
- [技术细节](TECHNICAL_NOTES.md)
- [测试指南](TESTING_GUIDE.md)
- [故障排除](TROUBLESHOOTING.md)

---

## 📞 获取帮助

- **GitHub Issues**: [报告问题](https://github.com/yourusername/anywp_engine/issues)
- **讨论区**: [提问和交流](https://github.com/yourusername/anywp_engine/discussions)

---

**最后更新**: 2025-11-01  
**版本**: 1.0.0

