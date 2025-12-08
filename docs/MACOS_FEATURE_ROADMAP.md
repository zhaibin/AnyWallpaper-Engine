# macOS 功能完整度提升计划

**目标**：将 macOS 版本从 85% 提升到 100%，完全追齐 Windows 功能  
**当前版本**：v2.6.0  
**目标版本**：v2.7.0+

---

## 📊 当前功能差距分析

| 功能模块 | Windows | macOS | 差距 |
|---------|---------|-------|------|
| **交互模式** | ✅ | ❌ | 15% |
| **全局鼠标钩子** | ✅ | ❌ | - |
| **全局键盘监听** | ✅ | ❌ | - |
| **本地文件服务器** | ✅ | ❌ | - |
| **轮询备份机制** | ✅ | N/A | - |

**总体功能完整度**：85% → 目标 100%

---

## 🎯 需要实现的 4 大功能

### 1. 交互模式（Interactive Mode）⭐⭐⭐

**优先级**：最高  
**难度**：★★★★☆  
**预计工时**：5-7 天

#### 功能描述

允许壁纸窗口接收鼠标和键盘输入，实现真正的交互式壁纸（游戏、工具等）。

#### Windows 实现原理

```cpp
// 1. 窗口样式切换
bool SetInteractive(HWND hwnd, bool interactive) {
  LONG ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE);
  
  if (interactive) {
    // 移除 WS_EX_TRANSPARENT - 窗口可接收鼠标事件
    ex_style &= ~WS_EX_TRANSPARENT;
  } else {
    // 添加 WS_EX_TRANSPARENT - 鼠标穿透
    ex_style |= WS_EX_TRANSPARENT;
  }
  
  // 始终保持 WS_EX_NOACTIVATE 防止抢焦点
  ex_style |= WS_EX_NOACTIVATE;
  SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style);
}

// 2. 全局鼠标钩子
HHOOK hook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, ...);

// 3. 全局键盘钩子
HHOOK hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, ...);
```

**关键技术**：
- 全局钩子捕获所有输入事件
- 判断事件是否发生在壁纸窗口区域
- 将事件转发到 WebView
- 防止干扰其他应用

#### macOS 实现方案

**挑战**：
- ❌ macOS 没有类似 Windows 的全局钩子 API
- ❌ 需要 **Accessibility 权限**（侵入性强，用户体验差）
- ❌ 系统安全策略限制

**技术方案**：

##### 方案 A：基于 Accessibility API（完整方案）

```objc
// 1. 请求 Accessibility 权限
- (BOOL)requestAccessibilityPermission {
    NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt: @YES};
    return AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

// 2. 创建全局事件监听器（需要 Accessibility 权限）
- (void)installGlobalEventMonitors {
    // 鼠标事件监听
    CGEventMask eventMask = 
        CGEventMaskBit(kCGEventLeftMouseDown) |
        CGEventMaskBit(kCGEventLeftMouseUp) |
        CGEventMaskBit(kCGEventRightMouseDown) |
        CGEventMaskBit(kCGEventRightMouseUp) |
        CGEventMaskBit(kCGEventMouseMoved) |
        CGEventMaskBit(kCGEventLeftMouseDragged);
    
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        eventMask,
        mouseEventCallback,
        (__bridge void *)self
    );
    
    // 键盘事件监听
    CGEventMask keyEventMask = 
        CGEventMaskBit(kCGEventKeyDown) |
        CGEventMaskBit(kCGEventKeyUp);
    
    CFMachPortRef keyEventTap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        keyEventMask,
        keyboardEventCallback,
        (__bridge void *)self
    );
}

// 3. 鼠标事件回调
CGEventRef mouseEventCallback(CGEventTapProxy proxy, 
                              CGEventType type,
                              CGEventRef event,
                              void *refcon) {
    InteractiveMode *self = (__bridge InteractiveMode *)refcon;
    
    // 获取事件位置
    CGPoint location = CGEventGetLocation(event);
    
    // 判断是否在壁纸窗口区域
    if ([self isPointInWallpaperWindow:location]) {
        // 转发事件到 WKWebView
        [self forwardMouseEventToWebView:event];
    }
    
    // 返回事件（不拦截，让其他应用也能收到）
    return event;
}

// 4. 键盘事件回调
CGEventRef keyboardEventCallback(CGEventTapProxy proxy,
                                 CGEventType type,
                                 CGEventRef event,
                                 void *refcon) {
    InteractiveMode *self = (__bridge InteractiveMode *)refcon;
    
    if (self.interactiveMode) {
        // 转发到 WKWebView
        [self forwardKeyboardEventToWebView:event];
    }
    
    return event;
}
```

