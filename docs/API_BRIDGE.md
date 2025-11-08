# AnyWP API Bridge - Technical Implementation

## 🌉 JavaScript Bridge Architecture

AnyWP Engine provides a complete JavaScript Bridge for web wallpapers to communicate with the native client.

**Document Type:** Technical implementation details for developers who want to understand the underlying architecture.

**For Usage Guide:** See [Web Developer Guide](WEB_DEVELOPER_GUIDE.md) or [Web Developer Guide (中文)](WEB_DEVELOPER_GUIDE_CN.md)

### Core Mechanism (v1.3.2+ Modular)

```
Desktop Click
     ↓
Windows Mouse Hook (WH_MOUSE_LL)
     ↓
MouseHookManager (windows/modules/mouse_hook_manager.cpp)
     ↓
Core Plugin → Dispatch Event
     ↓
SDKBridge (windows/modules/sdk_bridge.cpp)
     ↓
WebView2 ExecuteScript (AnyWP:mouse + AnyWP:click)
     ↓
WebView2 JavaScript Runtime
     ↓
AnyWP SDK (v4.2.0) - windows/anywp_sdk.js
     ↓
onClick Handler Match
     ↓
Callback Triggered
```

**模块化说明 (v1.3.2+)**:

- **SDKBridge** (`windows/modules/sdk_bridge.cpp`): 负责 SDK 注入、消息监听、事件分发
- **IframeDetector** (`windows/modules/iframe_detector.cpp`): 处理 iframe 坐标映射和边界检测
- **Core Plugin** (`windows/anywp_engine_plugin.cpp`): 协调各模块，提供统一接口

---

## 📡 Supported API Channels

### From Web to Native

#### 1. AnyWP.openURL(url)
**Web Call**:
```javascript
AnyWP.openURL('https://www.bing.com');
```

**Native Handler**:
```cpp
HandleWebMessage() {
  // Parse: {"type":"openURL","url":"..."}
  std::wstring wurl(url.begin(), url.end());
  ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}
```

**Effect**: Opens URL in system default browser

---

#### 2. AnyWP.ready(name)
**Web Call**:
```javascript
AnyWP.ready('Weather Wallpaper v1.0');
```

**Native Handler**:
```cpp
HandleWebMessage() {
  // Parse: {"type":"ready","name":"..."}
  std::cout << "[AnyWP] [API] Wallpaper ready: " << name << std::endl;
}
```

**Effect**: Notifies client that wallpaper has loaded

---

### From Native to Web

#### 3. Mouse Events (AnyWP:mouse + AnyWP:click)
**Native Capture**:
```cpp
// Windows Mouse Hook
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && hook_instance_) {
    if (hook_instance_->is_paused_) {
      return CallNextHookEx(nullptr, nCode, wParam, lParam);  // Skip when paused
    }
    
    MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    POINT pt = info->pt;
    
    if (wParam == WM_LBUTTONUP) {
      hook_instance_->SendClickToWebView(pt.x, pt.y, "mouseup");
    }
  }
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
```

**Sent to Web**:
```javascript
// AnyWP:mouse event (for general mouse handling)
window.dispatchEvent(new CustomEvent('AnyWP:mouse', {
  detail: {
    type: 'mouseup',  // or 'mousedown'
    x: 3200,          // Physical pixels
    y: 1600,
    button: 0         // 0=left, 1=middle, 2=right
  }
}));

// AnyWP:click event (for onClick handlers)
window.dispatchEvent(new CustomEvent('AnyWP:click', {
  detail: {
    x: 3200,
    y: 1600
  }
}));
```

**SDK Handler**:
```javascript
window.addEventListener('AnyWP:click', (event) => {
  this._handleClick(event.detail.x, event.detail.y);
});
```

**Effect**: SDK checks if click is within registered element bounds and triggers callback

---

#### 4. Interaction Mode (AnyWP:interactionMode)
**Native Send**:
```cpp
std::wstringstream script;
script << L"(function() {"
       << L"  if (window.AnyWP) {"
       << L"    var event = new CustomEvent('AnyWP:interactionMode', {"
       << L"      detail: { enabled: " << (enable_interaction_ ? L"true" : L"false") << L" }"
       << L"    });"
       << L"    window.dispatchEvent(event);"
       << L"  }"
       << L"})();";

webview_->ExecuteScript(script.str().c_str(), nullptr);
```

**SDK Receive**:
```javascript
window.addEventListener('AnyWP:interactionMode', (event) => {
  AnyWP.interactionEnabled = event.detail.enabled;
  console.log('Interaction mode:', event.detail.enabled ? 'ON' : 'OFF');
});
```

