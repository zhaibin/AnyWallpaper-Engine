# AnyWP Engine - 双向通讯快速参考

> **30 秒速览**: 引擎开发者需要修改的核心代码

---

## 🎯 核心任务

1. **扩展 FlutterBridge** - 接收 Flutter 消息，发送到 JavaScript
2. **扩展 SDKBridge** - 接收 JavaScript 消息，转发到 Flutter
3. **连接两个模块** - 在 AnyWPEnginePlugin 中建立消息路由

---

## 📝 修改清单

### 文件 1: `flutter_bridge.h`

```cpp
// 添加方法声明
void HandleSendMessage(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
```

### 文件 2: `flutter_bridge.cpp`

```cpp
// 在 RegisterAllHandlers() 中注册
RegisterHandler("sendMessage", [this](...) {
  this->HandleSendMessage(args, std::move(result));
});

// 实现方法
void FlutterBridge::HandleSendMessage(...) {
  // 1. 获取参数: message (string), monitorIndex (int, optional)
  // 2. 获取目标实例: GetInstanceForMonitor()
  // 3. 执行 JavaScript: webview->ExecuteScript()
  // 4. 返回结果: result->Success() or result->Error()
}
```

### 文件 3: `sdk_bridge.h`

```cpp
// 添加方法和成员
void SetFlutterCallback(std::function<void(const std::string&)> callback);
void ForwardMessageToFlutter(const std::string& message);

private:
  std::function<void(const std::string&)> flutter_callback_;
```

### 文件 4: `sdk_bridge.cpp`

```cpp
void SDKBridge::SetFlutterCallback(std::function<void(const std::string&)> callback) {
  flutter_callback_ = callback;
}

void SDKBridge::ForwardMessageToFlutter(const std::string& message) {
  if (flutter_callback_) {
    flutter_callback_(message);
  }
}

// 在 SetupMessageBridge() 中检查消息类型，决定是否转发
if (msg_type == "carouselStateChanged") {
  ForwardMessageToFlutter(message);
}
```

### 文件 5: `anywp_engine_plugin.h`

```cpp
private:
  void NotifyFlutterMessage(const std::string& message);
```

### 文件 6: `anywp_engine_plugin.cpp`

```cpp
// 构造函数中连接模块
AnyWPEnginePlugin::AnyWPEnginePlugin() {
  // ...
  sdk_bridge_->SetFlutterCallback([this](const std::string& msg) {
    this->NotifyFlutterMessage(msg);
  });
}

// 实现通知方法
void AnyWPEnginePlugin::NotifyFlutterMessage(const std::string& message) {
  if (!method_channel_) return;
  
  flutter::EncodableMap args;
  args[flutter::EncodableValue("message")] = flutter::EncodableValue(message);
  
  method_channel_->InvokeMethod("onMessage", 
                                std::make_unique<flutter::EncodableValue>(args));
}
```

### 文件 7: `anywp_sdk.js` (可选增强)

```javascript
// 监听来自 Flutter 的消息
window.addEventListener('AnyWP:message', (event) => {
  const message = event.detail;
  console.log('Message from Flutter:', message);
  // 根据 message.type 分发处理
});

// 发送消息到 Flutter
function sendToFlutter(type, data) {
  window.chrome.webview.postMessage({
    type: type,
    data: data,
    timestamp: Date.now()
  });
}
```

---

## 🔍 关键代码片段

### Flutter 发送消息到 JavaScript

```cpp
// flutter_bridge.cpp - HandleSendMessage()
std::wstring script = L"(function() {\n"
                      L"  const message = " + [JSON] + L";\n"
                      L"  const event = new CustomEvent('AnyWP:message', {\n"
                      L"    detail: message\n"
                      L"  });\n"
                      L"  window.dispatchEvent(event);\n"
                      L"})();\n";

instance->webview->ExecuteScript(script.c_str(), ...);
```

### JavaScript 消息转发到 Flutter

```cpp
// sdk_bridge.cpp - SetupMessageBridge()
webview_->add_WebMessageReceived(
    Callback<ICoreWebView2WebMessageReceivedEventHandler>(
        [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
          LPWSTR message_raw;
          args->get_WebMessageAsJson(&message_raw);
          
          std::string message = /* convert to UTF-8 */;
          
          // 转发到 Flutter
          ForwardMessageToFlutter(message);
          
          return S_OK;
        }
    ).Get(),
    &token
);
```

---

## ✅ 测试验证

### 1. 编译测试

```bash
cd windows
cmake --build build --config Release
```

### 2. 功能测试

```dart
// Dart 端
await AnyWPEngine.sendMessage(message: {
  'type': 'test',
  'data': {'value': 123}
});
```

```javascript
// JavaScript 端
console.log('Received:', event.detail); // 应该看到消息
```

### 3. 反向测试

```javascript
// JavaScript 端
window.chrome.webview.postMessage({
  type: 'test',
  data: {value: 456}
});
```

```dart
// Dart 端 - 设置回调
AnyWPEngine.setOnMessageCallback((message) {
  print('Received: $message'); // 应该看到消息
});
```

---

## 🐛 常见问题

### 问题 1: 消息未收到

**检查**:
1. WebView 是否已初始化？`instance->webview != nullptr`
2. 回调是否已设置？`sdk_bridge_->SetFlutterCallback(...)`
3. 消息格式是否正确？JSON 格式

### 问题 2: 编译错误

**检查**:
1. 头文件是否包含？`#include "modules/flutter_bridge.h"`
2. 链接器设置？WebView2Loader.lib
3. C++ 版本？需要 C++17 或更高

### 问题 3: 内存泄漏

**检查**:
1. 使用智能指针：`std::unique_ptr`, `std::shared_ptr`
2. ComPtr 自动管理 COM 对象
3. 回调函数不持有强引用

---

## 📊 性能指标

- **消息延迟**: < 10ms (单向)
- **吞吐量**: > 1000 消息/秒
- **内存占用**: < 1MB (消息队列)

---

## 🔗 相关文档

- [详细实施指南](./API_BRIDGE.md)
- [消息协议定义](../lib/models/wallpaper_message.dart)
- [测试用例](../test/message_test.dart)

---

**快速参考版本**: v1.0.0  
**更新日期**: 2025-11-12