**优点**：
- ✅ 完整的交互模式支持
- ✅ 与 Windows 功能对齐

**缺点**：
- ❌ 需要用户授予 Accessibility 权限（隐私担忧）
- ❌ 权限提示对用户体验影响大
- ❌ 部分企业环境禁止 Accessibility 权限
- ❌ 实现复杂度高

##### 方案 B：本地事件监听器（受限方案）

```objc
// 只监听应用内事件（不需要 Accessibility 权限）
- (void)installLocalEventMonitors {
    // 只能监听应用激活时的事件
    [NSEvent addLocalMonitorForEventsMatchingMask:
        NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp |
        NSEventMaskRightMouseDown | NSEventMaskRightMouseUp |
        NSEventMaskMouseMoved | NSEventMaskLeftMouseDragged
        handler:^NSEvent *(NSEvent *event) {
            [self handleLocalMouseEvent:event];
            return event;
        }];
}
```

**优点**：
- ✅ 无需特殊权限
- ✅ 用户体验友好

**缺点**：
- ❌ 只能监听应用激活时的事件
- ❌ 壁纸窗口通常不激活，监听不到事件
- ❌ 功能严重受限

##### 方案 C：WKWebView 原生交互（推荐方案）⭐

```objc
// 让 WKWebView 窗口接收事件（无需全局钩子）
- (void)enableInteractiveMode:(BOOL)enabled {
    if (enabled) {
        // 1. 移除 NSWindowStyleMaskNonactivatingPanel
        self.wallpaperWindow.styleMask &= ~NSWindowStyleMaskNonactivatingPanel;
        
        // 2. 设置窗口可接收鼠标事件
        [self.wallpaperWindow setIgnoresMouseEvents:NO];
        
        // 3. 设置 WKWebView 可交互
        [self.webView setUserInteractionEnabled:YES];
        
        // 4. 保持在桌面层（但可接收事件）
        self.wallpaperWindow.level = kCGDesktopWindowLevel + 1;
        
        [AWPLogger log:@"Interactive mode ENABLED"];
    } else {
        // 恢复壁纸模式
        [self.wallpaperWindow setIgnoresMouseEvents:YES];
        [self.webView setUserInteractionEnabled:NO];
        [AWPLogger log:@"Interactive mode DISABLED"];
    }
}
```

**优点**：
- ✅ **无需 Accessibility 权限** ⭐
- ✅ 实现简单
- ✅ 用户体验友好
- ✅ 可满足大部分交互需求

**缺点**：
- ❌ 窗口激活时可能影响桌面图标点击
- ❌ 无法实现复杂的全局快捷键

**推荐**：优先实现方案 C，作为可选功能提供方案 A

#### 实现步骤

1. **创建 InteractiveMode 模块** (2天)
   - `InteractiveMode.h/m`
   - 实现方案 C（基础交互）
   - 添加 API：`setInteractiveMode(bool enabled)`

2. **可选：实现 Accessibility 方案** (3天)
   - 权限检查和请求
   - 全局事件监听
   - 事件转发机制

3. **集成到 WallpaperManager** (1天)
   - 添加交互模式开关
   - 更新 Flutter API

4. **测试和文档** (1天)
   - 编写测试用例
   - 更新开发者文档

---

### 2. 全局键盘监听（Keyboard Monitoring）⭐⭐

**优先级**：高  
**难度**：★★★★☆  
**预计工时**：3-4 天

#### 功能描述

监听全局键盘事件（快捷键、游戏控制等）。

#### Windows 实现原理

```cpp
// 全局键盘钩子
HHOOK hook = SetWindowsHookExW(
    WH_KEYBOARD_LL,
    LowLevelKeyboardProc,
    GetModuleHandle(nullptr),
    0
);

// 回调函数
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT* info = (KBDLLHOOKSTRUCT*)lParam;
    
    // 判断事件类型
    if (wParam == WM_KEYDOWN) {
        // 发送到 Flutter
        SendKeyboardEvent("keydown", info->vkCode, ...);
    }
    
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
```

