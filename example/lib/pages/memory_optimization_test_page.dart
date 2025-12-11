import 'dart:async';
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

/// 内存优化效果测试页面
/// 
/// 功能:
/// 1. 显示实时内存使用情况
/// 2. 手动触发内存优化
/// 3. 开启/关闭自动定时优化
/// 4. 统计优化效果
class MemoryOptimizationTestPage extends StatefulWidget {
  const MemoryOptimizationTestPage({super.key});

  @override
  State<MemoryOptimizationTestPage> createState() => _MemoryOptimizationTestPageState();
}

class _MemoryOptimizationTestPageState extends State<MemoryOptimizationTestPage> {
  // 内存监控
  int _currentMemoryMB = 0;
  int _peakMemoryMB = 0;
  Timer? _memoryMonitorTimer;
  
  // 优化统计
  int _optimizationCount = 0;
  int _totalMemoryFreedMB = 0;
  final List<int> _recentOptimizations = [];
  
  // 自动优化
  Timer? _autoOptimizationTimer;
  bool _isAutoOptimizationEnabled = false;
  Duration _autoOptimizationInterval = const Duration(minutes: 1);
  
  // 日志
  final List<String> _logs = [];
  final ScrollController _logScrollController = ScrollController();
  
  @override
  void initState() {
    super.initState();
    _startMemoryMonitoring();
  }
  
  @override
  void dispose() {
    _memoryMonitorTimer?.cancel();
    _autoOptimizationTimer?.cancel();
    _logScrollController.dispose();
    super.dispose();
  }
  
  /// 开始监控内存
  void _startMemoryMonitoring() {
    _memoryMonitorTimer = Timer.periodic(const Duration(seconds: 2), (timer) async {
      try {
        final memory = await AnyWPEngine.getMemoryUsage();
        setState(() {
          _currentMemoryMB = memory;
          if (memory > _peakMemoryMB) {
            _peakMemoryMB = memory;
          }
        });
      } catch (e) {
        _addLog('❌ Failed to get memory usage: $e');
      }
    });
  }
  
  /// 手动触发内存优化
  Future<void> _manualOptimization() async {
    _addLog('🧹 Manual optimization triggered...');
    
    final memoryBefore = await AnyWPEngine.getMemoryUsage();
    _addLog('   Memory before: $memoryBefore MB');
    
    final startTime = DateTime.now();
    
    try {
      final success = await AnyWPEngine.optimizeMemory();
      
      if (success) {
        // 等待 2 秒让异步清理完成
        await Future.delayed(const Duration(seconds: 2));
        
        final memoryAfter = await AnyWPEngine.getMemoryUsage();
        final freed = memoryBefore > memoryAfter ? memoryBefore - memoryAfter : 0;
        final duration = DateTime.now().difference(startTime);
        
        setState(() {
          _optimizationCount++;
          _totalMemoryFreedMB += freed;
          _recentOptimizations.add(freed);
          if (_recentOptimizations.length > 10) {
            _recentOptimizations.removeAt(0);
          }
        });
        
        _addLog('   Memory after: $memoryAfter MB');
        _addLog('   ✅ Freed: $freed MB (took ${duration.inMilliseconds}ms)');
        
        if (freed < 10) {
          _addLog('   ⚠️  Low optimization effect (<10 MB)');
        }
      } else {
        _addLog('   ❌ Optimization failed');
      }
    } catch (e) {
      _addLog('   ❌ Exception: $e');
    }
  }
  
  /// 开启/关闭自动优化
  void _toggleAutoOptimization() {
    setState(() {
      _isAutoOptimizationEnabled = !_isAutoOptimizationEnabled;
    });
    
    if (_isAutoOptimizationEnabled) {
      _addLog('🤖 Auto optimization enabled (interval: ${_autoOptimizationInterval.inMinutes} min)');
      
      _autoOptimizationTimer = Timer.periodic(_autoOptimizationInterval, (timer) async {
        _addLog('⏰ Auto optimization triggered');
        await _manualOptimization();
      });
    } else {
      _addLog('🛑 Auto optimization disabled');
      _autoOptimizationTimer?.cancel();
      _autoOptimizationTimer = null;
    }
  }
  
  /// 更改自动优化间隔
  void _changeAutoOptimizationInterval(Duration newInterval) {
    setState(() {
      _autoOptimizationInterval = newInterval;
    });
    
    // 如果正在运行，重启定时器
    if (_isAutoOptimizationEnabled) {
      _autoOptimizationTimer?.cancel();
      _autoOptimizationTimer = Timer.periodic(_autoOptimizationInterval, (timer) async {
        _addLog('⏰ Auto optimization triggered');
        await _manualOptimization();
      });
      
      _addLog('🔄 Auto optimization interval changed to ${newInterval.inSeconds}s');
    }
  }
  
  /// 重置统计
  void _resetStatistics() {
    setState(() {
      _optimizationCount = 0;
      _totalMemoryFreedMB = 0;
      _recentOptimizations.clear();
      _peakMemoryMB = _currentMemoryMB;
      _logs.clear();
    });
    _addLog('🔄 Statistics reset');
  }
  