**Effect**: Controls whether to process mouse/keyboard events

---

#### 5. Visibility Events (AnyWP:visibility) 🆕 v4.1.0
**Native Send**:
```cpp
void NotifyWebContentVisibility(bool visible) {
  std::wstring visibility_script = L"(function() {"
    L"  var event = new Event('visibilitychange');"
    L"  document.dispatchEvent(event);"
    L"  "
    L"  var customEvent = new CustomEvent('AnyWP:visibility', {"
    L"    detail: { visible: " + std::wstring(visible ? L"true" : L"false") + L" }"
    L"  });"
    L"  window.dispatchEvent(customEvent);"
    L"})();";
  
  webview->ExecuteScript(visibility_script.c_str(), nullptr);
}
```

**SDK Handler**:
```javascript
window.addEventListener('AnyWP:visibility', (event) => {
  const visible = event.detail.visible;
  
  // Auto-pause videos/audio
  if (!visible) {
    pauseAllMedia();
  } else {
    resumeAllMedia();
  }
  
  // User callback
  if (this._visibilityCallback) {
    this._visibilityCallback(visible);
  }
});
```

**Effect**: Power saving - auto-pause media when wallpaper is hidden

---

## 🎯 Complete Interaction Flow

### Example: User Clicks Button

#### 1. Web Page Registers Click Area
```javascript
AnyWP.onClick('#btn-weather', (x, y) => {
  console.log('Weather button clicked!');
  AnyWP.openURL('https://weather.com');
});
```

#### 2. SDK Calculates Physical Pixel Bounds
```javascript
_calculateElementBounds(element) {
  const rect = element.getBoundingClientRect();
  const dpi = this.dpiScale;
  return {
    left: Math.round(rect.left * dpi),    // CSS -> Physical
    top: Math.round(rect.top * dpi),
    right: Math.round(rect.right * dpi),
    bottom: Math.round(rect.bottom * dpi),
    width: Math.round(rect.width * dpi),
    height: Math.round(rect.height * dpi)
  };
}
// Stored in _clickHandlers[]
```

#### 3. User Clicks Desktop
```
User clicks desktop at (3200, 1600)
     ↓
Windows Hook captures WM_LBUTTONUP
     ↓
C++: LowLevelMouseProc(WM_LBUTTONUP, {pt: {x:3200, y:1600}})
     ↓
Check if paused (skip if is_paused_ == true)
     ↓
Check if occluded by app window (skip if yes)
```

#### 4. Native Forwards to Web
```cpp
SendClickToWebView(3200, 1600, "mouseup");
  ↓
ExecuteScript("
  // Dispatch mouse event
  window.dispatchEvent(new CustomEvent('AnyWP:mouse', {
    detail: {type: 'mouseup', x: 3200, y: 1600, button: 0}
  }));
  
  // Dispatch click event
  window.dispatchEvent(new CustomEvent('AnyWP:click', {
    detail: {x: 3200, y: 1600}
  }));
");
```

#### 5. SDK Processes Click
```javascript
// SDK event listener
window.addEventListener('AnyWP:click', (event) => {
  this._handleClick(event.detail.x, event.detail.y);
});

_handleClick(x, y) {
  for (const handler of _clickHandlers) {
    if (_isInBounds(x, y, handler.bounds)) {
      handler.callback(x, y);  // Trigger callback!
      return;
    }
  }
}
```

#### 6. Callback Execution
```javascript
// User's callback is called
callback(3200, 1600) {
  console.log('Weather button clicked!');
  AnyWP.openURL('https://weather.com');
}
```

#### 7. Native Opens URL
```cpp
HandleWebMessage({"type":"openURL","url":"https://weather.com"})
  ↓
ShellExecuteW(L"open", L"https://weather.com", ...);
  ↓
Default browser opens the URL
```

---

## 🔧 实现细节

### C++ 端

#### 鼠标钩子安装
```cpp
void SetupMouseHook() {
  mouse_hook_ = SetWindowsHookExW(
    WH_MOUSE_LL,              // Low-level mouse hook
    LowLevelMouseProc,        // Callback function
    GetModuleHandleW(nullptr),
    0                         // All threads
  );
}
```

#### 事件分发
```cpp
void SendClickToWebView(int x, int y, const char* event_type) {
  std::wstringstream script;
  script << L"(function() {"
         << L"  var event = new CustomEvent('AnyWallpaper:mouse', {"
         << L"    detail: {"
         << L"      type: '" << event_type << L"',"
         << L"      x: " << x << L","
         << L"      y: " << y << L","
         << L"      button: 0"
         << L"    }"
         << L"  });"
         << L"  window.dispatchEvent(event);"
         << L"})();";
  
  webview_->ExecuteScript(script.str().c_str(), nullptr);
}
```

