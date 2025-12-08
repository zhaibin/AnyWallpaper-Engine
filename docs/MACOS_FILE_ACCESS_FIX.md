# macOS 测试页面访问问题修复方案

## 问题原因

macOS 应用运行在沙箱容器中，无法访问项目根目录的 `examples` 文件夹：
```
当前目录: /Users/zhaibin/Library/Containers/com.example.anywpEngineExample/Data
项目目录: /Users/zhaibin/Dev/anywp-engine/examples ❌ 无法访问
```

## 解决方案（3种）

### 方案 1: 使用 file:// 协议 ⭐ 最简单

直接使用绝对路径：

```dart
final url = 'file:///Users/zhaibin/Dev/anywp-engine/examples/test_carousel_control.html';
await AnyWPEngine.initializeWallpaper(url: url);
```

**优点**：
- ✅ 立即可用
- ✅ 无需额外配置

**缺点**：
- ⚠️ 可能有 CORS 限制

---

### 方案 2: 使用 LocalFileServer（localfile:// 协议）⭐⭐ 推荐

使用新实现的 macOS LocalFileServer：

```dart
// 1. 启动文件服务器
final result = await AnyWPEngine.startFileServer(
  rootPath: '/Users/zhaibin/Dev/anywp-engine',
);

if (result?['success'] == true) {
  print('Server started: ${result!['baseURL']}');
  
  // 2. 使用 localfile:// 协议访问
  final url = 'localfile:///examples/test_carousel_control.html';
  await AnyWPEngine.initializeWallpaper(url: url);
}
```

**优点**：
- ✅ 支持 CORS
- ✅ 无需 HTTP 端口
- ✅ 系统级集成

**缺点**：
- ⚠️ 需要显式启动服务器

---

### 方案 3: 将 examples 复制到 Bundle ⭐⭐⭐ 最佳（但需重新构建）

修改 `example/macos/Runner/Info.plist` 和构建脚本：

1. 在 `macos/Runner/Resources/` 创建符号链接或复制 examples：
```bash
cd example/macos/Runner
mkdir -p Resources
ln -s ../../../examples Resources/examples
# 或
cp -r ../../../examples Resources/examples
```

2. 在 Info.plist 中添加资源：
```xml
<key>CFBundleResourceSpecification</key>
<string>Resources/examples</string>
```

3. 代码中使用 Bundle 路径：
```dart
final bundlePath = Platform.resolvedExecutable.replaceAll('/Runner', '');
final examplesPath = '$bundlePath/Resources/examples';
```

**优点**：
- ✅ 完美的沙箱兼容
- ✅ HTTP 服务器可用
- ✅ 生产环境友好

**缺点**：
- ❌ 需要修改构建配置
- ❌ examples 更新需重新构建

---

## 快速测试（立即可用）

### 测试 file:// 协议

```dart
// 在 URL 输入框中输入：
file:///Users/zhaibin/Dev/anywp-engine/examples/test_simple.html
```

### 测试 localfile:// 协议

1. 在应用代码中添加启动代码：
```dart
@override
void initState() {
  super.initState();
  _startLocalFileServer();
}

Future<void> _startLocalFileServer() async {
  final result = await AnyWPEngine.startFileServer(
    rootPath: '/Users/zhaibin/Dev/anywp-engine',
  );
  print('LocalFileServer: $result');
}
```

2. 使用 URL：
```
localfile:///examples/test_simple.html
```

---

## 推荐做法

**开发测试**：
- 使用方案 1（file://）快速测试
- 使用方案 2（localfile://）测试新功能

**生产部署**：
- 使用方案 3（Bundle Resources）

---

## 修改示例应用

修改 `example/lib/main.dart` 的 `_startHttpServer` 方法：

```dart
Future<void> _startHttpServer() async {
  try {
    if (Platform.isMacOS) {
      // macOS: 使用 LocalFileServer
      print('[HTTP] macOS detected, using LocalFileServer');
      final devPath = '/Users/zhaibin/Dev/anywp-engine';
      final result = await AnyWPEngine.startFileServer(
        rootPath: devPath,
      );
      if (result?['success'] == true) {
        setState(() {
          _httpServerBaseUrl = 'localfile://';
        });
        print('[HTTP] ✅ LocalFileServer started');
        print('[HTTP] Test URL: localfile:///examples/test_carousel_control.html');
      }
    } else {
      // Windows: 使用 Dart HTTP Server
      // ... 原有代码 ...
    }
  } catch (e) {
    print('[HTTP] Failed: $e');
  }
}
```

---

## 总结

| 方案 | 复杂度 | CORS | 生产可用 | 推荐度 |
|------|--------|------|----------|--------|
| file:// | ⭐ | ⚠️ | ⚠️ | 测试用 |
| localfile:// | ⭐⭐ | ✅ | ✅ | 推荐 |
| Bundle | ⭐⭐⭐ | ✅ | ✅ | 最佳 |

**当前建议**：使用 **localfile://** 协议（方案2），这是我们今天实现的新功能！

