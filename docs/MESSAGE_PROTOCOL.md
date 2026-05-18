# AnyWP Engine - 消息协议规范 v1.0

> **目标**: 定义 Flutter ↔ C++ ↔ JavaScript 三层通讯的标准消息格式

---

## 📋 目录

1. [基础消息格式](#基础消息格式)
2. [消息类型定义](#消息类型定义)
3. [错误处理](#错误处理)
4. [示例](#示例)

---

## 📦 基础消息格式

### 标准消息结构

所有消息必须遵循以下 JSON 格式：

```json
{
  "id": "unique-message-id",
  "type": "message_type",
  "timestamp": 1699876543210,
  "data": {
    // 消息负载（根据类型不同）
  },
  "signature": "optional-security-signature"
}
```

### 字段说明

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `id` | `string` | ✅ | 消息唯一标识符（UUID v4 推荐） |
| `type` | `string` | ✅ | 消息类型（见下文） |
| `timestamp` | `int64` | ✅ | Unix 时间戳（毫秒） |
| `data` | `object` | ✅ | 消息数据负载 |
| `signature` | `string` | ❌ | 消息签名（可选，用于安全验证） |

---

## 📨 消息类型定义

### 1. 轮播控制消息

#### 1.1 更新轮播池 (`updateCarousel`)

**方向**: Flutter → JavaScript

**用途**: 完整更新轮播壁纸列表

```json
{
  "id": "msg-001",
  "type": "updateCarousel",
  "timestamp": 1699876543210,
  "data": {
    "images": [
      "https://example.com/image1.jpg",
      "https://example.com/image2.jpg"
    ],
    "interval": 30000,
    "transition": "fade",
    "autoPlay": true
  }
}
```

**字段说明**:
- `images`: 壁纸 URL 数组
- `interval`: 轮播间隔（毫秒）
- `transition`: 过渡效果（fade, slide, zoom）
- `autoPlay`: 是否自动播放

---

#### 1.2 添加到轮播池 (`addToCarousel`)

**方向**: Flutter → JavaScript

**用途**: 增量添加单张壁纸到轮播池

```json
{
  "id": "msg-002",
  "type": "addToCarousel",
  "timestamp": 1699876543210,
  "data": {
    "imageUrl": "https://example.com/new-image.jpg",
    "insertAt": -1
  }
}
```

**字段说明**:
- `imageUrl`: 壁纸 URL
- `insertAt`: 插入位置（-1 表示末尾）

---

#### 1.3 从轮播池移除 (`removeFromCarousel`)

**方向**: Flutter → JavaScript

**用途**: 从轮播池移除指定壁纸

```json
{
  "id": "msg-003",
  "type": "removeFromCarousel",
  "timestamp": 1699876543210,
  "data": {
    "imageUrl": "https://example.com/image-to-remove.jpg"
  }
}
```

---

### 2. 播放控制消息

#### 2.1 播放 (`play`)

```json
{
  "id": "msg-004",
  "type": "play",
  "timestamp": 1699876543210,
  "data": {}
}
```

#### 2.2 暂停 (`pause`)

```json
{
  "id": "msg-005",
  "type": "pause",
  "timestamp": 1699876543210,
  "data": {}
}
```

#### 2.3 停止 (`stop`)

```json
{
  "id": "msg-006",
  "type": "stop",
  "timestamp": 1699876543210,
  "data": {}
}
```

#### 2.4 下一张 (`next`)

```json
{
  "id": "msg-007",
  "type": "next",
  "timestamp": 1699876543210,
  "data": {}
}
```

#### 2.5 上一张 (`previous`)

```json
{
  "id": "msg-008",
  "type": "previous",
  "timestamp": 1699876543210,
  "data": {}
}
```

#### 2.6 跳转到指定索引 (`seek`)

```json
{
  "id": "msg-009",
  "type": "seek",
  "timestamp": 1699876543210,
  "data": {
    "index": 5
  }
}
```

---

### 3. 配置更新消息

#### 3.1 设置轮播间隔 (`setInterval`)

```json
{
  "id": "msg-010",
  "type": "setInterval",
  "timestamp": 1699876543210,
  "data": {
    "interval": 60000
  }
}
```

#### 3.2 设置过渡效果 (`setTransition`)

```json
{
  "id": "msg-011",
  "type": "setTransition",
  "timestamp": 1699876543210,
  "data": {
    "transition": "slide",
    "duration": 1000
  }
}
```

---

### 4. 状态同步消息

#### 4.1 轮播状态变化 (`carouselStateChanged`)

**方向**: JavaScript → Flutter

**用途**: 通知 Flutter 轮播状态改变

```json
{
  "id": "msg-012",
  "type": "carouselStateChanged",
  "timestamp": 1699876543210,
  "data": {
    "currentIndex": 3,
    "totalImages": 10,
    "isPlaying": true,
    "currentImageUrl": "https://example.com/image3.jpg"
  }
}
```

**触发时机**:
- 索引变化
- 播放状态变化
- 轮播池更新

---

#### 4.2 壁纸就绪 (`wallpaperReady`)

**方向**: JavaScript → Flutter

**用途**: 通知 Flutter 壁纸初始化完成

```json
{
  "id": "msg-013",
  "type": "wallpaperReady",
  "timestamp": 1699876543210,
  "data": {
    "loadTime": 1234,
    "imageCount": 10,
    "version": "1.0.0"
  }
}
```

---

#### 4.3 心跳 (`heartbeat`)

**方向**: 双向

**用途**: 检测连接状态

```json
{
  "id": "msg-014",
  "type": "heartbeat",
  "timestamp": 1699876543210,
  "data": {
    "seq": 123
  }
}
```

**响应**:

```json
{
  "id": "msg-015",
  "type": "heartbeatAck",
  "timestamp": 1699876543220,
  "data": {
    "seq": 123,
    "latency": 10
  }
}
```

---

### 5. 错误消息

#### 5.1 错误通知 (`error`)

**方向**: JavaScript → Flutter

**用途**: 报告 JavaScript 端错误

```json
{
  "id": "msg-016",
  "type": "error",
  "timestamp": 1699876543210,
  "data": {
    "code": "IMAGE_LOAD_FAILED",
    "message": "Failed to load image",
    "details": {
      "imageUrl": "https://example.com/broken.jpg",
      "httpStatus": 404
    }
  }
}
```

**错误代码**:
- `IMAGE_LOAD_FAILED`: 图片加载失败
- `NETWORK_ERROR`: 网络错误
- `INVALID_PARAMETER`: 参数错误
- `UNKNOWN_MESSAGE_TYPE`: 未知消息类型

---

### 6. 性能监控消息

#### 6.1 性能指标 (`performanceMetrics`)

**方向**: JavaScript → Flutter

**用途**: 报告性能指标

```json
{
  "id": "msg-017",
  "type": "performanceMetrics",
  "timestamp": 1699876543210,
  "data": {
    "fps": 60,
    "memoryUsage": 45678912,
    "imageLoadTime": 234,
    "renderTime": 16
  }
}
```

---

## ❌ 错误处理

### 错误响应格式

当消息处理失败时，应返回错误响应：

```json
{
  "id": "error-response-001",
  "type": "error",
  "timestamp": 1699876543210,
  "data": {
    "code": "ERROR_CODE",
    "message": "Human-readable error message",
    "originalMessageId": "msg-001",
    "details": {
      // 可选的详细信息
    }
  }
}
```

### 标准错误代码

| 错误代码 | 说明 |
|---------|------|
| `INVALID_MESSAGE_FORMAT` | 消息格式错误 |
| `MISSING_REQUIRED_FIELD` | 缺少必需字段 |
| `UNKNOWN_MESSAGE_TYPE` | 未知消息类型 |
| `OPERATION_FAILED` | 操作执行失败 |
| `TIMEOUT` | 操作超时 |
| `NOT_INITIALIZED` | 未初始化 |

---

## 📚 示例

### 示例 1: Flutter 更新轮播池

```dart
// Dart 端
final message = {
  'id': Uuid().v4(),
  'type': 'updateCarousel',
  'timestamp': DateTime.now().millisecondsSinceEpoch,
  'data': {
    'images': [
      'https://example.com/img1.jpg',
      'https://example.com/img2.jpg',
    ],
    'interval': 30000,
    'transition': 'fade',
    'autoPlay': true,
  },
};

await AnyWPEngine.sendMessage(message: message);
```

```javascript
// JavaScript 端
window.addEventListener('AnyWP:message', (event) => {
  const message = event.detail;
  
  if (message.type === 'updateCarousel') {
    carousel.updateImages(message.data.images);
    carousel.setInterval(message.data.interval);
    carousel.setTransition(message.data.transition);
    
    if (message.data.autoPlay) {
      carousel.play();
    }
  }
});
```

---

### 示例 2: JavaScript 报告状态变化

```javascript
// JavaScript 端
carousel.on('indexChanged', (index) => {
  const message = {
    id: generateUUID(),
    type: 'carouselStateChanged',
    timestamp: Date.now(),
    data: {
      currentIndex: index,
      totalImages: carousel.getTotalImages(),
      isPlaying: carousel.isPlaying(),
      currentImageUrl: carousel.getCurrentImageUrl()
    }
  };

  window.chrome.webview.postMessage(message);
});
```

```dart
// Dart 端
AnyWPEngine.setOnMessageCallback((message) {
  if (message['type'] == 'carouselStateChanged') {
    final data = message['data'] as Map<String, dynamic>;
    final currentIndex = data['currentIndex'] as int;
    final totalImages = data['totalImages'] as int;
    
    print('Carousel: $currentIndex / $totalImages');
    
    // 更新 UI
    notifyListeners();
  }
});
```

---

### 示例 3: 错误处理

```javascript
// JavaScript 端 - 捕获错误并报告
try {
  carousel.loadImage(imageUrl);
} catch (error) {
  const errorMessage = {
    id: generateUUID(),
    type: 'error',
    timestamp: Date.now(),
    data: {
      code: 'IMAGE_LOAD_FAILED',
      message: error.message,
      details: {
        imageUrl: imageUrl,
        error: error.toString()
      }
    }
  };

  window.chrome.webview.postMessage(errorMessage);
}
```

```dart
// Dart 端 - 处理错误
AnyWPEngine.setOnMessageCallback((message) {
  if (message['type'] == 'error') {
    final data = message['data'] as Map<String, dynamic>;
    final code = data['code'] as String;
    final errorMessage = data['message'] as String;
    
    print('Error: $code - $errorMessage');
    
    // 显示错误提示
    showErrorDialog(errorMessage);
  }
});
```

---

## 🔒 安全性

### 消息签名（可选）

对于敏感操作，可以添加消息签名：

```dart
String generateSignature(String messageJson, String secret) {
  final bytes = utf8.encode(messageJson + secret);
  final digest = sha256.convert(bytes);
  return digest.toString();
}

// 使用
final messageJson = jsonEncode(message);
final signature = generateSignature(messageJson, SECRET_KEY);

message['signature'] = signature;
```

```cpp
// C++ 端验证
bool VerifySignature(const std::string& message_json, const std::string& signature) {
  // 计算签名
  std::string computed = ComputeSHA256(message_json + SECRET_KEY);
  
  // 比较
  return computed == signature;
}
```

---

## 📊 性能考虑

### 1. 消息大小

- **推荐**: < 10KB
- **最大**: 100KB
- **超过限制**: 分批发送或使用文件传输

### 2. 发送频率

- **高频消息** (如 heartbeat): 每秒 1 次
- **中频消息** (如状态更新): 每秒 10 次
- **低频消息** (如配置更新): 按需发送

### 3. 批处理

对于大量消息，使用批处理：

```json
{
  "id": "batch-001",
  "type": "batch",
  "timestamp": 1699876543210,
  "data": {
    "messages": [
      { "type": "addToCarousel", ... },
      { "type": "addToCarousel", ... },
      { "type": "addToCarousel", ... }
    ]
  }
}
```

---

## 📖 版本兼容性

### 协议版本

在消息中包含协议版本：

```json
{
  "id": "msg-001",
  "type": "updateCarousel",
  "timestamp": 1699876543210,
  "version": "1.0",
  "data": { ... }
}
```

### 向后兼容

- 新增字段应为可选
- 弃用字段保留 2 个版本周期
- 重大变更需升级主版本号

---

## 🔗 相关文档

- [实施指南](./API_BRIDGE.md)
- [快速参考](./ENGINE_QUICK_REFERENCE.md)
- [测试用例](../test/message_protocol_test.dart)

---

**协议版本**: 1.0.0  
**更新日期**: 2025-11-12  
**维护者**: HKCW Desktop 开发团队