#### macOS 实现方案

**方案 A：CGEventTap（需要 Accessibility）**

```objc
// 创建键盘事件监听器
CGEventMask keyEventMask = 
    CGEventMaskBit(kCGEventKeyDown) |
    CGEventMaskBit(kCGEventKeyUp) |
    CGEventMaskBit(kCGEventFlagsChanged);

CFMachPortRef keyEventTap = CGEventTapCreate(
    kCGSessionEventTap,
    kCGHeadInsertEventTap,
    kCGEventTapOptionListenOnly,  // 只监听，不拦截
    keyEventMask,
    keyboardEventCallback,
    (__bridge void *)self
);

// 回调
CGEventRef keyboardEventCallback(CGEventTapProxy proxy,
                                 CGEventType type,
                                 CGEventRef event,
                                 void *refcon) {
    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(
        event, kCGKeyboardEventKeycode);
    
    // 发送到 Flutter
    [self sendKeyboardEventToFlutter:keyCode type:type];
    
    return event;  // 不拦截
}
```

**方案 B：NSEvent 全局监听（需要 Accessibility）**

```objc
// 全局监听（需要权限）
id globalMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:
    NSEventMaskKeyDown | NSEventMaskKeyUp
    handler:^(NSEvent *event) {
        [self handleKeyboardEvent:event];
    }];
```

**推荐**：使用方案 A（CGEventTap），与交互模式共用权限

#### 实现步骤

1. **复用 InteractiveMode 的权限检查** (0.5天)
2. **实现键盘事件监听** (1.5天)
3. **事件格式转换和传递** (1天)
4. **测试和文档** (1天)

---

### 3. 本地文件服务器（Local File Server）⭐

**优先级**：中  
**难度**：★★★☆☆  
**预计工时**：3-4 天

#### 功能描述

提供本地 HTTP 服务器，解决 CORS 跨域问题，支持加载本地资源。

#### Windows 实现原理

```cpp
class LocalFileServer {
public:
    bool Start(const std::string& root_directory, int port) {
        // 1. 创建 HTTP 服务器（使用 Windows HTTP Server API）
        HttpInitialize(...);
        HttpCreateHttpHandle(&server_handle_, 0);
        
        // 2. 注册 URL 前缀
        HttpAddUrl(server_handle_, L"http://localhost:8080/", ...);
        
        // 3. 启动监听线程
        std::thread([this]() {
            while (running_) {
                // 接收请求
                HttpReceiveHttpRequest(...);
                
                // 处理请求
                HandleRequest(request);
                
                // 发送响应
                HttpSendHttpResponse(...);
            }
        }).detach();
    }
    
private:
    void HandleRequest(HTTP_REQUEST* request) {
        // 1. 解析 URL，获取文件路径
        std::string path = ParseFilePath(request->pRawUrl);
        
        // 2. 读取文件
        std::vector<char> content = ReadFile(path);
        
        // 3. 检测 MIME 类型
        std::string mimeType = DetectMimeType(path);
        
        // 4. 添加 CORS 头
        AddCorsHeaders(response);
        
        // 5. 发送响应
        SendResponse(content, mimeType);
    }
};
```

**特性**：
- 自动分配端口
- CORS 头支持
- MIME 类型检测
- 目录浏览
- 文件缓存

#### macOS 实现方案

**方案 A：基于 GCDWebServer（推荐）⭐**

