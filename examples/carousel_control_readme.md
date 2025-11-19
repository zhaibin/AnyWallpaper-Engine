# 轮播壁纸控制测试

## 概述

轮播壁纸HTML (`test_carousel_control.html`) 是一个专门用于测试 WebMessage 双向通信和 Auto Recovery 功能的测试页面。

## 功能特性

### 1. 完全由 WebMessage 控制
- 不自主轮播，所有操作由 Flutter 端通过 WebMessage 控制
- 接收的消息类型：
  - `updateCarousel` - 更新图片列表、间隔、索引
  - `play` - 开始自动播放
  - `pause` - 暂停播放
  - `next` - 下一张
  - `previous` - 上一张
  - `setInterval` - 设置轮播间隔
  - `ping` - 心跳测试

### 2. 双向通信
- 发送消息到 Flutter：
  - `carouselReady` - 初始化完成
  - `carouselStateChanged` - 状态变化通知
  - `heartbeat` - 定期心跳（每10秒）
  - `pong` - 响应 ping 请求

### 3. Auto Recovery 测试
- 页面顶部显示 "Auto Recovery Enabled" 指示器
- 定期发送心跳消息，便于测试恢复功能
- 拔插显示器时，壁纸会自动恢复

### 4. 完整的状态面板
- **轮播状态面板**（右上角）
  - 当前状态：PLAYING / PAUSED / STOPPED
  - 当前索引、总图片数
  - 轮播间隔
  - 消息统计
  - 运行时间

- **图片信息**（底部中央）
  - 显示当前图片编号和 URL

- **消息日志**（右下角）
  - 最新10条消息记录
  - 显示消息类型、时间、数据

## Flutter 端控制界面

### Carousel Control 标签功能

1. **播放控制区**
   - 播放 / 暂停按钮
   - 上一张 / 下一张按钮

2. **轮播间隔设置**
   - 输入间隔时间（秒）
   - 点击"应用"生效

3. **图片列表管理**
   - 默认加载5张 Picsum 随机图片
   - 可添加/删除图片
   - 点击"同步到壁纸"更新

4. **通信统计**
   - 发送/接收消息计数
   - 消息列表（显示壁纸反馈）

## 使用流程

1. **自动启动**
   - 程序启动后自动加载轮播HTML壁纸
   - 默认加载5张示例图片

2. **控制轮播**
   - 切换到 "Carousel Control" 标签
   - 使用播放控制按钮
   - 观察右侧消息列表中的状态反馈

3. **自定义图片**
   - 点击"添加"按钮输入图片URL
   - 点击"同步到壁纸"更新

4. **测试 Auto Recovery**
   - 保持壁纸运行状态
   - 拔掉显示器
   - 重新插入显示器
   - 壁纸应自动恢复，继续播放

## 技术细节

### WebMessage 消息格式

**发送到壁纸：**
```dart
{
  'type': 'updateCarousel',
  'timestamp': 时间戳,
  'data': {
    'images': ['url1', 'url2', ...],
    'interval': 5000,  // 毫秒
    'currentIndex': 0
  }
}
```

**接收自壁纸：**
```javascript
{
  type: 'carouselStateChanged',
  timestamp: 时间戳,
  data: {
    status: 'playing',
    currentIndex: 0,
    totalImages: 5,
    interval: 5000
  }
}
```

### 轮播逻辑

- 播放时：每隔指定间隔自动切换到下一张
- 暂停时：停止自动切换，保持当前图片
- 手动切换：立即响应，不影响自动播放定时器

## 快捷测试页

主界面只保留3个核心测试页：
- **👁️ Visibility** - 可见性与省电测试
- **⚙️ API Test** - 完整API功能测试
- **👆 Click Test** - 鼠标点击检测测试

## 调试建议

1. 打开浏览器开发者工具（F12）
2. 查看 Console 日志
3. 观察 Flutter 端日志输出
4. 使用消息面板追踪通信状态


