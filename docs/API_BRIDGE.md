# AnyWP API Bridge 实现文档

## 🌉 JavaScript Bridge 架构

AnyWP 提供了完整的 JavaScript Bridge，让 Web 壁纸可以与原生客户端通信。

### 核心机制

```
Desktop Click
     ↓
Windows Mouse Hook (WH_MOUSE_LL)
     ↓
C++ Plugin (LowLevelMouseProc)
     ↓
Dispatch AnyWallpaper:mouse Event
     ↓
WebView2 JavaScript
     ↓
AnyWallpaper SDK
     ↓
onClick Handler Match
     ↓
Callback Triggered
```

---

## 📡 支持的 API

### 从 Web 到 Native

#### 1. AnyWallpaper.openURL(url)
**Web 调用**:
```javascript
AnyWallpaper.openURL('https://www.bing.com');
```

**Native 处理**:
```cpp
HandleWebMessage() {
  // Parse: {"type":"OPEN_URL","url":"..."}
  ShellExecuteW(nullptr, L"open", url, ...);
}
```

**效果**: 在系统默认浏览器打开 URL

---

#### 2. AnyWallpaper.ready(name)
**Web 调用**:
```javascript
AnyWallpaper.ready('Weather Wallpaper v1.0');
```

**Native 处理**:
```cpp
HandleWebMessage() {
  // Parse: {"type":"READY","name":"..."}
  std::cout << "[AnyWallpaper] Wallpaper ready: " << name;
}
```

**效果**: 通知客户端壁纸加载完成

---

### 从 Native 到 Web

#### 3. 鼠标事件 (AnyWallpaper:mouse)
**Native 捕获**:
```cpp
// Windows Mouse Hook
WH_MOUSE_LL -> LowLevelMouseProc()
  -> WM_LBUTTONDOWN / WM_LBUTTONUP
  -> SendClickToWebView(x, y, "mouseup")
```

**发送到 Web**:
```javascript
window.dispatchEvent(new CustomEvent('AnyWallpaper:mouse', {
  detail: {
    type: 'mouseup',  // or 'mousedown'
    x: 3200,          // Physical pixels
    y: 1600,
    button: 0         // 0=left, 1=middle, 2=right
  }
}));
```

**SDK 处理**:
```javascript
AnyWallpaper._setupEventListeners() {
  window.addEventListener('AnyWallpaper:mouse', (event) => {
    if (event.detail.type === 'mouseup' && event.detail.button === 0) {
      this._handleClick(event.detail.x, event.detail.y);
    }
  });
}
```

**效果**: SDK 检查点击是否在注册元素内，触发回调

---

#### 4. 交互模式 (AnyWallpaper:interactionMode)
**Native 发送**:
```cpp
// Navigation completed callback
webview_->add_NavigationCompleted([](args) {
  ExecuteScript("
    window.dispatchEvent(new CustomEvent('AnyWallpaper:interactionMode', {
      detail: { enabled: true }
    }));
  ");
});
```

**SDK 接收**:
```javascript
window.addEventListener('AnyWallpaper:interactionMode', (event) => {
  AnyWallpaper.interactionEnabled = event.detail.enabled;
  console.log('Interaction mode:', event.detail.enabled ? 'ON' : 'OFF');
});
```

**效果**: 控制是否处理鼠标/键盘事件

---

## 🎯 完整交互流程

### 用户点击按钮示例

#### 1. 网页注册点击区域
```javascript
AnyWallpaper.onClick('#btn-weather', (x, y) => {
  console.log('Weather button clicked!');
  AnyWallpaper.openURL('https://weather.com');
});
```

#### 2. SDK 计算物理像素边界
```javascript
_calculateElementBounds(element) {
  const rect = element.getBoundingClientRect();
  return {
    left: rect.left * dpiScale,    // CSS -> Physical
    top: rect.top * dpiScale,
    right: rect.right * dpiScale,
    bottom: rect.bottom * dpiScale
  };
}
// 存储到 _clickHandlers[]
```

#### 3. 用户点击桌面
```
User clicks desktop at (3200, 1600)
     ↓
Windows Hook captures WM_LBUTTONUP
     ↓
C++: LowLevelMouseProc(WM_LBUTTONUP, {pt: {x:3200, y:1600}})
```

#### 4. Native 透传到 Web
```cpp
SendClickToWebView(3200, 1600, "mouseup");
  ↓
ExecuteScript("
  window.dispatchEvent(new CustomEvent('AnyWallpaper:mouse', {
    detail: {type: 'mouseup', x: 3200, y: 1600, button: 0}
  }));
");
```

#### 5. SDK 处理点击
```javascript
// SDK 事件监听器
window.addEventListener('AnyWallpaper:mouse', (event) => {
  if (event.detail.type === 'mouseup') {
    _handleClick(3200, 1600);
  }
});

_handleClick(x, y) {
  for (const handler of _clickHandlers) {
    if (_isInBounds(x, y, handler.bounds)) {
      handler.callback(x, y);  // 触发回调！
      return;
    }
  }
}
```

