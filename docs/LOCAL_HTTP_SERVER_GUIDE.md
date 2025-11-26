# 本地 HTTP 文件服务器集成指南

> **版本**: v2.5.1+  
> **用途**: 在 Flutter 应用中启动本地 HTTP 服务器，解决本地 HTML 文件加载问题

---

## 为什么需要本地 HTTP 服务器？

当您使用本地 HTML 文件作为壁纸时，可能会遇到以下问题：

1. **CORS 跨域限制**: 本地文件 (`file://`) 协议无法访问外部资源
2. **Web API 限制**: 某些现代 Web API 要求 HTTP/HTTPS 协议
3. **资源加载失败**: 相对路径资源可能无法正确加载

本地 HTTP 服务器通过将文件通过 `http://127.0.0.1:端口` 提供服务来解决这些问题。

---

## 快速开始

### 1. 添加依赖

在 `pubspec.yaml` 中添加：

```yaml
dependencies:
  shelf: ^1.4.1
  shelf_static: ^1.1.2
```

### 2. 复制服务器类

将 `LocalFileServer` 类复制到您的项目中：

```dart
// lib/local_file_server.dart
import 'dart:io';
import 'package:shelf/shelf.dart';
import 'package:shelf/shelf_io.dart' as shelf_io;
import 'package:shelf_static/shelf_static.dart';

/// 本地 HTTP 文件服务器
/// 
/// 用于提供本地 HTML 壁纸文件服务，避免 CORS 问题
class LocalFileServer {
  HttpServer? _server;
  String _baseUrl = '';
  
  /// 检查服务器是否正在运行
  bool get isRunning => _server != null;
  
  /// 获取服务器基础 URL
  String get baseUrl => _baseUrl;
  
  /// 启动 HTTP 服务器
  /// 
  /// [rootPath] - 要提供服务的根目录
  /// 返回基础 URL (如 http://127.0.0.1:54321)
  Future<String> start(String rootPath) async {
    if (_server != null) {
      return _baseUrl;
    }
    
    // 创建静态文件处理器
    final handler = createStaticHandler(
      rootPath,
      defaultDocument: 'index.html',
      listDirectories: true,
    );
    
    // 添加 CORS 头
    final corsHandler = Pipeline()
        .addMiddleware(_corsHeaders())
        .addHandler(handler);
    
    // 启动服务器（端口 0 让系统自动分配）
    _server = await shelf_io.serve(
      corsHandler,
      '127.0.0.1',
      0,
    );
    
    _baseUrl = 'http://${_server!.address.host}:${_server!.port}';
    return _baseUrl;
  }
  
  /// 停止服务器
  Future<void> stop() async {
    if (_server == null) return;
    
    await _server!.close(force: true);
    _server = null;
    _baseUrl = '';
  }
  
  /// CORS 中间件
  Middleware _corsHeaders() {
    return (Handler handler) {
      return (Request request) async {
        final response = await handler(request);
        return response.change(headers: {
          'Access-Control-Allow-Origin': '*',
          'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
          'Access-Control-Allow-Headers': 'Origin, Content-Type, Accept',
        });
      };
    };
  }
}
```

### 3. 在应用中使用

```dart
import 'package:anywp_engine/anywp_engine.dart';
import 'local_file_server.dart';

class MyApp extends StatefulWidget {
  // ...
}

class _MyAppState extends State<MyApp> {
  final LocalFileServer _fileServer = LocalFileServer();
  String _serverUrl = '';
  
  @override
  void initState() {
    super.initState();
    _startServer();
  }
  
  @override
  void dispose() {
    _fileServer.stop();
    super.dispose();
  }
  
  Future<void> _startServer() async {
    // 启动服务器，提供 wallpapers 目录
    final url = await _fileServer.start('/path/to/your/wallpapers');
    setState(() {
      _serverUrl = url;
    });
    print('Server started at: $_serverUrl');
  }
  
  Future<void> _startWallpaper() async {
    // 使用 HTTP URL 加载壁纸
    await AnyWPEngine.initializeWallpaper(
      url: '$_serverUrl/my_wallpaper.html',
    );
  }
}
```

---

## 完整集成示例

### 项目结构

```
your_app/
├── lib/
│   ├── main.dart
│   └── local_file_server.dart
├── wallpapers/                    # 本地壁纸文件目录
│   ├── my_wallpaper.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       └── script.js
└── pubspec.yaml
```

### main.dart

