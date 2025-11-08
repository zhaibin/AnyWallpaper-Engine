import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'package:window_manager/window_manager.dart';
import 'package:path/path.dart' as path;

/// 自动化测试应用 - 依次测试所有功能并收集日志
void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  await windowManager.ensureInitialized();
  await AnyWPEngine.setApplicationName('AnyWPAutoTest');
  
  runApp(const AutoTestApp());
}

class AutoTestApp extends StatefulWidget {
  const AutoTestApp({Key? key}) : super(key: key);

  @override
  State<AutoTestApp> createState() => _AutoTestAppState();
}

class _AutoTestAppState extends State<AutoTestApp> {
  final List<TestCase> _testCases = [
    TestCase(
      name: 'test_simple.html',
      description: '基础壁纸测试 - 紫色渐变 + 时钟',
      duration: Duration(seconds: 10),
    ),
    TestCase(
      name: 'test_draggable.html',
      description: '拖拽功能测试 - 可拖拽元素 + 状态持久化',
      duration: Duration(seconds: 15),
    ),
    TestCase(
      name: 'test_api.html',
      description: '完整 API 测试 - 所有 SDK 功能',
      duration: Duration(seconds: 15),
    ),
    TestCase(
      name: 'test_basic_click.html',
      description: '点击检测测试 - 鼠标事件处理',
      duration: Duration(seconds: 10),
    ),
    TestCase(
      name: 'test_react.html',
      description: 'React SPA 测试',
      duration: Duration(seconds: 10),
    ),
    TestCase(
      name: 'test_vue.html',
      description: 'Vue SPA 测试',
      duration: Duration(seconds: 10),
    ),
    TestCase(
      name: 'test_iframe_ads.html',
      description: 'iframe 坐标映射测试',
      duration: Duration(seconds: 10),
    ),
  ];
  
  int _currentTestIndex = 0;
  List<String> _testLogs = [];
  bool _isRunning = false;
  String _status = '准备开始测试...';
  MonitorInfo? _monitor;
  
  @override
  void initState() {
    super.initState();
    _initTest();
  }
  
  Future<void> _initTest() async {
    // 获取显示器信息
    final monitors = await AnyWPEngine.getMonitors();
    if (monitors.isEmpty) {
      _log('错误: 未找到显示器');
      return;
    }
    
    _monitor = monitors.first;
    _log('找到显示器: ${_monitor!.deviceName} (${_monitor!.width}x${_monitor!.height})');
    
    // 延迟 2 秒后自动开始测试
    await Future.delayed(Duration(seconds: 2));
    _startAutoTest();
  }
  
  Future<void> _startAutoTest() async {
    setState(() {
      _isRunning = true;
      _status = '测试进行中...';
    });
    
    for (int i = 0; i < _testCases.length; i++) {
      _currentTestIndex = i;
      final testCase = _testCases[i];
      
      _log('\n========================================');
      _log('测试 ${i + 1}/${_testCases.length}: ${testCase.name}');
      _log('描述: ${testCase.description}');
      _log('========================================');
      
      setState(() {
        _status = '[${ i + 1}/${_testCases.length}] ${testCase.name}';
      });
      
      await _runTest(testCase);
      
      // 测试之间暂停 2 秒
      if (i < _testCases.length - 1) {
        _log('等待 2 秒后继续下一个测试...\n');
        await Future.delayed(Duration(seconds: 2));
      }
    }
    
    _log('\n========================================');
    _log('所有测试完成！');
    _log('========================================');
    
    setState(() {
      _isRunning = false;
      _status = '测试完成';
    });
    
    // 保存日志到文件
    await _saveLogsToFile();
    
    // 5 秒后自动退出
    await Future.delayed(Duration(seconds: 5));
    exit(0);
  }
  
  Future<void> _runTest(TestCase testCase) async {
    try {
      // 获取测试文件的绝对路径
      final exePath = Platform.resolvedExecutable;
      final exeDir = File(exePath).parent.path;
      final htmlPath = path.join(
        exeDir,
        'data',
        'flutter_assets',
        'assets',
        'examples',
        testCase.name,
      );
      
      final htmlFile = File(htmlPath);
      if (!htmlFile.existsSync()) {
        _log('错误: 测试文件不存在: $htmlPath');
        return;
      }
      
      final url = 'file:///${htmlPath.replaceAll('\\', '/')}';
      _log('加载 URL: $url');
      
      // 启动壁纸
      _log('启动壁纸...');
      final success = await AnyWPEngine.initializeWallpaperOnMonitor(
        url: url,
        monitorIndex: _monitor!.index,
        enableMouseTransparent: true,
      );
      
      if (!success) {
        _log('错误: 壁纸启动失败');
        return;
      }
      
      _log('壁纸启动成功，运行 ${testCase.duration.inSeconds} 秒...');
      
      // 等待测试持续时间
      await Future.delayed(testCase.duration);
      
      // 停止壁纸
      _log('停止壁纸...');
      await AnyWPEngine.stopWallpaperOnMonitor(_monitor!.index);
      _log('壁纸已停止');
      
    } catch (e, stackTrace) {
      _log('测试异常: $e');
      _log('堆栈: $stackTrace');
    }
  }
  
  void _log(String message) {
    final timestamp = DateTime.now().toString().substring(11, 19);
    final logMessage = '[$timestamp] $message';
    print(logMessage);
    setState(() {
      _testLogs.add(logMessage);
    });
  }
  
  Future<void> _saveLogsToFile() async {
    try {
      final logFile = File('auto_test_results.log');
      await logFile.writeAsString(_testLogs.join('\n'));
      _log('日志已保存到: ${logFile.absolute.path}');
    } catch (e) {
      _log('保存日志失败: $e');
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'AnyWP 自动化测试',
      theme: ThemeData.dark(),
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Padding(
          padding: EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                'AnyWP Engine 自动化测试',
                style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
              ),
              SizedBox(height: 16),
              Text(
                _status,
                style: TextStyle(fontSize: 18, color: Colors.green),
              ),
              SizedBox(height: 8),
              if (_isRunning)
                LinearProgressIndicator(
                  value: _currentTestIndex / _testCases.length,
                ),
              SizedBox(height: 16),
              Text(
                '测试进度: ${_currentTestIndex + (_isRunning ? 1 : 0)}/${_testCases.length}',
                style: TextStyle(fontSize: 16),
              ),
              SizedBox(height: 16),
              Expanded(
                child: Container(
                  decoration: BoxDecoration(
                    border: Border.all(color: Colors.grey),
                    borderRadius: BorderRadius.circular(4),
                  ),
                  child: ListView.builder(
                    itemCount: _testLogs.length,
                    itemBuilder: (context, index) {
                      return Padding(
                        padding: EdgeInsets.symmetric(horizontal: 8, vertical: 2),
                        child: Text(
                          _testLogs[index],
                          style: TextStyle(fontSize: 12, fontFamily: 'monospace'),
                        ),
                      );
                    },
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
  
  @override
  void dispose() {
    super.dispose();
  }
}

class TestCase {
  final String name;
  final String description;
  final Duration duration;
  
  TestCase({
    required this.name,
    required this.description,
    required this.duration,
  });
}