#### 消息桥接
```cpp
void SetupMessageBridge() {
  webview_->add_WebMessageReceived(
    Callback<ICoreWebView2WebMessageReceivedEventHandler>(
      [this](sender, args) -> HRESULT {
        LPWSTR message;
        args->get_WebMessageAsJson(&message);
        HandleWebMessage(ConvertToString(message));
        CoTaskMemFree(message);
        return S_OK;
      }).Get(), nullptr);
}
```

---

### JavaScript Side (AnyWP SDK v4.1.0)

#### Event Listeners
```javascript
_setupEventListeners() {
  // Interaction mode
  window.addEventListener('AnyWP:interactionMode', (event) => {
    this.interactionEnabled = event.detail.enabled;
  });
  
  // Mouse events
  window.addEventListener('AnyWP:mouse', (event) => {
    if (!this.interactionEnabled) return;  // Check interaction mode
    
    const {type, x, y, button} = event.detail;
    
    // Call user's onMouse callbacks
    this._mouseCallbacks.forEach(cb => cb(event.detail));
  });
  
  // Click events (optimized)
  window.addEventListener('AnyWP:click', (event) => {
    this._handleClick(event.detail.x, event.detail.y);
  });
  
  // Visibility events (v4.1.0)
  window.addEventListener('AnyWP:visibility', (event) => {
    const visible = event.detail.visible;
    if (!visible) {
      this._autoPauseAnimations();
    } else {
      this._autoResumeAnimations();
    }
    if (this._visibilityCallback) {
      this._visibilityCallback(visible);
    }
  });
}
```

#### Click Area Matching
```javascript
onClick(element, callback, options) {
  options = options || {};
  const waitFor = options.waitFor !== undefined ? options.waitFor : !options.immediate;
  
  function registerElement(el) {
    const bounds = this._calculateElementBounds(el);
    this._clickHandlers.push({
      element: el,
      callback: callback,
      bounds: bounds,
      selector: typeof element === 'string' ? element : null,
      autoRefresh: options.autoRefresh !== undefined ? options.autoRefresh : true
    });
    
    // Setup ResizeObserver for auto-refresh
    if (window.ResizeObserver && options.autoRefresh) {
      const resizeObserver = new ResizeObserver(() => {
        this._refreshElementBounds(handlerData);
      });
      resizeObserver.observe(el);
    }
  }
  
  if (waitFor && typeof element === 'string') {
    this._waitForElement(element, registerElement, options.maxWait || 10000);
  } else {
    // Register with delay or immediately
    const delay = options.immediate ? 0 : (options.delay || 100);
    setTimeout(() => registerElement(element), delay);
  }
}

_handleClick(x, y) {
  for (const handler of this._clickHandlers) {
    if (this._isInBounds(x, y, handler.bounds)) {
      handler.callback(x, y);  // Hit!
      return;
    }
  }
}
```

---

## 📊 Pixel Coordinate System

### Physical Pixels vs CSS Pixels

```javascript
// DPI Scaling = 2x (200%)
AnyWP.dpiScale = 2;

// User clicks desktop
Physical: (4000, 2000)

// SDK internal display
CSS: (4000 / 2, 2000 / 2) = (2000, 1000)

// Element bounds
DOM Rect: {left: 100, top: 50, width: 200, height: 100}  // CSS pixels
Physical: {left: 200, top: 100, width: 400, height: 200}  // Physical pixels

// Click detection
if (4000 >= 200 && 4000 <= 600 &&    // X: 200~600
    2000 >= 100 && 2000 <= 300) {    // Y: 100~300
  callback();  // Hit!
}
```

---

## 🎮 支持的功能

### ✅ 已实现

1. **onClick** - 点击区域注册
   - 物理像素边界计算
   - 点击检测和回调

2. **openURL** - 打开链接
   - 系统默认浏览器
   - 支持 https/http/ms-settings/file

3. **ready** - 就绪通知
   - 通知客户端加载完成

4. **onMouse** - 鼠标事件
   - mousedown / mouseup 事件
   - 坐标透传

5. **onKeyboard** - 键盘事件
   - 预留接口（需要键盘钩子）

6. **属性支持**
   - version / dpiScale / screenWidth / screenHeight
   - interactionEnabled

---

## 🔍 Debugging Support

### Debug Mode
```html
<!-- Enable via URL parameter -->
<script src="...?debug"></script>

<!-- Or manually enable -->
<script>
AnyWP.enableDebug();
</script>
```

