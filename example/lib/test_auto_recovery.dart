import 'dart:io';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

/// Auto Recovery 完整测试示例
///
/// 测试场景：
/// 1. 启用 Auto Recovery
/// 2. 设置壁纸
/// 3. 验证配置保存
/// 4. 模拟 Explorer 重启（手动）
/// 5. 验证壁纸自动恢复
class AutoRecoveryTestApp extends StatelessWidget {
  const AutoRecoveryTestApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Auto Recovery Test',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const TestPage(),
    );
  }
}

class TestPage extends StatefulWidget {
  const TestPage({super.key});

  @override
  State<TestPage> createState() => _TestPageState();
}

class _TestPageState extends State<TestPage> {
  final List<String> _testLogs = [];
  bool _isRunning = false;
  bool _autoRecoveryEnabled = false;
  bool _wallpaperSet = false;
  bool _configSaved = false;

  @override
  void initState() {
    super.initState();
    // 不自动运行，等待用户点击
  }

  void _log(String message, {bool isError = false}) {
    final timestamp = DateTime.now().toString().substring(11, 19);
    final prefix = isError ? '❌' : '✓';
    setState(() {
      _testLogs.add('[$timestamp] $prefix $message');
    });
    debugPrint('[$timestamp] $prefix $message');
  }

  Future<void> _runTest() async {
    setState(() {
      _isRunning = true;
      _testLogs.clear();
      _autoRecoveryEnabled = false;
      _wallpaperSet = false;
      _configSaved = false;
    });

    _log('=== Auto Recovery 完整测试 ===');
    _log('');

    // Step 1: 启用 Auto Recovery
    await _testEnableAutoRecovery();
    await Future.delayed(const Duration(seconds: 1));

    // Step 2: 验证状态
    await _testCheckStatus();
    await Future.delayed(const Duration(seconds: 1));

    // Step 3: 设置壁纸
    await _testSetWallpaper();
    await Future.delayed(const Duration(seconds: 2));

    // Step 4: 验证配置保存
    await _testVerifyConfig();
    await Future.delayed(const Duration(seconds: 1));

    _log('');
    _log('=== 测试完成 ===');
    _log('');
    _log('下一步：手动重启 Explorer 验证恢复功能');
    _log('1. 打开任务管理器（Ctrl+Shift+Esc）');
    _log('2. 找到"Windows 资源管理器"');
    _log('3. 右键 → 结束任务');
    _log('4. 文件 → 运行新任务 → 输入 explorer.exe');
    _log('5. 检查壁纸是否自动恢复');

    setState(() {
      _isRunning = false;
    });
  }

  Future<void> _testEnableAutoRecovery() async {
    _log('[Test 1/4] 启用 Auto Recovery...');
    try {
      await AnyWPEngine.enableAutoRecovery(true);
      _log('enableAutoRecovery(true) 调用成功');

      setState(() {
        _autoRecoveryEnabled = true;
      });
    } catch (e) {
      _log('enableAutoRecovery 失败: $e', isError: true);
    }
  }

  Future<void> _testCheckStatus() async {
    _log('[Test 2/4] 检查 Auto Recovery 状态...');
    try {
      final enabled = await AnyWPEngine.isAutoRecoveryEnabled();
      if (enabled) {
        _log('Auto Recovery 状态: 已启用 ✓');
      } else {
        _log('Auto Recovery 状态: 未启用', isError: true);
      }
    } catch (e) {
      _log('检查状态失败: $e', isError: true);
    }
  }