```objc
#import "GCDWebServer.h"
#import "GCDWebServerDataResponse.h"

@implementation LocalFileServer

- (BOOL)startWithRootDirectory:(NSString *)rootPath port:(NSUInteger)port {
    self.webServer = [[GCDWebServer alloc] init];
    
    // 添加文件处理器
    [self.webServer addGETHandlerForBasePath:@"/"
                                directoryPath:rootPath
                                indexFilename:nil
                                     cacheAge:3600
                               allowRangeRequests:YES];
    
    // 添加 CORS 支持
    [self.webServer addDefaultHandlerForMethod:@"OPTIONS"
                                  requestClass:[GCDWebServerRequest class]
                                  processBlock:^GCDWebServerResponse *(GCDWebServerRequest *request) {
        GCDWebServerResponse *response = [GCDWebServerResponse response];
        [response setValue:@"*" forAdditionalHeader:@"Access-Control-Allow-Origin"];
        [response setValue:@"GET, POST, OPTIONS" forAdditionalHeader:@"Access-Control-Allow-Methods"];
        return response;
    }];
    
    // 启动服务器
    return [self.webServer startWithPort:port bonjourName:nil];
}

- (void)stop {
    if (self.webServer.isRunning) {
        [self.webServer stop];
    }
}

- (NSUInteger)port {
    return self.webServer.port;
}

@end
```

**方案 B：基于 NSURLProtocol（轻量级）**

```objc
// 注册自定义 URL 协议
[NSURLProtocol registerClass:[LocalFileProtocol class]];

@implementation LocalFileProtocol

+ (BOOL)canInitWithRequest:(NSURLRequest *)request {
    // 拦截 localfile:// 协议
    return [request.URL.scheme isEqualToString:@"localfile"];
}

- (void)startLoading {
    // 读取本地文件
    NSString *filePath = [self.request.URL.path stringByRemovingPercentEncoding];
    NSData *data = [NSData dataWithContentsOfFile:filePath];
    
    // 创建响应
    NSURLResponse *response = [[NSURLResponse alloc] 
        initWithURL:self.request.URL
        MIMEType:[self detectMIMEType:filePath]
        expectedContentLength:data.length
        textEncodingName:nil];
    
    // 发送响应
    [self.client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageNotAllowed];
    [self.client URLProtocol:self didLoadData:data];
    [self.client URLProtocolDidFinishLoading:self];
}

@end
```

**推荐**：
- **简单场景**：使用方案 B（NSURLProtocol）
- **完整功能**：使用方案 A（GCDWebServer）

#### 实现步骤

1. **选择方案并添加依赖** (0.5天)
   - CocoaPods: `pod 'GCDWebServer'`

2. **实现 LocalFileServer 模块** (1.5天)
   - `LocalFileServer.h/m`
   - 启动/停止服务器
   - CORS 配置

3. **集成到 Flutter API** (1天)
   - 添加 `startFileServer(path, port)`
   - 添加 `stopFileServer()`

4. **测试和文档** (1天)

---

### 4. 鼠标事件轮询备份（Polling Fallback）⭐

**优先级**：低  
**难度**：★★☆☆☆  
**预计工时**：2-3 天

#### 功能描述

当其他程序干扰全局鼠标钩子时，使用轮询机制作为备份（仅 Windows 特有问题）。

#### Windows 实现原理

```cpp
// v2.5.1+ 轮询备份机制
void MouseHookManager::StartPollingThread() {
    if (polling_thread_running_) return;
    
    polling_thread_should_stop_ = false;
    polling_thread_ = std::thread([this]() {
        while (!polling_thread_should_stop_) {
            if (!is_mouse_down_) {
                Sleep(100);  // 鼠标未按下，低频检查
                continue;
            }
            
            // 检查钩子是否被干扰（超过50ms没收到mousemove）
            DWORD now = GetTickCount();
            DWORD last_move = last_hook_mousemove_time_.load();
            if (now - last_move > 50) {
                // 钩子被干扰，使用轮询
                POINT cursor_pos;
                GetCursorPos(&cursor_pos);
                
                if (cursor_pos.x != last_polled_position_.x ||
                    cursor_pos.y != last_polled_position_.y) {
                    // 鼠标位置改变，添加到队列
                    QueuePolledMouseEvent(cursor_pos);
                    last_polled_position_ = cursor_pos;
                }
            }
            
            Sleep(polling_interval_ms_);  // 默认16ms (60fps)
        }
    });
}
```

**问题背景**：
- Lively Wallpaper 等程序使用全局钩子
- 可能干扰其他程序的鼠标事件
- Windows 特有问题

#### macOS 是否需要？

**结论**：❌ **不需要**

**原因**：
1. macOS 没有全局钩子干扰问题
2. CGEventTap 机制不会相互干扰
3. 如果使用 WKWebView 原生交互，更不需要轮询
4. 增加复杂度，收益很小

