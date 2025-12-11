# Memory Optimization Test Page

## 功能

这是一个测试 `AnyWPEngine.optimizeMemory()` 效果的调试页面。

## 主要特性

### 1. 实时内存监控
- 当前内存使用 (Current)
- 峰值内存 (Peak)
- 内存使用率百分比

### 2. 手动内存优化
- 一键触发 `optimizeMemory()`
- 显示优化前后内存对比
- 计算释放的内存量

### 3. 自动定时优化
- 可开启/关闭自动优化
- 支持多种时间间隔（30秒、1分钟、3分钟、5分钟）
- 适用于测试不同场景

### 4. 优化统计
- 优化次数
- 总释放内存量
- 平均每次释放内存

### 5. 详细日志
- 时间戳日志
- 优化过程详细信息
- 支持清空日志

## 使用方法

### 集成到 Example App

1. **引入测试页面**

在 `example/lib/main.dart` 中添加：

```dart
import 'pages/memory_optimization_test_page.dart';
```

2. **添加导航按钮**

在合适的位置（如设置页面或调试标签）添加按钮：

```dart
ElevatedButton(
  onPressed: () {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => const MemoryOptimizationTestPage(),
      ),
    );
  },
  child: const Text('Memory Optimization Test'),
),
```

### 独立运行

也可以创建单独的测试应用：

```dart
import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';
import 'pages/memory_optimization_test_page.dart';

void main() {
  runApp(MaterialApp(
    home: const MemoryOptimizationTestPage(),
  ));
}
```

## 测试场景

### 场景 1: 静态图片壁纸
1. 初始化一个静态图片壁纸
2. 关闭自动优化
3. 手动触发优化
4. **预期**: 释放内存 < 10 MB（静态图片无大量缓存）

### 场景 2: 视频壁纸（单视频）
1. 初始化一个 1080p 视频壁纸
2. 播放 5 分钟
3. 手动触发优化
4. **预期**: 释放内存 50-150 MB（视频解码器缓冲区）

### 场景 3: 视频轮播（连续播放）
1. 初始化视频轮播壁纸（3-5 个视频）
2. 开启自动优化（1 分钟间隔）
3. 播放 10 分钟
4. **预期**: 
   - 每次优化释放 100-200 MB
   - 内存稳定在 300-500 MB
   - 无自动优化时会增长至 1.5-2.5 GB

### 场景 4: 多视频同时播放
1. 在多个显示器上初始化视频壁纸
2. 开启自动优化（30 秒间隔）
3. 播放 5 分钟
4. **预期**: 每次优化释放 200-500 MB

## 验证要点

### ✅ 成功标志
- 每次优化后内存明显下降（> 50 MB）
- 内存使用趋势保持稳定（不持续增长）
- 视频播放不卡顿（优化仅 0.1-0.2 秒暂停）
- 日志显示成功清理缓存和视频缓冲区

### ❌ 失败标志
- 优化后内存释放 < 10 MB（可能没有视频或缓存未积累）
- 内存持续增长（优化未生效）
- 视频长时间卡顿（> 1 秒）
- 日志显示错误或异常

## 故障排查

### 问题 1: 优化后内存释放不明显

**可能原因**:
- 壁纸未在播放视频
- 调用频率过高，缓存还未积累
- 系统未回收已释放的内存页

**解决方案**:
- 增加优化间隔（例如 3-5 分钟）
- 检查是否有视频元素在播放
- 等待系统回收（可能需要 10-30 秒）

### 问题 2: 内存监控显示 0 MB

**可能原因**:
- `getMemoryUsage()` API 调用失败
- 平台不支持（不太可能）

**解决方案**:
- 查看日志中的错误信息
- 检查 Flutter 日志: `flutter run -v`

### 问题 3: 自动优化导致视频卡顿

**可能原因**:
- 多个视频同时重载
- 网络加载慢

**解决方案**:
- 增加优化间隔（至少 1 分钟）
- 使用本地视频文件
- 添加视频预加载: `<video preload="auto">`

## 开发者提示

### 日志分析

```
[19:30:00] 🧹 Manual optimization triggered...
[19:30:00]    Memory before: 850 MB
[19:30:02]    Memory after: 650 MB
[19:30:02]    ✅ Freed: 200 MB (took 2100ms)
```

**解读**:
- 优化耗时 2.1 秒（正常，包含 2 秒等待）
- 释放 200 MB（效果显著）
- 建议：视频场景下每 1-2 分钟优化一次

### 性能基准

| 场景 | 初始内存 | 优化后内存 | 释放量 | 推荐间隔 |
|------|---------|-----------|--------|---------|
| 静态图片 | 150 MB | 145 MB | 5 MB | 10 分钟 |
| 单视频 (1080p) | 500 MB | 350 MB | 150 MB | 2 分钟 |
| 视频轮播 (3个) | 800 MB | 450 MB | 350 MB | 1 分钟 |
| 多显示器视频 | 1500 MB | 800 MB | 700 MB | 30 秒 |

## 相关文档

- [Memory Optimization Guide](../../docs/MEMORY_OPTIMIZATION_GUIDE.md)
- [Memory Optimization Fix Summary](../../docs/MEMORY_OPTIMIZATION_FIX_SUMMARY.md)
- [Developer API Reference](../../docs/DEVELOPER_API_REFERENCE.md)

## 版本历史

- **v2.6.6** - 初始版本，支持完整的内存优化测试功能