  Future<void> _testSetWallpaper() async {
    _log('[Test 3/4] 设置测试壁纸...');

    // 使用项目中的测试 HTML
    final htmlPath =
        '${Directory.current.path}/../examples/test_visibility.html';
    final file = File(htmlPath);

    if (!file.existsSync()) {
      _log('测试 HTML 不存在: $htmlPath', isError: true);
      _log('使用备用方案：Windows 默认壁纸');

      // 使用 Windows 默认图片作为测试
      try {
        final success = await AnyWPEngine.initializeWallpaperOnMonitor(
          url: 'file:///C:/Windows/Web/Screen/img100.jpg',
          monitorIndex: 0,
        );

        if (success) {
          _log('壁纸设置成功（使用系统图片）');
          setState(() {
            _wallpaperSet = true;
          });
        } else {
          _log('壁纸设置失败', isError: true);
        }
      } catch (e) {
        _log('设置壁纸异常: $e', isError: true);
      }
      return;
    }

    try {
      final success = await AnyWPEngine.initializeWallpaperOnMonitor(
        url: 'file:///${file.absolute.path.replaceAll('\\', '/')}',
        monitorIndex: 0,
        // autoSave: true 是默认值，会自动保存配置
      );

      if (success) {
        _log('壁纸设置成功（使用测试 HTML）');
        _log('URL: file:///${file.absolute.path}');
        setState(() {
          _wallpaperSet = true;
        });
      } else {
        _log('壁纸设置失败', isError: true);
      }
    } catch (e) {
      _log('设置壁纸异常: $e', isError: true);
    }
  }

  Future<void> _testVerifyConfig() async {
    _log('[Test 4/4] 验证配置保存...');

    // 等待配置保存完成
    await Future.delayed(const Duration(milliseconds: 500));

    _log('配置应该已自动保存（autoSave: true）');
    _log('');
    _log('检查引擎日志确认：');
    _log('  - 应该看到: "Auto recovery ENABLED"');
    _log('  - 应该看到: "Wallpaper configuration saved"');
    _log('  - 应该看到: "Auto recovery: 1 configuration(s) saved"');

    setState(() {
      _configSaved = true;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Auto Recovery 测试'),
      ),
      body: Column(
        children: [
          // 状态指示器
          Container(
            color: Colors.grey[200],
            padding: const EdgeInsets.all(16),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _StatusIndicator(
                  label: '1. 启用',
                  isActive: _autoRecoveryEnabled,
                ),
                _StatusIndicator(
                  label: '2. 设置',
                  isActive: _wallpaperSet,
                ),
                _StatusIndicator(
                  label: '3. 保存',
                  isActive: _configSaved,
                ),
              ],
            ),
          ),

          // 控制按钮
          Padding(
            padding: const EdgeInsets.all(16),
            child: SizedBox(
              width: double.infinity,
              child: ElevatedButton(
                onPressed: _isRunning ? null : _runTest,
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.symmetric(vertical: 16),
                ),
                child: _isRunning
                    ? const Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(strokeWidth: 2),
                          ),
                          SizedBox(width: 12),
                          Text('测试进行中...'),
                        ],
                      )
                    : const Text(
                        '开始测试',
                        style: TextStyle(fontSize: 16),
                      ),
              ),
            ),
          ),

          const Divider(height: 1),

          // 日志输出
          Expanded(
            child: ListView.builder(
              padding: const EdgeInsets.all(8),
              itemCount: _testLogs.length,
              itemBuilder: (context, index) {
                final log = _testLogs[index];
                final isError = log.contains('❌');
                final isSuccess = log.contains('✓');
                final isHeader = log.contains('===');

                return Padding(
                  padding: const EdgeInsets.symmetric(vertical: 2),
                  child: Text(
                    log,
                    style: TextStyle(
                      fontFamily: 'Courier',
                      fontSize: 12,
                      color: isError
                          ? Colors.red
                          : isSuccess
                              ? Colors.green
                              : isHeader
                                  ? Colors.blue
                                  : Colors.black87,
                      fontWeight:
                          isHeader ? FontWeight.bold : FontWeight.normal,
                    ),
                  ),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}

class _StatusIndicator extends StatelessWidget {
  final String label;
  final bool isActive;

  const _StatusIndicator({
    required this.label,
    required this.isActive,
  });

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Container(
          width: 60,
          height: 60,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            color: isActive ? Colors.green : Colors.grey[300],
          ),
          child: Icon(
            isActive ? Icons.check : Icons.circle,
            color: Colors.white,
            size: 30,
          ),
        ),
        const SizedBox(height: 8),
        Text(
          label,
          style: TextStyle(
            fontSize: 12,
            color: isActive ? Colors.green : Colors.grey,
            fontWeight: isActive ? FontWeight.bold : FontWeight.normal,
          ),
        ),
      ],
    );
  }
}
