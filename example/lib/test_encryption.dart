import 'dart:io';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

/// 测试加密解密功能
class EncryptionTestPage extends StatefulWidget {
  const EncryptionTestPage({Key? key}) : super(key: key);

  @override
  State<EncryptionTestPage> createState() => _EncryptionTestPageState();
}

class _EncryptionTestPageState extends State<EncryptionTestPage> {
  final List<String> _logs = [];
  bool _testing = false;

  void _addLog(String message) {
    setState(() {
      _logs.add('[${DateTime.now().toString().substring(11, 19)}] $message');
    });
    print(message);
  }

  Future<void> _runEncryptionTest() async {
    if (_testing) return;
    
    setState(() {
      _testing = true;
      _logs.clear();
    });

    try {
      _addLog('开始加密解密测试...');
      
      // 1. 创建测试文件
      final testDir = Directory.systemTemp.createTempSync('anywp_test_');
      final testFile = File('${testDir.path}/test_image.txt');
      final testContent = 'Hello AnyWP Engine! This is a test file for encryption/decryption. ' * 10;
      await testFile.writeAsString(testContent);
      _addLog('✅ 测试文件创建成功: ${testFile.path}');
      _addLog('   文件大小: ${testContent.length} bytes');

      // 2. 加密测试
      final encryptedFile = File('${testDir.path}/test_encrypted.dat');
      _addLog('');
      _addLog('📦 开始加密...');
      _addLog('   源文件: ${testFile.path}');
      _addLog('   目标文件: ${encryptedFile.path}');
      
      final encryptSuccess = await AnyWPEngine.encryptFile(
        sourcePath: testFile.path,
        destPath: encryptedFile.path,
      );
      
      if (encryptSuccess) {
        _addLog('✅ 加密成功！');
        _addLog('   加密文件大小: ${await encryptedFile.length()} bytes');
        
        // 验证加密文件内容与原文件不同
        final originalBytes = await testFile.readAsBytes();
        final encryptedBytes = await encryptedFile.readAsBytes();
        
        if (originalBytes.length == encryptedBytes.length) {
          bool isDifferent = false;
          for (int i = 0; i < 64 && i < originalBytes.length; i++) {
            if (originalBytes[i] != encryptedBytes[i]) {
              isDifferent = true;
              break;
            }
          }
          if (isDifferent) {
            _addLog('✅ 验证通过：前64字节已加密');
          } else {
            _addLog('⚠️ 警告：前64字节未改变');
          }
        }
      } else {
        _addLog('❌ 加密失败！');
        return;
      }

      // 3. 解密测试
      final decryptedFile = File('${testDir.path}/test_decrypted.txt');
      _addLog('');
      _addLog('📂 开始解密...');
      _addLog('   加密文件: ${encryptedFile.path}');
      _addLog('   目标文件: ${decryptedFile.path}');
      
      final decryptSuccess = await AnyWPEngine.decryptFile(
        encryptedPath: encryptedFile.path,
        destPath: decryptedFile.path,
      );
      
      if (decryptSuccess) {
        _addLog('✅ 解密成功！');
        _addLog('   解密文件大小: ${await decryptedFile.length()} bytes');
        
        // 4. 验证解密内容
        final decryptedContent = await decryptedFile.readAsString();
        if (decryptedContent == testContent) {
          _addLog('');
          _addLog('🎉 测试完全成功！');
          _addLog('   原始内容与解密内容完全一致');
        } else {
          _addLog('');
          _addLog('❌ 测试失败！');
          _addLog('   原始内容与解密内容不一致');
          _addLog('   原始长度: ${testContent.length}');
          _addLog('   解密长度: ${decryptedContent.length}');
        }
      } else {
        _addLog('❌ 解密失败！');
      }

      // 清理临时文件
      _addLog('');
      _addLog('🧹 清理临时文件...');
      await testDir.delete(recursive: true);
      _addLog('✅ 测试完成！');

    } catch (e, stackTrace) {
      _addLog('');
      _addLog('❌ 测试异常: $e');
      _addLog('Stack trace: $stackTrace');
    } finally {
      setState(() {
        _testing = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('AnyWP Engine - 加密解密测试'),
        backgroundColor: Colors.blue,
      ),
      body: Column(
        children: [
          // 控制按钮
          Container(
            padding: const EdgeInsets.all(16),
            color: Colors.grey[100],
            child: Row(
              children: [
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: _testing ? null : _runEncryptionTest,
                    icon: Icon(_testing ? Icons.hourglass_empty : Icons.play_arrow),
                    label: Text(_testing ? '测试进行中...' : '开始测试'),
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 16),
                      backgroundColor: Colors.blue,
                      foregroundColor: Colors.white,
                    ),
                  ),
                ),
                const SizedBox(width: 16),
                ElevatedButton.icon(
                  onPressed: () {
                    setState(() {
                      _logs.clear();
                    });
                  },
                  icon: const Icon(Icons.clear),
                  label: const Text('清空日志'),
                  style: ElevatedButton.styleFrom(
                    padding: const EdgeInsets.symmetric(vertical: 16),
                    backgroundColor: Colors.grey,
                    foregroundColor: Colors.white,
                  ),
                ),
              ],
            ),
          ),
          
          // 日志区域
          Expanded(
            child: Container(
              color: Colors.black87,
              padding: const EdgeInsets.all(16),
              child: _logs.isEmpty
                  ? const Center(
                      child: Text(
                        '点击 "开始测试" 按钮运行加密解密测试',
                        style: TextStyle(color: Colors.white70, fontSize: 16),
                      ),
                    )
                  : ListView.builder(
                      itemCount: _logs.length,
                      itemBuilder: (context, index) {
                        final log = _logs[index];
                        Color textColor = Colors.white;
                        
                        if (log.contains('✅')) {
                          textColor = Colors.green;
                        } else if (log.contains('❌')) {
                          textColor = Colors.red;
                        } else if (log.contains('⚠️')) {
                          textColor = Colors.orange;
                        } else if (log.contains('🎉')) {
                          textColor = Colors.greenAccent;
                        }
                        
                        return Padding(
                          padding: const EdgeInsets.symmetric(vertical: 2),
                          child: Text(
                            log,
                            style: TextStyle(
                              fontFamily: 'Courier',
                              fontSize: 13,
                              color: textColor,
                            ),
                          ),
                        );
                      },
                    ),
            ),
          ),
          
          // 底部说明
          Container(
            padding: const EdgeInsets.all(16),
            color: Colors.blue[50],
            child: const Row(
              children: [
                Icon(Icons.info_outline, color: Colors.blue),
                SizedBox(width: 8),
                Expanded(
                  child: Text(
                    '此测试将创建临时文件进行加密/解密验证，测试完成后自动清理',
                    style: TextStyle(fontSize: 12, color: Colors.black87),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

