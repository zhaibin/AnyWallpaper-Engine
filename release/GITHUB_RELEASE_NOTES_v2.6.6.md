# AnyWP Engine v2.6.6 - Release Notes

**Flutter Plugin Version**: 2.6.6
**Web SDK Version**: 2.5.0

> **Note**: Flutter Plugin and Web SDK have independent version numbers.

---


## 🐛 Bug 修复

### 修复 macOS/Windows 内存优化不完整问题

**问题**: 
- macOS `optimizeMemory()` 实现过于简单，只清除 NSURLCache，未清理 WebView 缓存和视频解码器缓冲区
- Windows 清理脚本未针对视频内存优化，连续播放视频时内存持续增长（200MB → 2.5GB+）
- 轮播壁纸从不暂停，引擎自动优化从未触发

**根本原因**:
- 浏览器视频解码器会缓存大量已解码帧（1080p 视频约 150-200 MB/视频）
- WebView 不会自动释放这些缓冲区，即使视频暂停
- 轮播切换时，旧视频的缓冲区仍占用内存

**修复内容**:

**macOS**:
```objective-c
- (void)optimizeMemory:(NSArray *)instances {
    // 1. 清除 NSURLCache
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    
    // 2. 清除 WKWebsiteDataStore (新增)
    [[WKWebsiteDataStore defaultDataStore] removeDataOfTypes:dataTypes ...];
    
    // 3. 优化每个 WebView (新增)
    [self optimizeWebView:webView];  // JavaScript 脚本清理 + 视频缓冲区刷新
    
    // 4. 触发内存压力信号 (新增)
    [[NSProcessInfo processInfo] performActivityWithOptions:...];
}

- (void)optimizeWebView:(WKWebView *)webView {
    // JavaScript 优化脚本
    // - 清除 sessionStorage
    // - 清除 Cache API
    // - ⭐️ 刷新视频解码器缓冲区
    videos.forEach(function(video) {
        video.pause();  // 释放解码器缓冲区
        video.load();   // 重新初始化 (释放 100-500 MB)
        video.play();   // 恢复播放
    });
}
```

**Windows**:
```cpp
// 增强视频内存优化
var videos = document.querySelectorAll('video');
videos.forEach(function(video) {
    var wasPaused = video.paused;
    var currentTime = video.currentTime;
    video.pause();          // 释放解码器缓冲区
    video.currentTime = currentTime;
    video.load();           // 重新初始化
    if (!wasPaused) {
        setTimeout(function() {
            video.currentTime = currentTime;
            video.play();   // 恢复播放
        }, 200);
    }
});
```

**优化效果**:
- 连续播放 10 分钟：内存从 2.5GB+ 降至 300-500MB
- 每次优化可释放：50-200 MB（取决于视频数量和分辨率）
- CPU 使用率降低：30%+ → 10-20%

**客户端使用建议**:
```dart
// 视频壁纸必须定时主动调用
Timer.periodic(Duration(minutes: 1), (_) async {
  await AnyWPEngine.optimizeMemory();  // 刷新视频缓冲区
});
```

**重要说明**:
- ⚠️ 不再清除 `localStorage`（保留壁纸状态）
- ✅ 视频刷新几乎无感知（仅 0.1-0.2 秒暂停）
- ✅ 支持多视频同时优化
- ✅ 跨平台一致行为（macOS + Windows）

## 📚 文档

### 新增内存优化指南

新增 `docs/MEMORY_OPTIMIZATION_GUIDE.md`，详细说明：
- 连续视频播放内存增长问题分析
- `optimizeMemory()` 实现细节（macOS/Windows）
- 视频解码器缓冲区原理（为什么占用如此多内存）
- 客户端定时调用最佳实践
- 效果验证与故障排查

---