```dart
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'package:path/path.dart' as path;
import 'local_file_server.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await AnyWPEngine.setApplicationName('MyWallpaperApp');
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);
  
  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final LocalFileServer _fileServer = LocalFileServer();
  String _serverUrl = '';
  bool _isServerRunning = false;
  bool _isWallpaperRunning = false;
  
  @override
  void initState() {
    super.initState();
    _initializeServer();
  }
  
  @override
  void dispose() {
    AnyWPEngine.stopWallpaper();
    _fileServer.stop();
    super.dispose();
  }
  
  Future<void> _initializeServer() async {
    try {
      // 获取 wallpapers 目录路径
      final appDir = Directory.current.path;
      final wallpapersPath = path.join(appDir, 'wallpapers');
      
      // 确保目录存在
      final dir = Directory(wallpapersPath);
      if (!await dir.exists()) {
        await dir.create(recursive: true);
        print('Created wallpapers directory: $wallpapersPath');
      }
      
      // 启动服务器
      final url = await _fileServer.start(wallpapersPath);
      
      setState(() {
        _serverUrl = url;
        _isServerRunning = true;
      });
      
      print('✅ HTTP Server started at: $_serverUrl');
    } catch (e) {
      print('❌ Failed to start server: $e');
    }
  }
  
  Future<void> _toggleWallpaper() async {
    if (_isWallpaperRunning) {
      await AnyWPEngine.stopWallpaper();
      setState(() => _isWallpaperRunning = false);
    } else {
      // 使用 HTTP URL 加载本地壁纸
      final success = await AnyWPEngine.initializeWallpaper(
        url: '$_serverUrl/my_wallpaper.html',
      );
      setState(() => _isWallpaperRunning = success);
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('AnyWP + HTTP Server')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              // 服务器状态
              Text(
                _isServerRunning 
                    ? '✅ Server: $_serverUrl' 
                    : '⏳ Starting server...',
                style: TextStyle(
                  color: _isServerRunning ? Colors.green : Colors.orange,
                ),
              ),
              const SizedBox(height: 20),
              
              // 壁纸控制按钮
              ElevatedButton(
                onPressed: _isServerRunning ? _toggleWallpaper : null,
                child: Text(_isWallpaperRunning ? 'Stop Wallpaper' : 'Start Wallpaper'),
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

## 高级用法

### 指定固定端口

如果需要固定端口，修改 `start` 方法：

```dart
Future<String> start(String rootPath, {int port = 0}) async {
  // ...
  _server = await shelf_io.serve(
    corsHandler,
    '127.0.0.1',
    port,  // 传入 0 自动分配，或指定端口号
  );
  // ...
}
```

### 处理服务器启动失败

```dart
Future<void> _initializeServer() async {
  try {
    final url = await _fileServer.start(wallpapersPath);
    // 成功
  } on SocketException catch (e) {
    // 端口被占用或网络问题
    print('Network error: $e');
  } catch (e) {
    // 其他错误
    print('Server error: $e');
  }
}
```

### 从可执行文件打包目录获取路径

打包后的 Windows 应用，壁纸文件通常在可执行文件旁边：

```dart
import 'package:path/path.dart' as path;

String getWallpapersPath() {
  // 获取可执行文件目录
  final exePath = Platform.resolvedExecutable;
  final exeDir = path.dirname(exePath);
  
  // wallpapers 目录在可执行文件旁边
  return path.join(exeDir, 'data', 'wallpapers');
}
```

---

## 常见问题

### Q: 服务器启动失败？

1. 检查端口是否被占用
2. 检查目录路径是否正确
3. 确保依赖已正确安装：`flutter pub get`

### Q: 壁纸加载失败？

1. 确认服务器已启动（检查 `isRunning`）
2. 在浏览器中测试 URL：`http://127.0.0.1:端口/your_file.html`
3. 检查文件是否存在于指定目录

### Q: 如何调试？

启动服务器后，在浏览器中访问：
- 根目录列表：`http://127.0.0.1:端口/`
- 特定文件：`http://127.0.0.1:端口/your_wallpaper.html`

### Q: 是否支持热重载？

是的！服务器直接提供文件服务，修改 HTML/CSS/JS 后刷新壁纸即可看到更新。

---

## 对比其他方案

| 方案 | 优点 | 缺点 |
|------|------|------|
| **本地 HTTP 服务器** | 无 CORS 问题，支持所有 Web API | 需要额外依赖 |
| `file://` 协议 | 简单直接 | CORS 限制，部分 API 不可用 |
| `anywp://file?path=` | 内置支持 | 仅支持加密文件 |
| 远程 URL | 无需本地文件 | 需要网络，加载慢 |

---

## 相关文档

- [Flutter 开发者指南](FOR_FLUTTER_DEVELOPERS.md)
- [API 参考](DEVELOPER_API_REFERENCE.md)
- [Web 开发者指南](WEB_DEVELOPER_GUIDE_CN.md)

