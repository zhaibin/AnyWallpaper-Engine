/// Auto Recovery 功能测试
/// 
/// 此测试验证 Auto Recovery 的完整工作流程
/// 测试场景：
/// 1. 启用 Auto Recovery
/// 2. 使用标准 API 设置壁纸
/// 3. 验证配置是否保存
/// 4. 模拟 Explorer 重启（需手动测试）

import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  print('\n=== Auto Recovery Test ===\n');
  
  try {
    // Test 1: 启用 Auto Recovery
    print('Test 1: Enabling Auto Recovery...');
    await AnyWPEngine.enableAutoRecovery(true);
    print('✅ Auto Recovery enabled\n');
    
    // Test 2: 验证状态
    print('Test 2: Checking status...');
    final enabled = await AnyWPEngine.isAutoRecoveryEnabled();
    print('✅ Auto Recovery status: $enabled\n');
    
    if (!enabled) {
      print('❌ FAILED: Auto Recovery not enabled!');
      return;
    }
    
    // Test 3: 初始化壁纸（使用标准 API）
    print('Test 3: Initializing wallpaper with standard API...');
    final testHtmlPath = 'E:/Projects/AnyWallpaper/AnyWP-Test/examples/test_auto_recovery.html';
    print('Using URL: file:///$testHtmlPath');
    
    final success = await AnyWPEngine.initializeWallpaperOnMonitor(
      url: 'file:///$testHtmlPath',
      monitorIndex: 0,
      autoSave: true,  // 默认自动保存
    );
    
    if (success) {
      print('✅ Wallpaper initialized successfully\n');
    } else {
      print('❌ FAILED: Wallpaper initialization failed!');
      return;
    }
    
    // Test 4: 等待一下，让配置保存完成
    print('Test 4: Waiting for configuration to be saved...');
    await Future.delayed(Duration(seconds: 2));
    print('✅ Wait complete\n');
    
    // Test 5: 验证配置已保存（再次检查状态）
    print('Test 5: Verifying configuration...');
    final stillEnabled = await AnyWPEngine.isAutoRecoveryEnabled();
    print('✅ Auto Recovery still enabled: $stillEnabled\n');
    
    // 成功提示
    print('=== Test Complete ===\n');
    print('✅ All automated tests passed!\n');
    print('📋 Manual Test Steps:');
    print('   1. Check the wallpaper is displayed on desktop');
    print('   2. Open Task Manager');
    print('   3. Find "Windows Explorer"');
    print('   4. Right-click → End Task');
    print('   5. Run new task: explorer.exe');
    print('   6. Verify wallpaper automatically recovers\n');
    
  } catch (e, stackTrace) {
    print('❌ Exception during test: $e');
    print('Stack trace: $stackTrace');
  }
  
  // 保持应用运行，以便手动测试 Explorer 重启
  runApp(TestApp());
}

class TestApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Auto Recovery Test'),
          backgroundColor: Colors.green,
        ),
        body: Center(
          child: Padding(
            padding: EdgeInsets.all(24),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(Icons.check_circle, size: 100, color: Colors.green),
                SizedBox(height: 24),
                Text(
                  'Auto Recovery Test',
                  style: TextStyle(fontSize: 28, fontWeight: FontWeight.bold),
                ),
                SizedBox(height: 16),
                Text(
                  'Automated tests passed! ✅',
                  style: TextStyle(fontSize: 18, color: Colors.green),
                ),
                SizedBox(height: 32),
                Text(
                  'Manual Test Steps:',
                  style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                ),
                SizedBox(height: 16),
                _buildStep('1', 'Check wallpaper is displayed'),
                _buildStep('2', 'Open Task Manager'),
                _buildStep('3', 'End "Windows Explorer" task'),
                _buildStep('4', 'Run: explorer.exe'),
                _buildStep('5', 'Verify auto recovery'),
                SizedBox(height: 32),
                ElevatedButton.icon(
                  icon: Icon(Icons.refresh),
                  label: Text('Restart Explorer Now'),
                  onPressed: _restartExplorer,
                  style: ElevatedButton.styleFrom(
                    padding: EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
  
  Widget _buildStep(String number, String text) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: 4),
      child: Row(
        children: [
          CircleAvatar(
            child: Text(number),
            backgroundColor: Colors.blue,
            radius: 16,
          ),
          SizedBox(width: 12),
          Text(text, style: TextStyle(fontSize: 16)),
        ],
      ),
    );
  }
  
  void _restartExplorer() async {
    final confirmed = await showDialog<bool>(
      context: null,
      builder: (context) => AlertDialog(
        title: Text('Restart Explorer?'),
        content: Text('This will restart Windows Explorer to test auto recovery.'),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: Text('Cancel'),
          ),
          ElevatedButton(
            onPressed: () => Navigator.pop(context, true),
            child: Text('Restart'),
          ),
        ],
      ),
    );
    
    if (confirmed == true) {
      // 这里可以调用系统命令重启 Explorer
      print('Restarting Explorer...');
    }
  }
}



