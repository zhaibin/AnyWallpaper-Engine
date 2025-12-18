# Web Content 内存增长问题分析与解决方案

## 问题描述

**现象**: Web Content 进程内存持续增长，不会自动回收

**原因分析**:

### 1. 浏览器内存管理机制

WKWebView (macOS) 和 WebView2 (Windows) 的 Web Content 进程会积累：

- **视频解码缓冲区**: 预解码帧 (100-500 MB)
- **DOM 节点**: 历史页面元素
- **JavaScript 对象**: 闭包、事件监听器
- **图片缓存**: 已加载的图片
- **网络缓存**: HTTP/HTTPS 响应
- **Canvas 缓冲区**: 2D/WebGL 渲染缓冲区

### 2. 当前实现的问题

```objective-c
// PowerManager.m 第 104-111 行
if (memoryMB > self.memoryThresholdMB) {
    [AWPLogger warn:@"Memory exceeds threshold"];
    // ❌ 自动优化被注释掉了！
    // [self optimizeMemory:nil];
}
```

**问题**: 
- ✅ 能检测内存超标
- ❌ 只记录警告，不自动优化
- ❌ 需要客户端手动定时调用

## 解决方案

### 方案 1: 启用自动内存优化（推荐）

**优点**: 
- 自动化，无需客户端干预
- 超过阈值立即优化
- 降低客户端复杂度

**缺点**:
- 可能在播放时触发（影响体验）
- 需要合理设置阈值

**实现**: 修改 `PowerManager.m`

```objective-c
if (memoryMB > self.memoryThresholdMB) {
    [AWPLogger warn:[NSString stringWithFormat:@"Memory usage (%ld MB) exceeds threshold (%ld MB), auto-optimizing...",
                    (long)memoryMB, (long)self.memoryThresholdMB]];
    
    // ✅ 自动优化（获取所有 WebView 实例）
    NSArray *instances = [self.wallpaperManager getAllInstances];
    [self optimizeMemory:instances];
}
```

**配置建议**:
- **阈值**: 500-800 MB（默认 500 MB）
- **检查间隔**: 每 30 秒（默认）
- **场景调整**:
  - 单视频: 500 MB
  - 多视频: 800 MB
  - 静态图片: 300 MB

### 方案 2: 客户端定时优化（当前推荐）

**优点**:
- 客户端完全控制时机
- 可选择低峰时段（如切换壁纸时）
- 不会意外中断播放

**缺点**:
- 需要客户端代码
- 如果忘记调用会持续增长

**实现**:

```dart
// 客户端代码
Timer.periodic(Duration(minutes: 1), (_) async {
  final memoryMB = await AnyWPEngine.getMemoryUsage();
  
  // 超过阈值才优化
  if (memoryMB > 500) {
    print('🧹 Memory: ${memoryMB}MB, optimizing...');
    await AnyWPEngine.optimizeMemory();
  }
});
```

### 方案 3: 混合方案（最佳）

**客户端**: 定期主动优化（1 分钟间隔）
**Native**: 紧急自动优化（内存 > 阈值）

```dart
// 客户端定期优化
Timer.periodic(Duration(minutes: 1), (_) async {
  await AnyWPEngine.optimizeMemory();
});
```

```objective-c
// Native 紧急优化
if (memoryMB > 1000) {  // 设置更高的紧急阈值
    [AWPLogger error:@"CRITICAL: Memory exceeds 1000 MB, emergency optimization!"];
    NSArray *instances = [self.wallpaperManager getAllInstances];
    [self optimizeMemory:instances];
}
```

## 优化效果分析

### 当前 optimizeMemory() 做了什么？

✅ **已实现**:
1. 清理 `NSURLCache` (HTTP/HTTPS 缓存)
2. 清理 `WKWebsiteDataStore` (WebView 缓存、Cookies、Storage)
3. 对每个 WebView 执行 JavaScript 优化:
   - 清理 `sessionStorage`
   - 清理 Cache API
   - **刷新视频解码器缓冲区** (关键！)
