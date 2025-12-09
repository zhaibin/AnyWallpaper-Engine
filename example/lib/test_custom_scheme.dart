import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

/// anywp:// 协议测试页面
/// 
/// 功能：
/// 1. 加载测试 HTML 页面
/// 2. 监听协议请求
/// 3. 显示测试结果
class TestCustomSchemePage extends StatefulWidget {
  const TestCustomSchemePage({super.key});

  @override
  State<TestCustomSchemePage> createState() => _TestCustomSchemePageState();
}

class _TestCustomSchemePageState extends State<TestCustomSchemePage> {
  bool _isInitialized = false;
  String _status = 'Initializing...';

  @override
  void initState() {
    super.initState();
    _initialize();
  }

  Future<void> _initialize() async {
    try {
      setState(() {
        _status = 'Initializing wallpaper engine...';
      });

      // 获取测试 HTML 路径
      final htmlPath = 'E:\\Projects\\AnyWallpaper\\AnyWP-Test\\examples\\test_custom_scheme.html';
      
      // 初始化引擎
      await AnyWPEngine.initializeWallpaper(url: 'file:///$htmlPath');
      
      setState(() {
        _isInitialized = true;
        _status = 'Wallpaper engine initialized!\nTest HTML loaded: test_custom_scheme.html';
      });

      // 延迟显示壁纸
      await Future.delayed(const Duration(milliseconds: 500));
      // await _engine.show(); // Note: show() method removed, wallpaper is visible by default

      debugPrint('[TestCustomScheme] ✓ Wallpaper shown');
      debugPrint('[TestCustomScheme] Test page URL: file:///$htmlPath');
      debugPrint('[TestCustomScheme] Expected to handle anywp:// protocol requests');
      
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
      });
      debugPrint('[TestCustomScheme] ✗ Error: $e');
    }
  }

  @override
  void dispose() async {
    await AnyWPEngine.stopWallpaper();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('anywp:// Protocol Test'),
        backgroundColor: Colors.deepPurple,
      ),
      body: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
            colors: [Colors.deepPurple.shade400, Colors.purple.shade700],
          ),
        ),
        child: Center(
          child: Container(
            constraints: const BoxConstraints(maxWidth: 600),
            margin: const EdgeInsets.all(20),
            padding: const EdgeInsets.all(30),
            decoration: BoxDecoration(
              color: Colors.white,
              borderRadius: BorderRadius.circular(20),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.3),
                  blurRadius: 20,
                  offset: const Offset(0, 10),
                ),
              ],
            ),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                // 状态图标
                Icon(
                  _isInitialized ? Icons.check_circle : Icons.hourglass_empty,
                  size: 80,
                  color: _isInitialized ? Colors.green : Colors.orange,
                ),
                const SizedBox(height: 20),
                
                // 标题
                const Text(
                  'Custom Scheme Test',
                  style: TextStyle(
                    fontSize: 28,
                    fontWeight: FontWeight.bold,
                    color: Colors.black87,
                  ),
                ),
                const SizedBox(height: 10),
                
                // 状态文本
                Text(
                  _status,
                  textAlign: TextAlign.center,
                  style: TextStyle(
                    fontSize: 16,
                    color: Colors.grey[700],
                    height: 1.5,
                  ),
                ),
                const SizedBox(height: 30),
                
                // 功能说明
                Container(
                  padding: const EdgeInsets.all(20),
                  decoration: BoxDecoration(
                    color: Colors.grey[100],
                    borderRadius: BorderRadius.circular(10),
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Text(
                        '📋 Test Features:',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(height: 10),
                      _buildFeatureItem('✓ anywp:// protocol registration'),
                      _buildFeatureItem('✓ XOR decryption (Key: 0x5A)'),
                      _buildFeatureItem('✓ MIME type detection'),
                      _buildFeatureItem('✓ Stream-based file transfer'),
                      const SizedBox(height: 15),
                      const Text(
                        '🎯 Check the test HTML page for results',
                        style: TextStyle(
                          fontSize: 14,
                          color: Colors.deepPurple,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                ),
                const SizedBox(height: 20),
                
                // 测试文件信息
                Container(
                  padding: const EdgeInsets.all(15),
                  decoration: BoxDecoration(
                    color: Colors.blue[50],
                    borderRadius: BorderRadius.circular(10),
                    border: Border.all(color: Colors.blue.shade200),
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: const [
                      Text(
                        '📂 Test Files Location:',
                        style: TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 14,
                        ),
                      ),
                      SizedBox(height: 5),
                      Text(
                        '%AppData%\\HKCW_Desktop\\cache\\images\\',
                        style: TextStyle(
                          fontFamily: 'Courier New',
                          fontSize: 12,
                          color: Colors.blue,
                        ),
                      ),
                      SizedBox(height: 5),
                      Text(
                        '• test_jpeg_001.encrypted\n'
                        '• test_png_001.encrypted',
                        style: TextStyle(fontSize: 12),
                      ),
                    ],
                  ),
                ),
                const SizedBox(height: 20),
                
                // 操作按钮
                if (_isInitialized) ...[
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      ElevatedButton.icon(
                        onPressed: () async {
                          await AnyWPEngine.pauseWallpaper();
                          setState(() {
                            _status = 'Wallpaper paused';
                          });
                        },
                        icon: const Icon(Icons.visibility_off),
                        label: const Text('Hide'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.orange,
                          padding: const EdgeInsets.symmetric(
                            horizontal: 20,
                            vertical: 12,
                          ),
                        ),
                      ),
                      const SizedBox(width: 10),
                      ElevatedButton.icon(
                        onPressed: () async {
                          await AnyWPEngine.resumeWallpaper();
                          setState(() {
                            _status = 'Wallpaper resumed';
                          });
                        },
                        icon: const Icon(Icons.visibility),
                        label: const Text('Show'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.green,
                          padding: const EdgeInsets.symmetric(
                            horizontal: 20,
                            vertical: 12,
                          ),
                        ),
                      ),
                    ],
                  ),
                ],
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildFeatureItem(String text) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 5),
      child: Text(
        text,
        style: const TextStyle(fontSize: 14),
      ),
    );
  }
}