  /// 添加日志
  void _addLog(String message) {
    final timestamp = DateTime.now().toString().substring(11, 19);
    setState(() {
      _logs.add('[$timestamp] $message');
      if (_logs.length > 100) {
        _logs.removeAt(0);
      }
    });
    
    // 自动滚动到底部
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (_logScrollController.hasClients) {
        _logScrollController.animateTo(
          _logScrollController.position.maxScrollExtent,
          duration: const Duration(milliseconds: 200),
          curve: Curves.easeOut,
        );
      }
    });
  }
  
  @override
  Widget build(BuildContext context) {
    final avgOptimization = _recentOptimizations.isEmpty
        ? 0.0
        : _recentOptimizations.reduce((a, b) => a + b) / _recentOptimizations.length;
    
    return Scaffold(
      appBar: AppBar(
        title: const Text('Memory Optimization Test'),
        backgroundColor: Colors.blue,
      ),
      body: Column(
        children: [
          // 内存状态卡片
          _buildMemoryStatusCard(),
          
          // 优化统计卡片
          _buildStatisticsCard(avgOptimization),
          
          // 控制按钮
          _buildControlPanel(),
          
          // 日志输出
          Expanded(
            child: _buildLogPanel(),
          ),
        ],
      ),
    );
  }
  
  Widget _buildMemoryStatusCard() {
    final memoryUsagePercent = (_currentMemoryMB / 1024 * 100).clamp(0.0, 100.0);
    
    return Card(
      margin: const EdgeInsets.all(16),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              '💾 Memory Status',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _buildMemoryInfo('Current', '$_currentMemoryMB MB', Colors.blue),
                _buildMemoryInfo('Peak', '$_peakMemoryMB MB', Colors.orange),
                _buildMemoryInfo('Usage', '${memoryUsagePercent.toStringAsFixed(1)}%', Colors.purple),
              ],
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildMemoryInfo(String label, String value, Color color) {
    return Column(
      children: [
        Text(
          label,
          style: TextStyle(fontSize: 12, color: Colors.grey[600]),
        ),
        const SizedBox(height: 4),
        Text(
          value,
          style: TextStyle(
            fontSize: 16,
            fontWeight: FontWeight.bold,
            color: color,
          ),
        ),
      ],
    );
  }
  
  Widget _buildStatisticsCard(double avgOptimization) {
    return Card(
      margin: const EdgeInsets.symmetric(horizontal: 16),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              '📊 Optimization Statistics',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                _buildStatInfo('Count', '$_optimizationCount', Colors.green),
                _buildStatInfo('Total Freed', '$_totalMemoryFreedMB MB', Colors.blue),
                _buildStatInfo('Avg Freed', '${avgOptimization.toStringAsFixed(1)} MB', Colors.orange),
              ],
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildStatInfo(String label, String value, Color color) {
    return Column(
      children: [
        Text(
          label,
          style: TextStyle(fontSize: 12, color: Colors.grey[600]),
        ),
        const SizedBox(height: 4),
        Text(
          value,
          style: TextStyle(
            fontSize: 14,
            fontWeight: FontWeight.bold,
            color: color,
          ),
        ),
      ],
    );
  }
  
  Widget _buildControlPanel() {
    return Card(
      margin: const EdgeInsets.all(16),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              '🎮 Control Panel',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            
            // 手动优化按钮
            SizedBox(
              width: double.infinity,
              child: ElevatedButton.icon(
                onPressed: _manualOptimization,
                icon: const Icon(Icons.cleaning_services),
                label: const Text('Manual Optimize'),
                style: ElevatedButton.styleFrom(
                  backgroundColor: Colors.blue,
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.all(12),
                ),
              ),
            ),
            
            const SizedBox(height: 8),
            
            // 自动优化开关
            SwitchListTile(
              title: const Text('Auto Optimization'),
              subtitle: Text('Interval: ${_autoOptimizationInterval.inSeconds}s'),
              value: _isAutoOptimizationEnabled,
              onChanged: (value) => _toggleAutoOptimization(),
            ),
            
            // 间隔选择
            if (_isAutoOptimizationEnabled)
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  _buildIntervalButton('30s', const Duration(seconds: 30)),
                  _buildIntervalButton('1m', const Duration(minutes: 1)),
                  _buildIntervalButton('3m', const Duration(minutes: 3)),
                  _buildIntervalButton('5m', const Duration(minutes: 5)),
                ],
              ),
            
            const SizedBox(height: 8),
            
            // 重置统计
            SizedBox(
              width: double.infinity,
              child: OutlinedButton.icon(
                onPressed: _resetStatistics,
                icon: const Icon(Icons.refresh),
                label: const Text('Reset Statistics'),
              ),
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildIntervalButton(String label, Duration duration) {
    final isSelected = _autoOptimizationInterval == duration;
    
    return ElevatedButton(
      onPressed: () => _changeAutoOptimizationInterval(duration),
      style: ElevatedButton.styleFrom(
        backgroundColor: isSelected ? Colors.blue : Colors.grey[300],
        foregroundColor: isSelected ? Colors.white : Colors.black,
      ),
      child: Text(label),
    );
  }
  
  Widget _buildLogPanel() {
    return Card(
      margin: const EdgeInsets.fromLTRB(16, 0, 16, 16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Padding(
            padding: const EdgeInsets.all(12),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text(
                  '📝 Logs',
                  style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
                ),
                TextButton(
                  onPressed: () {
                    setState(() {
                      _logs.clear();
                    });
                  },
                  child: const Text('Clear'),
                ),
              ],
            ),
          ),
          const Divider(height: 1),
          Expanded(
            child: ListView.builder(
              controller: _logScrollController,
              padding: const EdgeInsets.all(8),
              itemCount: _logs.length,
              itemBuilder: (context, index) {
                return Padding(
                  padding: const EdgeInsets.symmetric(vertical: 2),
                  child: Text(
                    _logs[index],
                    style: const TextStyle(
                      fontSize: 12,
                      fontFamily: 'monospace',
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