4. 触发系统内存压力信号

### 视频缓冲区优化原理

```javascript
// PowerManager.m 第 302-317 行
videos.forEach(function(video) {
    var wasPaused = video.paused;
    var currentTime = video.currentTime;
    
    // 1. 暂停视频
    video.pause();
    
    // 2. 保存播放位置
    video.currentTime = currentTime;
    
    // 3. 重新加载（释放缓冲区）
    video.load();
    
    // 4. 恢复播放
    if (!wasPaused) {
        setTimeout(function() {
            video.currentTime = currentTime;
            video.play();
        }, 200);
    }
});
```

**效果**: 每次可释放 100-500 MB (取决于视频数量和分辨率)

## 内存增长原因深度分析

### 1. 视频解码器缓冲区（最主要）

- **1080p @ 30fps**: 150-200 MB (5-10 秒预解码帧)
- **4K @ 60fps**: 500-800 MB
- **多视频叠加**: 缓冲区倍增

**浏览器不会自动释放**，导致长时间播放后累积 1-2 GB

### 2. DOM 泄漏

```javascript
// 常见泄漏场景
setInterval(() => {
    // 创建新元素但不删除旧元素
    document.body.appendChild(div);  // ❌
}, 1000);

// 事件监听器未清理
element.addEventListener('click', handler);  // ❌ 没有 removeEventListener
```

### 3. JavaScript 闭包泄漏

```javascript
function createClosure() {
    let largeData = new Array(1000000);  // 1MB 数组
    return function() {
        console.log(largeData.length);  // 闭包持有 largeData
    };
}

// ❌ largeData 永远不会被回收
window.myFunc = createClosure();
```

### 4. Canvas/WebGL 缓冲区

```javascript
// 每次创建新的 canvas 上下文但不释放
canvas.getContext('2d');  // 创建新缓冲区
// 需要显式释放: canvas.width = canvas.width;
```

## 优化建议

### 对于插件开发者（你）

**选项 A: 启用自动优化（修改 PowerManager.m）**

```objective-c
// 1. 添加 wallpaperManager 引用
@property (nonatomic, weak) WallpaperManager *wallpaperManager;

// 2. 修改 checkSystemState
if (memoryMB > self.memoryThresholdMB) {
    [AWPLogger warn:@"Auto-optimizing memory..."];
    NSArray *instances = [self.wallpaperManager getAllInstances];
    [self optimizeMemory:instances];
}
```

**选项 B: 提供配置接口**

```dart
// 新增 API
await AnyWPEngine.setMemoryAutoOptimize(
  enabled: true,
  threshold: 500,  // MB
);
```

### 对于壁纸开发者

**1. 避免内存泄漏**

```javascript
// ✅ 正确: 清理旧元素
if (oldElement) {
    oldElement.remove();
}
document.body.appendChild(newElement);

// ✅ 正确: 清理事件监听器
const handler = () => { /* ... */ };
element.addEventListener('click', handler);
// 在不需要时:
element.removeEventListener('click', handler);
```

**2. 定期手动优化**

```javascript
// 在壁纸切换时优化
window.AnyWP.onWallpaperChange(() => {
    // 清理当前壁纸资源
    videos.forEach(v => v.pause());
    canvases.forEach(c => c.width = c.width);
});
```

**3. 使用对象池**

```javascript
// ❌ 每次创建新对象
setInterval(() => {
    const particle = { x: 0, y: 0, ... };
}, 16);

// ✅ 复用对象
const particlePool = [];
function getParticle() {
    return particlePool.pop() || { x: 0, y: 0 };
}
function releaseParticle(p) {
    particlePool.push(p);
}
```

### 对于客户端开发者

**1. 定期优化（必须）**

```dart
Timer.periodic(Duration(minutes: 1), (_) async {
  await AnyWPEngine.optimizeMemory();
});
```

