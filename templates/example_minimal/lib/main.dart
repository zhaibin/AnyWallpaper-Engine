import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  // 设置独立的应用名称，确保存储隔离
  await AnyWPEngine.setApplicationName('ExampleApp');

  // 检查插件版本兼容性
  final compatible = await AnyWPEngine.isCompatible(expectedPrefix: '1.2.');
  if (!compatible) {
    final version = await AnyWPEngine.getPluginVersion();
    debugPrint('[Example] ⚠️ 插件版本不兼容（当前: $version，需要: 1.2.x）');
  }

  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'AnyWP Engine Example',
      theme: ThemeData.dark(useMaterial3: true),
      home: const MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  bool _wallpaperRunning = false;
  String _statusMessage = '点击按钮启动壁纸';

  Future<void> _startWallpaper() async {
    setState(() {
      _statusMessage = '正在启动...';
    });

    final success = await AnyWPEngine.initializeWallpaper(
      url: 'https://www.bing.com',
      enableMouseTransparent: true,
    );

    setState(() {
      _wallpaperRunning = success;
      _statusMessage = success ? '✅ 壁纸运行中（Bing）' : '❌ 启动失败，请检查控制台日志';
    });
  }

  Future<void> _stopWallpaper() async {
    final success = await AnyWPEngine.stopWallpaper();
    setState(() {
      _wallpaperRunning = !success;
      _statusMessage = success ? '壁纸已停止' : '停止失败，请重试';
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('AnyWP Engine 示例'),
      ),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text(
              _statusMessage,
              style: const TextStyle(fontSize: 16),
            ),
            const SizedBox(height: 24),
            FilledButton.icon(
              onPressed: _wallpaperRunning ? null : _startWallpaper,
              icon: const Icon(Icons.play_arrow),
              label: const Text('启动壁纸'),
            ),
            const SizedBox(height: 12),
            FilledButton.icon(
              onPressed: _wallpaperRunning ? _stopWallpaper : null,
              icon: const Icon(Icons.stop),
              label: const Text('停止壁纸'),
            ),
          ],
        ),
      ),
    );
  }
}