### Debug Output
```
========================================
AnyWP Engine v4.1.0 [DEBUG MODE]
========================================
Screen: 5120x2880
DPI Scale: 2x
========================================

----------------------------------------
Click Handler Registered:
  Element: btn-weather
  Physical: [4000,2000] ~ [4400,2200]
  Size: 400x200 px
  CSS: [2000,1000] 200x100
----------------------------------------

Click at physical: (4200,2100) CSS: (2100,1050)
  -> HIT: btn-weather
```

### Debug 边框
- 红色实线边框
- 红色发光效果
- 显示所有注册的点击区域

---

## 🚀 性能考虑

### 鼠标钩子性能

**当前实现**:
- 仅捕获 `mousedown` 和 `mouseup`
- **不捕获** `mousemove`（避免性能问题）

**可选启用 mousemove**:
```cpp
// 在 LowLevelMouseProc 中取消注释
} else if (wParam == WM_MOUSEMOVE) {
  event_type = "mousemove";  // 启用移动跟踪
}
```

**注意**: 启用 mousemove 会大量调用，需要在 JavaScript 端节流：
```javascript
let lastTime = 0;
AnyWallpaper.onMouse((event) => {
  if (event.type !== 'mousemove') return;
  
  const now = Date.now();
  if (now - lastTime < 16) return;  // 60fps 限制
  lastTime = now;
  
  // 处理移动事件
});
```

---

## 📝 Complete Example

### Weather Wallpaper
```html
<!DOCTYPE html>
<html>
<head>
  <title>Weather Wallpaper</title>
  <!-- SDK auto-injected by AnyWP Engine -->
</head>
<body>
  <div id="weather-card">
    <h1>25°C</h1>
    <p>Sunny</p>
  </div>

  <script>
    // Wait for SDK
    if (window.AnyWP) {
      // Register click event
      AnyWP.onClick('#weather-card', () => {
        console.log('Weather card clicked!');
        AnyWP.openURL('https://weather.com');
      });
      
      // Notify ready
      AnyWP.ready('Weather Wallpaper v1.0');
    }
  </script>
</body>
</html>
```

### Execution Flow
1. User clicks weather card area on desktop
2. Windows hook captures click
3. Coordinates sent to JavaScript
4. SDK detects hit on #weather-card bounds
5. Triggers callback
6. Calls AnyWP.openURL()
7. Message sent to C++ via chrome.webview.postMessage
8. ShellExecute opens browser

---

## 🎯 Current Implementation Status

### ✅ Fully Supported (v4.1.0)
- [x] onClick - Click area registration with SPA support
- [x] openURL - Open links in browser
- [x] ready - Ready notification
- [x] onMouse - Mouse events (down/up)
- [x] onKeyboard - Keyboard event placeholder
- [x] **onVisibilityChange** - Visibility change callback (NEW v4.1.0)
- [x] Interaction mode control
- [x] DPI scaling support
- [x] Debug mode with visual borders
- [x] SPA framework auto-detection (React/Vue/Angular)
- [x] Multi-monitor support
- [x] **Auto-pause media** (videos/audio) (NEW v4.1.0)

### ⚠️ Partial Support
- [ ] Keyboard hook (interface ready, needs implementation)
- [ ] mousemove (disabled for performance, can be enabled)

### 📋 Future Enhancements
- [ ] Drag and drop support
- [ ] Scroll event forwarding
- [ ] Multi-touch support

---

## 🧪 测试方法

### 1. 使用测试页面
```bash
# 启动应用
.\scripts\build_and_run.bat

# 点击 "Start Wallpaper"
# 访问: file:///path/to/your/project/examples/test_api.html
```

### 2. 验证功能
- 点击桌面任意位置 → 控制台输出坐标
- 点击按钮区域 → SDK 触发回调
- 点击 "打开网页" → 浏览器打开

### 3. Check Logs
```
[AnyWP] [Hook] Mouse hook installed successfully
[AnyWP] [PowerSaving] Power saving monitoring setup complete
[AnyWP] [API] SDK manually injected successfully
[AnyWP] [API] Sent interaction mode to JS: 1
[AnyWP] [Memory] Auto-optimizing memory after page load...
[AnyWP] [Memory] Auto-optimization: 200MB -> 95MB (freed 105MB)
[AnyWP] [API] Received message: {"type":"openURL","url":"..."}
```

---

## 🔒 Security Mechanisms