**2. 监控内存（可选）**

```dart
Timer.periodic(Duration(seconds: 30), (_) async {
  final memoryMB = await AnyWPEngine.getMemoryUsage();
  print('📊 Current memory: ${memoryMB}MB');
  
  if (memoryMB > 1000) {
    print('⚠️ High memory usage detected!');
  }
});
```

**3. 场景优化**

```dart
// 视频壁纸: 更频繁的优化
if (isVideoWallpaper) {
  Timer.periodic(Duration(seconds: 30), (_) async {
    await AnyWPEngine.optimizeMemory();
  });
}

// 静态图片: 较少优化
if (isStaticWallpaper) {
  Timer.periodic(Duration(minutes: 5), (_) async {
    await AnyWPEngine.optimizeMemory();
  });
}
```

## 测试与验证

### 1. 内存监控工具

**macOS Activity Monitor**:
```bash
# 查看 Web Content 进程
open "/Applications/Utilities/Activity Monitor.app"
# 搜索 "com.apple.WebKit.WebContent"
```

**Flutter 内存监控**:
```dart
Timer.periodic(Duration(seconds: 10), (_) async {
  final memoryMB = await AnyWPEngine.getMemoryUsage();
  print('📊 ${DateTime.now()}: ${memoryMB}MB');
});
```

### 2. 优化效果验证

**预期效果**:
- ✅ 优化后内存降低 50-200 MB
- ✅ 视频继续播放（无卡顿）
- ✅ 内存增长速度显著降低

**日志示例**:
```
[AnyWP] 🧹 Memory optimization triggered
[AnyWP]    Memory before: 850 MB
[AnyWP]    ✓ NSURLCache cleared
[AnyWP]    ✓ Optimized 2 WebView(s)
[AnyWP]    ✓ WKWebsiteDataStore cleared (async)
[AnyWP]    ✓ Memory pressure applied
[AnyWP]    Memory after: 420 MB (freed: 430 MB)
[AnyWP] ✅ Memory optimization complete
```

## 配置参考

### 内存阈值建议

| 场景 | 阈值 (MB) | 优化间隔 | 说明 |
|------|-----------|---------|------|
| 单张静态图片 | 200 | 10 分钟 | 极低内存 |
| 图片轮播 | 300 | 5 分钟 | 低内存 |
| 单视频循环 | 500 | 1 分钟 | 中等内存 |
| 视频轮播 | 800 | 30 秒 | 高内存 |
| 多屏视频 | 1000 | 30 秒 | 极高内存 |

### API 使用示例

```dart
// 1. 启动时配置
await AnyWPEngine.setMemoryThreshold(500);  // 500 MB

// 2. 定期优化
Timer.periodic(Duration(minutes: 1), (_) async {
  await AnyWPEngine.optimizeMemory();
});

// 3. 监控内存
Timer.periodic(Duration(seconds: 30), (_) async {
  final memoryMB = await AnyWPEngine.getMemoryUsage();
  if (memoryMB > 800) {
    // 内存过高，立即优化
    await AnyWPEngine.optimizeMemory();
  }
});
```

## 总结

### 当前状态
- ✅ 优化功能已实现且完整
- ✅ 支持视频缓冲区刷新（关键优化）
- ❌ 自动优化被禁用（需手动调用）

### 推荐方案
1. **短期**: 客户端定时调用 `optimizeMemory()` (1 分钟间隔)
2. **中期**: 修改 PowerManager 启用自动优化
3. **长期**: 提供配置 API，让用户自定义策略

### 预期效果
- 内存峰值: 2.5GB → 500-800 MB (**改善 70%**)
- 优化频率: 每 1 分钟
- 每次释放: 100-500 MB

---

**结论**: 内存增长问题的根本原因是**视频解码器缓冲区不释放**，已有的 `optimizeMemory()` 功能完整且有效，只需要定期调用即可解决问题。