**建议**：跳过此功能

---

## 📋 实现路线图

### 阶段 1：基础交互模式 (v2.7.0)

**时间**：2-3 周  
**功能**：
- ✅ WKWebView 原生交互（方案 C）
- ✅ 基本的鼠标点击和键盘输入
- ✅ 无需 Accessibility 权限

**优先级**：最高  
**风险**：低

### 阶段 2：本地文件服务器 (v2.7.1)

**时间**：1 周  
**功能**：
- ✅ 基于 GCDWebServer 或 NSURLProtocol
- ✅ CORS 支持
- ✅ MIME 类型检测

**优先级**：中  
**风险**：低

### 阶段 3：高级交互模式（可选）(v2.8.0)

**时间**：1-2 周  
**功能**：
- ✅ 基于 CGEventTap 的全局监听
- ✅ Accessibility 权限管理
- ✅ 全局键盘快捷键

**优先级**：低（可选功能）  
**风险**：中（用户权限问题）

---

## 🎯 功能完整度里程碑

### v2.7.0（目标：95%）
- ✅ WKWebView 原生交互模式
- ✅ 基本鼠标和键盘输入
- ❌ 全局事件监听（不实现）

### v2.7.1（目标：98%）
- ✅ 本地文件服务器
- ✅ CORS 解决方案

### v2.8.0（目标：100%）
- ✅ 高级交互模式（可选）
- ✅ Accessibility 权限支持
- ✅ 全局键盘快捷键

---

## ⚠️ 技术挑战和风险

### 1. Accessibility 权限问题

**挑战**：
- 用户需要手动授予权限
- 权限提示影响用户体验
- 企业环境可能禁用

**缓解措施**：
- 提供无权限的基础交互模式
- 清晰的文档说明
- 优雅的权限请求流程

### 2. 窗口层级管理

**挑战**：
- 交互窗口可能遮挡桌面图标
- 需要平衡交互性和非侵入性

**缓解措施**：
- 提供交互模式开关
- 允许用户自定义窗口行为
- 完善的文档和示例

### 3. 性能影响

**挑战**：
- 全局事件监听可能影响性能
- 文件服务器增加内存占用

**缓解措施**：
- 按需启用功能
- 优化事件处理逻辑
- 轻量级实现方案

---

## 📊 开发工作量估算

| 功能 | 开发 | 测试 | 文档 | 总计 |
|------|------|------|------|------|
| **基础交互模式** | 5天 | 2天 | 1天 | 8天 |
| **键盘监听** | 3天 | 1天 | 0.5天 | 4.5天 |
| **文件服务器** | 3天 | 1天 | 1天 | 5天 |
| **高级交互（可选）** | 5天 | 2天 | 1天 | 8天 |
| **总计** | **16天** | **6天** | **3.5天** | **25.5天** |

**不含可选功能**：17.5天（约 3.5 周）  
**含可选功能**：25.5天（约 5 周）

---

## 🎁 额外优化机会

### 1. 性能优化
- WKWebView 内存优化（目标：降低到 120-150MB）
- 启动时间优化（目标：< 300ms）

### 2. 开发体验
- 添加更多示例应用
- 改进调试工具
- 完善错误提示

### 3. 文档完善
- 交互模式最佳实践
- 性能优化指南
- 故障排查文档

---

## 📝 总结

### 推荐实施策略

**阶段 1（必须）**：
1. ✅ WKWebView 原生交互模式（无需权限）
2. ✅ 本地文件服务器（解决 CORS）

**预期完整度**：95%+

**阶段 2（可选）**：
1. ⚠️ CGEventTap 全局监听（需权限）
2. ⚠️ 高级键盘快捷键

**预期完整度**：100%

### 优先级建议

1. **先实现无权限方案**
   - 更好的用户体验
   - 更快的开发速度
   - 满足大部分需求

2. **文档要清晰说明限制**
   - 哪些功能需要权限
   - 为什么需要权限
   - 如何授予权限

3. **提供可选的高级功能**
   - 默认不启用
   - 由开发者按需启用
   - 完善的文档支持

---

**文档版本**：v1.0  
**创建日期**：2025-12-08  
**维护者**：AnyWP Engine Team