### URL Validation
All URLs via `openURL()` are validated:
```cpp
// Blacklist (default)
url_validator_.AddBlacklist("file:///c:/windows");
url_validator_.AddBlacklist("file:///c:/program");

// Whitelist (optional)
// url_validator_.AddWhitelist("https://*");  // Uncomment to enable
```

### Permission Control
WebView2 permissions auto-denied:
- Microphone
- Camera
- Geolocation
- Clipboard read

### Safety Features (v4.1.0)
- NULL pointer checks on all webview access
- COM pointer validation (.Get())
- Memory value range validation
- Graceful error handling (no crashes)

---

## 🆕 What's New in v4.1.0

### Power Saving & Optimization
- **Lightweight pause**: WebView stops rendering but preserves state
- **Instant resume**: <50ms recovery (20x faster than v1.0)
- **Auto memory optimization**: 200MB -> 100MB after page load
- **Visibility API**: Page Visibility API integration

### Safety Enhancements
- No dangling pointers (removed detached threads)
- Comprehensive NULL checks
- Memory overflow protection
- API call validation

### Performance
- Reduced logging overhead
- Optimized mouse hook (skip when paused)
- Lower memory threshold (150MB)
- More frequent cleanup (15min)

---

## 🏗️ C++ 模块化实现 (v1.3.2+)

### SDK 注入与桥接

#### SDKBridge 模块
**位置**: `windows/modules/sdk_bridge.cpp/h`

**职责**:
- SDK 文件注入到 WebView2
- 消息监听器设置
- 消息解析和分发
- 处理器注册系统

**关键方法**:
```cpp
// 注入 SDK JavaScript 代码
bool InjectSDK(ICoreWebView2* webview, const std::string& sdk_content);

// 设置消息桥接
void SetupMessageBridge(ICoreWebView2* webview);

// 处理来自 Web 的消息
void HandleWebMessage(const std::string& message);

// 注册消息处理器
void RegisterHandler(const std::string& type, MessageHandler handler);
```

**使用示例**:
```cpp
// 在核心插件中使用
SDKBridge sdk_bridge;

// 1. 注入 SDK
sdk_bridge.InjectSDK(webview, sdk_content);

// 2. 设置消息桥接
sdk_bridge.SetupMessageBridge(webview);

// 3. 注册处理器
sdk_bridge.RegisterHandler("openURL", [this](const json& data) {
  // 处理 openURL 消息
});
```

### iframe 检测与坐标映射

#### IframeDetector 模块
**位置**: `windows/modules/iframe_detector.cpp/h`

**职责**:
- 解析 iframe 边界信息
- 点击坐标映射
- iframe 层级管理

**关键方法**:
```cpp
// 更新 iframe 列表
void UpdateIframes(const std::string& json_data);

// 获取指定点的 iframe 信息
std::optional<IframeInfo> GetIframeAtPoint(int x, int y);

// 清空 iframe 数据
void Clear();
```

**数据流**:
```
Web Page (JavaScript)
     ↓ AnyWP.postMessage({type: "iframe_update"})
SDKBridge.HandleWebMessage()
     ↓ Parse JSON
IframeDetector.UpdateIframes()
     ↓ Store bounds
Mouse Click Event
     ↓ Check position
IframeDetector.GetIframeAtPoint()
     ↓ Return iframe info
Forward to iframe
```

### 架构优势

**模块化设计 (v1.3.2+)**:
- ✅ **单一职责**: SDKBridge 专注通信，IframeDetector 专注坐标
- ✅ **低耦合**: 模块间通过接口交互
- ✅ **易测试**: 独立模块便于单元测试
- ✅ **易扩展**: 添加新消息类型只需注册新处理器

**向后兼容**:
- ✅ 所有 API 行为保持不变
- ✅ Web 开发者无需修改代码
- ✅ JavaScript SDK 接口完全兼容

---

**版本更新 (v1.3.2)**:
- 本文档已更新以反映 C++ 模块化重构
- SDK 注入和消息桥接现由 SDKBridge 模块处理
- iframe 检测现由 IframeDetector 模块处理
- 外部 API 行为完全保持不变

**Version**: v4.2.0  
**Last Updated**: 2025-11-07  
**SDK Compatible**: AnyWP Engine SDK v4.2.0

**Related Documentation**:
- [Web Developer Guide (English)](WEB_DEVELOPER_GUIDE.md) - Usage guide for web developers
- [Web Developer Guide (中文)](WEB_DEVELOPER_GUIDE_CN.md) - Complete SDK guide with examples
- [Developer API Reference](DEVELOPER_API_REFERENCE.md) - Flutter/Dart API documentation
- [Technical Notes](TECHNICAL_NOTES.md) ⭐ 已更新 v1.3.2 模块化架构