#### 6. 回调执行
```javascript
// 用户的回调被调用
callback(3200, 1600) {
  console.log('Weather button clicked!');
  AnyWallpaper.openURL('https://weather.com');
}
```

#### 7. Native 打开 URL
```cpp
HandleWebMessage({"type":"OPEN_URL","url":"https://weather.com"})
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

### JavaScript 端 (AnyWallpaper SDK)

#### 事件监听器
```javascript
_setupEventListeners() {
  // 交互模式
  window.addEventListener('AnyWallpaper:interactionMode', (event) => {
    this.interactionEnabled = event.detail.enabled;
  });
  
  // 鼠标事件
  window.addEventListener('AnyWallpaper:mouse', (event) => {
    if (!this.interactionEnabled) return;  // 检查交互模式
    
    const {type, x, y, button} = event.detail;
    
    // 调用用户的 onMouse 回调
    this._mouseHandlers.forEach(handler => handler(event.detail));
    
    // 处理点击（仅 mouseup + 左键）
    if (type === 'mouseup' && button === 0) {
      this._handleClick(x, y);
    }
  });
}
```

#### 点击区域匹配
```javascript
onClick(element, callback, options) {
  // 延迟 2 秒确保 DOM 渲染完成
  setTimeout(() => {
    const bounds = this._calculateElementBounds(element);
    this._clickHandlers.push({bounds, callback, element});
  }, 2000);
}

_handleClick(x, y) {
  for (const handler of this._clickHandlers) {
    if (this._isInBounds(x, y, handler.bounds)) {
      handler.callback(x, y);  // 命中！
      return;
    }
  }
}
```

---

## 📊 像素坐标系统

### 物理像素 vs CSS 像素

```javascript
// DPI 缩放 = 2x (200%)
AnyWallpaper.dpiScale = 2;

// 用户点击桌面
Physical: (4000, 2000)

// SDK 转换显示
CSS: (4000 / 2, 2000 / 2) = (2000, 1000)

// 元素边界
DOM Rect: {left: 100, top: 50, width: 200, height: 100}  // CSS pixels
Physical: {left: 200, top: 100, width: 400, height: 200}  // Physical pixels

// 点击检测
if (4000 >= 200 && 4000 <= 600 &&    // X: 200~600
    2000 >= 100 && 2000 <= 300) {    // Y: 100~300
  callback();  // 命中！
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

## 🔍 调试支持

### Debug 模式
```html
<!-- URL 参数启用 -->
<script src="...?debug"></script>

<!-- 或手动启用 -->
<script>
AnyWallpaper.enableDebug();
</script>
```

### Debug 输出
```
========================================
AnyWallpaper Engine v3.1.0 [DEBUG MODE]
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

## 📝 完整示例

### 天气壁纸
```html
<!DOCTYPE html>
<html>
<head>
  <script src="/path/to/anywallpaper_sdk.js"></script>
</head>
<body>
  <div id="weather-card">
    <h1>25°C</h1>
    <p>Sunny</p>
  </div>

  <script>
    // 注册点击事件
    AnyWallpaper.onClick('#weather-card', () => {
      AnyWallpaper.openURL('https://weather.com');
    });
    
    // 通知就绪
    AnyWallpaper.ready('Weather Wallpaper v1.0');
  </script>
</body>
</html>
```

### 运行效果
1. 用户点击天气卡片区域
2. Windows 钩子捕获点击
3. 坐标发送到 JavaScript
4. SDK 检测命中 #weather-card
5. 触发回调
6. 调用 AnyWallpaper.openURL()
7. 消息发送到 C++
8. ShellExecute 打开浏览器

---

## 🎯 当前实现状态

### ✅ 完全支持
- [x] onClick 点击区域注册
- [x] openURL 打开链接
- [x] ready 就绪通知
- [x] onMouse 鼠标事件（down/up）
- [x] 交互模式控制
- [x] DPI 缩放支持
- [x] Debug 模式

### ⚠️ 部分支持
- [ ] onKeyboard（需要键盘钩子）
- [ ] mousemove（已禁用，可选启用）

### 📋 未来增强
- [ ] onResize - 窗口大小变化
- [ ] onFocus - 窗口焦点
- [ ] 多显示器坐标转换

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

### 3. 查看日志
```
[AnyWallpaper] [Hook] Mouse hook installed successfully
[AnyWallpaper] [API] Sent interaction mode to JS: 1
[AnyWallpaper] [Hook] Mouse click at: 3200,1600
[AnyWallpaper] [API] Received message: {"type":"OPEN_URL","url":"..."}
```

---

## 🔒 安全机制

### URL 验证
所有通过 `openURL()` 的 URL 都经过验证：
```cpp
// 黑名单
url_validator_.AddBlacklist("file:///c:/windows");
url_validator_.AddBlacklist("file:///c:/program");

// 白名单（可选）
url_validator_.AddWhitelist("https://*");
```

### 权限控制
WebView2 权限自动拒绝：
- 麦克风
- 摄像头
- 地理位置
- 剪贴板读取

---

**版本**: v1.2.0  
**最后更新**: 2025-10-31  
**SDK 兼容**: AnyWallpaper Engine SDK v3.1.0

