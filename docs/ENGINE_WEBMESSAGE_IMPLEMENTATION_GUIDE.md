# AnyWP Engine - WebMessage 双向通讯实施指南

> **面向对象**: AnyWP Engine C++ 开发者  
> **目标**: 实现 Flutter ↔ C++ ↔ JavaScript 三层双向通讯  
> **预计工时**: 2-3 天  
> **版本**: v2.1.0+

---

## 📋 目录

1. [当前架构分析](#当前架构分析)
2. [需要实现的功能](#需要实现的功能)
3. [详细实施步骤](#详细实施步骤)
4. [测试验证](#测试验证)
5. [性能优化建议](#性能优化建议)

---

## 🏗️ 当前架构分析

### 现有模块

```cpp
// packages/anywp_engine_v2.0.0/windows/

├── modules/
│   ├── flutter_bridge.h/cpp      // ✅ Flutter MethodChannel 通讯
│   ├── sdk_bridge.h/cpp          // ✅ JavaScript WebMessage 通讯
│   ├── instance_manager.h/cpp    // ✅ 多显示器实例管理
│   └── ...
└── src/
    └── anywp_engine_plugin.h/cpp // ✅ 主插件类
```

### 现有通讯流程

```
┌──────────────┐ MethodChannel  ┌───────────────┐
│ Flutter Dart │ ←──────────→   │ FlutterBridge │
└──────────────┘                │  (C++)        │
                                └───────────────┘
                                        ↓
                                ┌───────────────┐
                                │ AnyWPEngine   │
                                │  Plugin       │
                                └───────────────┘
                                        ↓
                                ┌───────────────┐
                                │  SDKBridge    │
                                │  (C++)        │
                                └───────────────┘
                                        ↓
┌──────────────┐ WebMessage     ┌───────────────┐
│ JavaScript   │ ←──────────→   │ WebView2      │
│ (HTML)       │                │ (chrome.webview)
└──────────────┘                └───────────────┘
```

### 当前问题

❌ **单向通讯**:  
- Flutter → C++ ✅ (MethodChannel)
- C++ → JavaScript ✅ (WebMessage)
- **JavaScript → C++ → Flutter** ❌ (未实现返回路径)

❌ **缺乏消息路由**:  
- 无法将 JavaScript 的消息转发回 Flutter
- 无法实现状态同步

---

## 🎯 需要实现的功能

### 功能 1: 扩展 FlutterBridge - 添加 `sendMessage` 方法

**作用**: 允许 Flutter 发送消息到 JavaScript

```cpp
// flutter_bridge.h
class FlutterBridge {
public:
  // 新增方法
  void HandleSendMessage(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};
```

### 功能 2: 扩展 SDKBridge - 添加消息转发能力

**作用**: 将 JavaScript 消息转发回 Flutter

```cpp
// sdk_bridge.h
class SDKBridge {
public:
  // 新增方法
  void SetFlutterCallback(std::function<void(const std::string&)> callback);
  void ForwardMessageToFlutter(const std::string& message);
  
private:
  std::function<void(const std::string&)> flutter_callback_;
};
```

### 功能 3: 实现消息分发机制

**作用**: 根据消息类型路由到不同的处理器

```cpp
// 消息类型枚举
enum class MessageType {
  UPDATE_CAROUSEL,      // 更新轮播池
  ADD_TO_CAROUSEL,      // 添加壁纸
  CONTROL_PLAYBACK,     // 控制播放
  STATE_SYNC,           // 状态同步
  HEARTBEAT,            // 心跳
  CUSTOM                // 自定义消息
};
```

---

## 🔨 详细实施步骤

### 第 1 步: 扩展 FlutterBridge (flutter_bridge.h/cpp)

#### 1.1 修改 `flutter_bridge.h`

```cpp
// File: packages/anywp_engine_v2.0.0/windows/modules/flutter_bridge.h

class FlutterBridge {
public:
  // ... 现有方法 ...

  // ========================================
  // 新增: 消息通讯方法
  // ========================================
  
  void HandleSendMessage(
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

private:
  // ... 现有成员 ...
};
```

#### 1.2 实现 `flutter_bridge.cpp`

```cpp
// File: packages/anywp_engine_v2.0.0/windows/modules/flutter_bridge.cpp

void FlutterBridge::RegisterAllHandlers() {
  // ... 现有注册 ...
  
  // 新增: 注册消息发送方法
  RegisterHandler("sendMessage", [this](
      const flutter::EncodableMap* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    this->HandleSendMessage(args, std::move(result));
  });
}

void FlutterBridge::HandleSendMessage(
    const flutter::EncodableMap* args,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  // 1. 验证参数
  std::string message_json;
  if (!GetStringArgument(args, "message", message_json, result)) {
    return; // Error already sent
  }

  // 2. 获取目标显示器索引（可选）
  int monitor_index = -1; // -1 表示所有显示器
  auto monitor_it = args->find(flutter::EncodableValue("monitorIndex"));
  if (monitor_it != args->end() && !monitor_it->second.IsNull()) {
    monitor_index = std::get<int>(monitor_it->second);
  }

  OutputDebugStringA("[FlutterBridge] sendMessage called\n");
  OutputDebugStringA(("  Message: " + message_json + "\n").c_str());
  OutputDebugStringA(("  Monitor: " + std::to_string(monitor_index) + "\n").c_str());

  // 3. 获取壁纸实例
  std::vector<WallpaperInstance*> target_instances;
  
  if (monitor_index >= 0) {
    // 发送到指定显示器
    auto* instance = plugin_->GetInstanceForMonitor(monitor_index);
    if (instance) {
      target_instances.push_back(instance);
    } else {
      result->Error("INSTANCE_NOT_FOUND", 
                   "No wallpaper instance for monitor " + std::to_string(monitor_index));
      return;
    }
  } else {
    // 发送到所有显示器
    std::lock_guard<std::mutex> lock(plugin_->instances_mutex_);
    for (auto& instance : plugin_->wallpaper_instances_) {
      target_instances.push_back(&instance);
    }
  }

  if (target_instances.empty()) {
    result->Error("NO_INSTANCES", "No active wallpaper instances");
    return;
  }

  // 4. 发送消息到 JavaScript
  bool all_success = true;
  int sent_count = 0;

  for (auto* instance : target_instances) {
    if (!instance || !instance->webview) {
      all_success = false;
      continue;
    }

    // 构建 JavaScript 代码：触发 WebMessage 事件
    std::wstring script = L"(function() {\n"
                          L"  try {\n"
                          L"    const message = " + std::wstring(message_json.begin(), message_json.end()) + L";\n"
                          L"    const event = new CustomEvent('AnyWP:message', {\n"
                          L"      detail: message,\n"
                          L"      bubbles: true\n"
                          L"    });\n"
                          L"    window.dispatchEvent(event);\n"
                          L"    console.log('[AnyWP Engine] Message dispatched:', message);\n"
                          L"  } catch(e) {\n"
                          L"    console.error('[AnyWP Engine] Failed to dispatch message:', e);\n"
                          L"  }\n"
                          L"})();\n";

    // 执行脚本
    HRESULT hr = instance->webview->ExecuteScript(
        script.c_str(),
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [](HRESULT error_code, LPCWSTR result_object_as_json) -> HRESULT {
              if (FAILED(error_code)) {
                OutputDebugStringA("[FlutterBridge] ExecuteScript failed\n");
              }
              return S_OK;
            }
        ).Get()
    );

    if (SUCCEEDED(hr)) {
      sent_count++;
    } else {
      all_success = false;
      OutputDebugStringA("[FlutterBridge] Failed to send message to instance\n");
    }
  }

  // 5. 返回结果
  if (all_success && sent_count > 0) {
    result->Success(flutter::EncodableValue(true));
    OutputDebugStringA(("[FlutterBridge] Message sent successfully to " + 
                       std::to_string(sent_count) + " instance(s)\n").c_str());
  } else {
    result->Error("SEND_FAILED", 
                 "Failed to send message to some instances (" + 
                 std::to_string(sent_count) + "/" + 
                 std::to_string(target_instances.size()) + " succeeded)");
  }
}
```

---

### 第 2 步: 扩展 SDKBridge (sdk_bridge.h/cpp)

#### 2.1 修改 `sdk_bridge.h`

```cpp
// File: packages/anywp_engine_v2.0.0/windows/modules/sdk_bridge.h

class SDKBridge {
public:
  // ... 现有方法 ...

  // ========================================
  // 新增: Flutter 消息转发
  // ========================================
  
  // 设置 Flutter 回调（用于转发消息）
  void SetFlutterCallback(std::function<void(const std::string&)> callback);
  
  // 转发消息到 Flutter
  void ForwardMessageToFlutter(const std::string& message);

private:
  // ... 现有成员 ...
  
  // Flutter 回调函数
  std::function<void(const std::string&)> flutter_callback_;
};
```

#### 2.2 实现 `sdk_bridge.cpp`

```cpp
// File: packages/anywp_engine_v2.0.0/windows/modules/sdk_bridge.cpp

void SDKBridge::SetFlutterCallback(std::function<void(const std::string&)> callback) {
  flutter_callback_ = callback;
  OutputDebugStringA("[SDKBridge] Flutter callback registered\n");
}

void SDKBridge::ForwardMessageToFlutter(const std::string& message) {
  if (!flutter_callback_) {
    OutputDebugStringA("[SDKBridge] WARNING: Flutter callback not set\n");
    return;
  }

  OutputDebugStringA("[SDKBridge] Forwarding message to Flutter\n");
  OutputDebugStringA(("  Message: " + message + "\n").c_str());

  // 调用 Flutter 回调
  flutter_callback_(message);
}

void SDKBridge::SetupMessageBridge() {
  if (!webview_) {
    OutputDebugStringA("[SDKBridge] ERROR: WebView not set\n");
    return;
  }

  // 添加消息接收处理器
  EventRegistrationToken token;
  webview_->add_WebMessageReceived(
      Callback<ICoreWebView2WebMessageReceivedEventHandler>(
          [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
            LPWSTR message_raw;
            args->get_WebMessageAsJson(&message_raw);

            // 转换为 UTF-8
            std::wstring message_wide(message_raw);
            std::string message(message_wide.begin(), message_wide.end());
            CoTaskMemFree(message_raw);

            OutputDebugStringA("[SDKBridge] WebMessage received\n");
            OutputDebugStringA(("  Message: " + message + "\n").c_str());

            // 解析消息类型
            std::string msg_type = GetMessageType(message);
            OutputDebugStringA(("  Type: " + msg_type + "\n").c_str());

            // 检查是否需要转发到 Flutter
            if (msg_type == "carouselStateChanged" || 
                msg_type == "wallpaperReady" ||
                msg_type == "error" ||
                msg_type.find("sync") != std::string::npos) {
              
              OutputDebugStringA("[SDKBridge] Forwarding to Flutter\n");
              ForwardMessageToFlutter(message);
            }

            // 调用已注册的处理器
            HandleMessage(message);

            return S_OK;
          }
      ).Get(),
      &token
  );

  OutputDebugStringA("[SDKBridge] Message bridge setup complete\n");
}
```

---

### 第 3 步: 修改 AnyWPEnginePlugin (anywp_engine_plugin.cpp)

#### 3.1 初始化时连接 SDKBridge 和 FlutterBridge

```cpp
// File: packages/anywp_engine_v2.0.0/windows/src/anywp_engine_plugin.cpp

AnyWPEnginePlugin::AnyWPEnginePlugin() {
  // ... 现有初始化 ...

  // 创建模块
  flutter_bridge_ = std::make_unique<FlutterBridge>(this);
  sdk_bridge_ = std::make_unique<SDKBridge>();

  // 新增: 连接 SDKBridge 到 Flutter
  sdk_bridge_->SetFlutterCallback([this](const std::string& message) {
    // 在主线程中调用 Flutter 方法
    this->NotifyFlutterMessage(message);
  });

  OutputDebugStringA("[AnyWPEngine] Plugin initialized with message forwarding\n");
}

// 新增方法: 通知 Flutter 接收到消息
void AnyWPEnginePlugin::NotifyFlutterMessage(const std::string& message) {
  if (!method_channel_) {
    OutputDebugStringA("[AnyWPEngine] ERROR: Method channel not available\n");
    return;
  }

  OutputDebugStringA("[AnyWPEngine] Notifying Flutter of message\n");
  OutputDebugStringA(("  Message: " + message + "\n").c_str());

  // 构建参数
  flutter::EncodableMap args;
  args[flutter::EncodableValue("message")] = flutter::EncodableValue(message);

  // 调用 Dart 方法
  method_channel_->InvokeMethod(
      "onMessage",
      std::make_unique<flutter::EncodableValue>(args)
  );

  OutputDebugStringA("[AnyWPEngine] Flutter notification sent\n");
}
```

#### 3.2 在头文件中添加声明

```cpp
// File: packages/anywp_engine_v2.0.0/windows/src/anywp_engine_plugin.h

class AnyWPEnginePlugin : public flutter::Plugin {
  // ... 现有代码 ...

private:
  // 新增: Flutter 消息通知
  void NotifyFlutterMessage(const std::string& message);

  // ... 现有成员 ...
};
```

---

### 第 4 步: 更新 JavaScript SDK (anywp_sdk.js)

#### 4.1 添加消息接收监听

```javascript
// File: packages/anywp_engine_v2.0.0/windows/anywp_sdk.js

// 在 SDK 初始化时添加消息监听
window.addEventListener('AnyWP:message', (event) => {
  const message = event.detail;
  console.log('[AnyWP SDK] Message received from Flutter:', message);

  // 根据消息类型分发
  if (message.type === 'updateCarousel') {
    handleUpdateCarousel(message.data);
  } else if (message.type === 'addToCarousel') {
    handleAddToCarousel(message.data);
  } else if (message.type === 'play') {
    carousel.play();
  } else if (message.type === 'pause') {
    carousel.pause();
  }
  // ... 更多消息类型处理 ...
});

// 发送消息到 Flutter 的辅助函数
function sendToFlutter(type, data) {
  if (!window.chrome || !window.chrome.webview) {
    console.warn('[AnyWP SDK] chrome.webview not available');
    return;
  }

  const message = {
    type: type,
    timestamp: Date.now(),
    data: data || {}
  };

  console.log('[AnyWP SDK] Sending message to Flutter:', message);
  window.chrome.webview.postMessage(message);
}

// 暴露给用户的 API
window.AnyWP.sendToFlutter = sendToFlutter;

// 示例: 轮播状态变化时通知 Flutter
carousel.on('indexChanged', (index) => {
  sendToFlutter('carouselStateChanged', {
    currentIndex: index,
    totalImages: carousel.getTotalImages()
  });
});
```

---

## 🧪 测试验证

### 测试 1: Flutter → JavaScript 消息发送

#### Dart 测试代码

```dart
// test/message_test.dart
void main() async {
  // 初始化引擎
  await AnyWPEngine.setApplicationName('HKCW_Test');
  
  // 发送消息到 JavaScript
  final message = {
    'type': 'updateCarousel',
    'timestamp': DateTime.now().millisecondsSinceEpoch,
    'data': {
      'images': [
        'https://example.com/img1.jpg',
        'https://example.com/img2.jpg',
      ],
      'interval': 30000,
    },
  };

  final success = await AnyWPEngine.sendMessage(message: message);
  
  if (success) {
    print('✅ Message sent successfully');
  } else {
    print('❌ Failed to send message');
  }
}
```

#### JavaScript 测试代码

```html
<!-- test/message_receiver_test.html -->
<!DOCTYPE html>
<html>
<head>
  <title>Message Receiver Test</title>
  <script src="../windows/anywp_sdk.js"></script>
</head>
<body>
  <h1>AnyWP Message Receiver Test</h1>
  <div id="log"></div>

  <script>
    const logDiv = document.getElementById('log');

    // 监听来自 Flutter 的消息
    window.addEventListener('AnyWP:message', (event) => {
      const message = event.detail;
      const timestamp = new Date().toISOString();
      
      logDiv.innerHTML += `
        <div style="border: 1px solid #ccc; padding: 10px; margin: 5px 0;">
          <strong>[${timestamp}] Message Received:</strong><br>
          <pre>${JSON.stringify(message, null, 2)}</pre>
        </div>
      `;

      console.log('✅ Message received:', message);
    });

    console.log('✅ Message receiver initialized');
  </script>
</body>
</html>
```

### 测试 2: JavaScript → Flutter 消息发送

#### JavaScript 测试代码

```javascript
// 在浏览器控制台执行
AnyWP.sendToFlutter('carouselStateChanged', {
  currentIndex: 2,
  totalImages: 10
});
```

#### Dart 测试代码

```dart
// 设置消息接收回调
AnyWPEngine.setOnMessageCallback((message) {
  print('✅ Received message from JavaScript:');
  print('  Type: ${message['type']}');
  print('  Data: ${message['data']}');
  print('  Timestamp: ${message['timestamp']}');
});
```

### 测试 3: 完整双向通讯测试

```dart
// test/bidirectional_test.dart
void testBidirectionalCommunication() async {
  int messageCount = 0;

  // 1. 设置接收回调
  AnyWPEngine.setOnMessageCallback((message) {
    messageCount++;
    print('[$messageCount] Received from JS: ${message['type']}');
  });

  // 2. 发送消息到 JavaScript
  await AnyWPEngine.sendMessage(message: {
    'type': 'ping',
    'data': {'requestId': 'test-001'}
  });

  // 3. JavaScript 应该回复 'pong'
  await Future.delayed(Duration(seconds: 2));

  if (messageCount > 0) {
    print('✅ Bidirectional communication working!');
  } else {
    print('❌ No response from JavaScript');
  }
}
```

---

## ⚡ 性能优化建议

### 1. 消息批处理

```cpp
// 避免频繁发送，合并多个消息
class MessageBatcher {
public:
  void QueueMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    message_queue_.push_back(message);
    
    if (message_queue_.size() >= BATCH_SIZE) {
      Flush();
    }
  }

  void Flush() {
    if (message_queue_.empty()) return;
    
    // 批量发送
    std::string batch = "[" + Join(message_queue_, ",") + "]";
    SendToJavaScript(batch);
    message_queue_.clear();
  }

private:
  static constexpr size_t BATCH_SIZE = 10;
  std::vector<std::string> message_queue_;
  std::mutex queue_mutex_;
};
```

### 2. 消息压缩

```cpp
// 对大消息进行压缩
std::string CompressMessage(const std::string& message) {
  if (message.size() < 1024) {
    return message; // 小消息不压缩
  }

  // 使用 zlib 压缩
  // ... 压缩实现 ...
  
  return compressed_message;
}
```

### 3. 异步处理

```cpp
// 使用线程池处理消息
class AsyncMessageHandler {
public:
  void ProcessMessage(const std::string& message) {
    thread_pool_.enqueue([this, message]() {
      // 后台处理
      this->HandleMessageAsync(message);
    });
  }

private:
  void HandleMessageAsync(const std::string& message) {
    // 耗时操作
    ParseMessage(message);
    UpdateState(message);
  }

  ThreadPool thread_pool_;
};
```

---

## 📊 实施检查清单

### 阶段 1: 基础实现 (1 天)

- [ ] 修改 `flutter_bridge.h` - 添加 `HandleSendMessage` 声明
- [ ] 实现 `flutter_bridge.cpp` - 实现 `HandleSendMessage` 方法
- [ ] 注册新方法到 MethodChannel
- [ ] 编译测试

### 阶段 2: 消息转发 (1 天)

- [ ] 修改 `sdk_bridge.h` - 添加 Flutter 回调接口
- [ ] 实现 `sdk_bridge.cpp` - 实现消息转发逻辑
- [ ] 修改 `anywp_engine_plugin.cpp` - 连接 SDKBridge 和 FlutterBridge
- [ ] 添加 `NotifyFlutterMessage` 方法
- [ ] 编译测试

### 阶段 3: JavaScript 集成 (0.5 天)

- [ ] 更新 `anywp_sdk.js` - 添加消息监听
- [ ] 添加 `sendToFlutter` 辅助函数
- [ ] 测试消息收发

### 阶段 4: 测试验证 (0.5 天)

- [ ] Flutter → JavaScript 单向测试
- [ ] JavaScript → Flutter 单向测试
- [ ] 双向通讯测试
- [ ] 多显示器测试
- [ ] 性能测试

---

## 📝 注意事项

### 1. 线程安全

```cpp
// 所有跨线程调用都需要加锁
std::lock_guard<std::mutex> lock(instances_mutex_);
```

### 2. 内存管理

```cpp
// 使用智能指针避免内存泄漏
std::unique_ptr<Message> msg = std::make_unique<Message>(data);
```

### 3. 错误处理

```cpp
// 所有外部调用都需要 try-catch
try {
  SendMessage(message);
} catch (const std::exception& e) {
  OutputDebugStringA(("Error: " + std::string(e.what()) + "\n").c_str());
  result->Error("EXCEPTION", e.what());
}
```

### 4. 调试日志

```cpp
// 使用 OutputDebugStringA 输出调试信息
#ifdef _DEBUG
  OutputDebugStringA("[Module] Operation completed\n");
#endif
```

---

## 🎯 总结

实施完成后，您将获得：

✅ **完整的三层双向通讯**  
✅ **类型安全的消息路由**  
✅ **高性能的消息传递**  
✅ **易于扩展的架构**

预计工时：**2-3 天**

如有问题，请参考：
- [AnyWP Engine 源码](../../packages/anywp_engine_v2.0.0/)
- [Flutter Platform Channels 文档](https://docs.flutter.dev/platform-integration/platform-channels)
- [WebView2 文档](https://learn.microsoft.com/en-us/microsoft-edge/webview2/)

---

**文档版本**: v1.0.0  
**最后更新**: 2025-11-12  
**维护者**: HKCW Desktop 开发团队

